<p align="center">
  <img height="100" src="https://i.imgur.com/HsBiYxw.png" alt="bitpacker"/>
</p>

<p align="center">
  <a href="https://travis-ci.org/CrustyAuklet/bitpacker">
    <img src="https://travis-ci.org/CrustyAuklet/bitpacker.svg?branch=master" alt="travis"/>
  </a>
  <a href="https://github.com/CrustyAuklet/bitpacker/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/license-BSL-blue.svg" alt="license"/>
  </a>
  <img src="https://img.shields.io/badge/version-0.1-blue.svg?cacheSeconds=2592000" alt="version"/>
</p>

# Bitpacker
A library to do type-safe and low-boilerplate bit level serialization

## Highlights
* Header only library
* Requires C++14 or later for low level functions
* Requires C++17 or later for python style interface
* Boost License
* works as a CMake sub-project

## Goals
-  Express bit-level serialization formats in a 100% cross platform way
-  Produce code of a similar size/overhead as the shift and mask method
-  Increase type safety (define how to pack and unpack a type *once*)
-  constexpr - create static messages at compile time with no code in final binary
-  Binary compatibility with [python bitstruct](https://pypi.org/project/bitstruct/) module
-  If possible support older embedded compilers (may end up a separate library)

## Limitations
-  API is *not* stable, mainly to allow flexibility in addressing other limitations and future goals
-  No float support yet

## Motivation
On small embedded systems structures of compressed data often need to be sent between different
 nodes in a network. Ideally a well established serialization library such as 
 [FlatBuffers](https://google.github.io/flatbuffers/),
 [ProtoBuffers](https://developers.google.com/protocol-buffers),
 or [Cereal](https://uscilab.github.io/cereal/) can be used. **BUT** Sometimes this is not possible
 for various reasons:
 -  code size and/or memory overhead in extremely small systems
 -  legacy protocols that are already set in stone with bitfields
 -  values need to be encoded in sub-byte sized fields
 
 Consider the following hypothetical message format:
 
 | Field      | Size   |
 | :--------- | :----- |
 | Voltage    | 12     |
 | Error flag | 1      |
 | Other flag | 1      |
 | Pressure   | 14     |
 | Time       | 24     |
 
 Each field is either a direct value, or there is some sort of pre-agreed upon function 
 that will interpret the value. The most common method to serialize/deserialize this data would be to
 use a union of bitfields:
 
```C++
// platform specific packing directive here
union Message {
    struct {
        uint32_t voltage  : 12;
        uint32_t error    : 1;
        uint32_t other    : 1;
        uint32_t pressure : 14;
    }; // 28 bits
    struct {
        uint32_t pad28_   : 28; // padding aligns the next value with the first 4
        uint32_t time     : 24;
        // 4 bits of implicit padding here
    };
};
```
While this is OK in C, it is Undefined Behaviour in C++ (type pun through a union). In both languages the
exact behaviour is *implementation defined*! between different implementations the packing may be 
left-to-right or right-to-left, and fields may or may not straddle byte boundaries.

A more portable solution would be to shift and mask data into the message:
```C++
// many other ways to do this, but for example....
std::array<uint8_t, 7> make_message(const message_data& data) {
    std::array<uint8_t, 7> buff{};
    buff[0] = data.voltage >> 4U;
    buff[1] = (data.voltage << 8U) | (data.error << 3U) | (data.other << 2U) | (data.pressure >> 12U);
    buff[2] = data.pressure >> 4U;
    buff[3] = (data.pressure << 4U) | (data.time >> 20U);
    buff[4] = data.time >> 12U;
    buff[5] = data.time >> 4U;
    buff[6] = data.time << 4U;
    return buff;
}
```
*Note: I did this quickly, and I bet there are mistakes. I am leaving them because it proves my point*

Shifting and masking is much less expressive, we lose the the ability to pass the message around as an object,
and it requires much more boilerplate (leading to more bugs). The one upside of this technique is that if done
correctly it is not reliant on implementation defined behaviour.

## Solution with Bitpacker
## Basic functionality
*(C++14 compatible)*

The low level BitPacker interface consists of two free functions that work on a view of bytes using 
`std::span`<sup>1</sup>. Fields are abstracted to a offset and size in bits. Already this is
much better than the other options, but produces [similar assembly](https://godbolt.org/z/stT9YF) 
even on `-O1`.
```C++
/// NOTE: I am ignoring any compression/transformations needed to change real values
///       into unsigned integers and back into real values for now.
std::array<uint8_t, 7> pack_message(const MessageData& data) {
    std::array<bitpacker::byte_type, 7> buff{};
    bitpacker::pack_into(buff, 0, 12,  static_cast<uint16_t>(data.voltage));
    bitpacker::pack_into(buff, 12, 1,  static_cast<uint8_t>(data.error));
    bitpacker::pack_into(buff, 13, 1,  static_cast<uint8_t>(data.other));
    bitpacker::pack_into(buff, 14, 14, static_cast<uint16_t>(data.pressure));
    bitpacker::pack_into(buff, 28, 24, static_cast<uint32_t>(data.time));
    return buff;
}

MessageData unpack_message(const std::array<uint8_t, 7>& buff) {
    MessageData data{};
    data.voltage  = bitpacker::unpack_from<uint16_t>(buff, 0, 12);
    data.error    = bitpacker::unpack_from<uint8_t>(buff, 12, 1);
    data.other    = bitpacker::unpack_from<uint8_t>(buff, 13, 1);
    data.pressure = bitpacker::unpack_from<uint16_t>(buff, 14, 14);
    data.time     = bitpacker::unpack_from<uint32_t>(buff, 28, 24);
    return data;
}
```
The bitfield method has the advantage of being able to read values larger than 1 byte in a single instruction,
essentially "type punning". Currently bitpacker only attempts to read values a byte at a time, essentially
the same as the shift and mask method but abstracted behind some very basic generic programming techniques.

### Type Safety
The above example is still not very "type safe": `pack_into` and `unpack_from` only support packing and
unpacking unsigned integer types. All other types must be manually cast when packed or unpacked. Also 
having two integer arguments that mean very different things is... not great. These functions are 
intentionally restrictive though. They are mainly intended to support code generation, higher levels of
abstraction, and older compilers.

If you are want to use these functions, but desire a slightly more type-safe interface there are two
function templates in the `bitpacker` namespace:
```c++
template <typename T>
constexpr T get(span<const byte_type> buffer, size_type offset) noexcept;

template <typename T>
constexpr void store(span<byte_type> buffer, size_type offset, T value) noexcept;
```
Specializations are provided for all fixed width integer types, and
user specializations are easy to create.

*NOTE: bool and floating point types are intentionally not specialized since the way to pack them
 is will change from project to project*
```C++
/// there are better ways to do this, but just for the example here are some compression functions
constexpr uint16_t compress_float_12(const float f) { /* Some compression method... */ }
constexpr uint16_t compress_float_14(const float f) { /* Some compression method... */ }
constexpr uint8_t compress_bool(const bool b) { /* Some compression method... */ }
constexpr uint32_t compress_time(const std::chrono::system_clock::rep) { /* Some compression method... */ }

namespace bitpacker {
    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, MessageData value) noexcept {
        // assume each type has some was to cast to the unsigned types
        pack_into(buffer, offset+0,  12, compress_float_12(data.voltage));
        pack_into(buffer, offset+12, 1,  compress_bool(data.error));
        pack_into(buffer, offset+13, 1,  compress_bool(data.other));
        pack_into(buffer, offset+14, 14, compress_float_14(data.pressure));
        pack_into(buffer, offset+28, 24, compress_time(data.time));
    }
}

constexpr std::array<uint8_t, 7> pack_message(const MessageData& data) {
    std::array<bitpacker::byte_type, 7> buff{};
    bitpacker::store(buff, 0, data);
    return buff;
}

// Something similar for unpacking by specializing
// template <typename T>
// constexpr T get(span<const byte_type> buffer, size_type offset) noexcept;
```

### Constexpr
both `pack_into` and `unpack_from` are constexpr, as are all type safe overloads. This allows the creating of
compile time message buffers. This is very useful if, for example, a device only has a few set messages that it
sends.
```C++
/// using the function from the last example using. Values obviously not real.
constexpr auto static_message = pack_message( { 3.3, true, false, 45.2, 16764793 } );
```

## Compile time python-like interface
If compiled with a C++17 compiler BitPacker also provides an interface that is compatible with
the [python bitstruct](https://pypi.org/project/bitstruct/) library. Unit tests ensure binary
compatability and the format string semantics are the same. Format strings are parsed at compile-time,
so the resulting code is the same as using the low level BitPacker API (sometimes better!).

*NOTE: float support is not yet implemented for packing/unpacking!*

[Compared to previous examples](https://godbolt.org/z/drUUQb)
```c++
// since float support is TBD, assume fixed point values for voltage/pressure
constexpr auto static_message = bitpacker::pack(BP_STRING("u12b1b1u14s24"), 3300, true, false, 4500, 16764793);

std::array<uint8_t, 7> make_message_py(const MessageData& data) {
    return bitpacker::pack(BP_STRING("u12b1b1u14s24"),
                data.voltage,
                data.error,
                data.other,
                data.pressure,
                data.time);
}
```

### Future Work
-  Packaging/install support and adding to some package managers
-  Implement floating point support in the python-like interface for full compatibility with 
[bitstruct](https://pypi.org/project/bitstruct/).
-  A container adaptor that will abstract away the offset by auto incrementing it as values are added.
This would be useful for runtime packing and hopefully be compatible with older compilers.
-  C++03 compatible version of the low-level interface. This is looking more and more like a separate thing.
-  Support some sort of code generation via a DSL, similar to other serialization
libraries. This is to enable a single human readable text file that can generate code (in multiple languages)
and documentation for bit-level message formats.

## Supported Toolchains
See the [Travis CI pipelines](https://travis-ci.com/github/CrustyAuklet/bitpacker) for all tested environments.

Notes:
 - Tests are run on MSVC 14 locally using visual studio 2019 (still haven't gotten it working in travis).
 - No reason it shouldn't work on earlier clang versions, I just haven't tested it
 
| Compiler             | Standard Library | Test Environment   |
| :------------------- | :--------------- | :----------------- |
| GCC >= 5.5.0         | libstdc++        | Ubuntu 18.04       |
| Clang >= 10          | libc++           | Xcode 10.2         |
| MSVC >= 14.1         | Microsoft STL    | Visual Studio 19   |

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [Boost](https://www.boost.org/users/license.html) license.

## Notes
1.  If not compiling with C++20 BitPacker will look for span-lite, gsl, or gsl-lite in the system and use their
implementation of span.