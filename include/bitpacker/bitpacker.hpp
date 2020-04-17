/**
 *  BITPACKER
 *  type-safe and low boilerplate bit-level serialization
 *  https://github.com/CrustyAuklet/bitpacker
 *
 *  Copyright 2020 Ethan Slattery
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <cstdint>
#include <cstddef>
#include <climits>
#include <limits>
#include <type_traits>
#include <tuple>

#ifndef   bitpacker_CPLUSPLUS
# if defined(_MSVC_LANG ) && !defined(__clang__)
#  define bitpacker_CPLUSPLUS  (_MSC_VER == 1900 ? 201103L : _MSVC_LANG )
# else
#  define bitpacker_CPLUSPLUS  __cplusplus
# endif
#endif

#define bitpacker_CPP98_OR_GREATER  ( bitpacker_CPLUSPLUS >= 199711L )
#define bitpacker_CPP11_OR_GREATER  ( bitpacker_CPLUSPLUS >= 201103L )
#define bitpacker_CPP14_OR_GREATER  ( bitpacker_CPLUSPLUS >= 201402L )
#define bitpacker_CPP17_OR_GREATER  ( bitpacker_CPLUSPLUS >= 201703L )
#define bitpacker_CPP20_OR_GREATER  ( bitpacker_CPLUSPLUS > 201703L )

#if bitpacker_CPP20_OR_GREATER && defined(__has_include )
# if __has_include( <span> )
#  define bitpacker_HAVE_STD_SPAN  1
# else
#  define bitpacker_HAVE_STD_SPAN  0
# endif
#else
# define  bitpacker_HAVE_STD_SPAN  0
#endif

#if bitpacker_HAVE_STD_SPAN
#    include <span>
#else
#    if defined __has_include
#        if __has_include(<gsl/gsl-lite.hpp>)
#            include <gsl/gsl-lite.hpp>
namespace bitpacker {
using gsl::span;
}
#        elif __has_include(<nonstd/span.hpp>)
#            include <nonstd/span.hpp>
namespace bitpacker {
using nonstd::span;
}
#        elif __has_include(<gsl/gsl>)
#            include <gsl/gsl>
namespace bitpacker {
using gsl::span;
}
#        endif
#    endif
#endif

namespace bitpacker {
#if bitpacker_HAVE_STD_SPAN
    using std::span;
    using std::as_bytes;
#endif

# if bitpacker_CPP17_OR_GREATER && defined(BITPACKER_USE_STD_BYTE)
    using byte_type = std::byte;
#else
    using byte_type = uint8_t;
# endif

    using size_type = std::size_t;
    constexpr size_type ByteSize = sizeof(byte_type) * CHAR_BIT;
    static_assert(CHAR_BIT == 8, "The target system has bytes that are not 8 bits!");
    static_assert(static_cast<unsigned>(-1) == ~0U, "The target system is not 2's compliment! Default pack specializations will not work!");
    static_assert(
#if bitpacker_CPP17_OR_GREATER && defined(BITPACKER_USE_STD_BYTE)
            std::is_same<byte_type, std::byte>::value ||
#endif
            std::is_unsigned<byte_type>::value && std::is_integral<byte_type>::value && sizeof(byte_type) == 1U,
            "ByteType needs to be either std::byte or uint8_t");

    namespace impl {

        /// Represents a given bit offset
        struct Offset {
            size_type byte;
            size_type bit;
        };

        /// returns true if the bit offset is byte aligned
        constexpr bool is_aligned(const size_t bit_offset) noexcept {
            return (bit_offset % ByteSize) == 0;
        }

        /// returns true if the offset is byte aligned
        constexpr bool is_aligned(const Offset offset) noexcept {
            return offset.bit == 0;
        }

        constexpr bool operator==(const Offset& lhs, const Offset& rhs) {
            return lhs.byte == rhs.byte && lhs.bit == rhs.bit;
        }
        constexpr bool operator>(const Offset& lhs, const Offset& rhs) {
            return lhs.byte > rhs.byte || ( lhs.byte == rhs.byte && lhs.bit > rhs.bit );
        }
        constexpr bool operator<(const Offset& lhs, const Offset& rhs) {
            return lhs.byte < rhs.byte || ( lhs.byte == rhs.byte && lhs.bit < rhs.bit );
        }
        constexpr bool operator!=(const Offset& lhs, const Offset& rhs) {
            return !(lhs == rhs);
        }
        constexpr bool operator>=(const Offset& lhs, const Offset& rhs) {
            return lhs > rhs || lhs == rhs;
        }
        constexpr bool operator<=(const Offset& lhs, const Offset& rhs) {
            return lhs < rhs || lhs == rhs;
        }

        /// get the byte and bit offset of a given bit number
        constexpr Offset get_offset(size_type offset) noexcept {
            return { (offset / ByteSize), (offset % ByteSize) };
        }
        /// create mask from the nth bit from the MSB to bit 0 (inclusive)
        constexpr byte_type right_mask(const size_type n) noexcept {
            return static_cast< byte_type >((1U << (ByteSize - n)) - 1);
        }
        /// create mask from the nth bit from the MSB to bit 8 (inclusive)
        constexpr byte_type left_mask(const size_type n) noexcept {
            return static_cast<byte_type>(~static_cast<byte_type>(right_mask(n) >> 1U));
        }

        template <size_t NumBits>
        using unsigned_type = std::conditional_t<NumBits <= 8, uint8_t,
                std::conditional_t<NumBits <= 16, uint16_t,
                        std::conditional_t<NumBits <= 32, uint32_t,
                                std::conditional_t<NumBits <= 64, uint64_t,
                                        void >>>>;

        template <size_t NumBits>
        using signed_type = std::conditional_t<NumBits <= 8, int8_t,
                std::conditional_t<NumBits <= 16, int16_t,
                        std::conditional_t<NumBits <= 32, int32_t,
                                std::conditional_t<NumBits <= 64, int64_t,
                                        void >>>>;
#pragma warning(push)
#pragma warning(disable : 4293)
        template<typename T, size_t BitSize>
        constexpr signed_type<BitSize> sign_extend(T val) noexcept
        {
            using return_type = signed_type<BitSize>;
            static_assert( std::is_unsigned<T>::value && std::is_integral<T>::value, "ValueType needs to be an unsigned integral type");
            // warning disabled for shifts bigger than type, since this if statement avoids that case.
            // if constexpr would work too, but trying to keep this section c++14 compatable
            if (BitSize < (sizeof(T)*ByteSize)) {
                const T upper_mask = static_cast<T>(~((static_cast<return_type>(1U) << BitSize) - 1));
                const T msb = static_cast<return_type>(1U) << (BitSize - 1);
                if (val & msb) { 
                    return static_cast<return_type>(val | upper_mask);
                }
            }
            return static_cast<return_type>(val);
        }
#pragma warning(pop)

        template < typename T, size_t BitSize >
        constexpr auto reverse_bits(std::remove_cv_t< T > val) noexcept
        {
            using return_type = std::remove_reference_t< std::remove_cv_t< T >>;
            size_t count = BitSize-1;
            return_type retval = val & 0x01U;

            val >>= 1U;
            while (val && count) {
                retval <<= 1U;
                retval |= val & 0x01U;
                val >>= 1U;
                --count;
            }
            return retval << count; 
        }

    }  // implementation namespace

    template<typename ValueType>
    constexpr void pack_into(span<byte_type> buffer, size_type offset, size_type size, ValueType v) noexcept {
        static_assert( std::is_unsigned<ValueType>::value && std::is_integral<ValueType>::value, "ValueType needs to be an unsigned integral type");
        const auto start = impl::get_offset(offset);
        const auto end   = impl::get_offset(offset + size - 1);
        const byte_type startMask   = impl::right_mask(start.bit);    // mask of the start byte, 1s where data is
        const byte_type endMask     = impl::left_mask(end.bit);       // mask of the end byte, 1s where data is

        // mask off any bits outside the size of the actual field, if size < bits in ValueType
        // NOTE: it is UB to left shift ANY value if the shift is >= the bits in the value!
        // this also takes care of zero size values.
        if( size < sizeof(ValueType)*ByteSize ) {
            v &= static_cast<ValueType>(( ValueType{0x1U} << size) - 1);
        }

        if (start.byte == end.byte) {
            // case where start and end are in the same byte
            buffer[start.byte] &= static_cast<byte_type>(~( static_cast<uint8_t>(startMask & endMask)));
            buffer[start.byte] |= static_cast<byte_type>(v << (ByteSize - (end.bit + 1)));
        }
        else {
            // case where start and end are in different bytes
            buffer[end.byte] &= static_cast<byte_type>(~endMask);
            // TODO: simpler way to get shift. byte aligned data is a special case (%ByteSize and the ternary)
            buffer[end.byte] |= static_cast<byte_type>(v << (ByteSize - ((end.bit+1)%ByteSize) ) % ByteSize);
            v >>= ((end.bit+1)%ByteSize) != 0 ? (end.bit+1)%ByteSize : ByteSize;

            for (size_type i = end.byte - 1; i > start.byte; --i) {
                buffer[i] = static_cast<byte_type>(v);
                // NOLINTNEXTLINE - this loop will NOT run 1-byte types
                v >>= ByteSize;
            }

            buffer[start.byte] &= static_cast<byte_type>(~startMask);
            buffer[start.byte] |= static_cast<byte_type>(v);
        }
    }

    template<typename ReturnType>
    constexpr ReturnType unpack_from(span<const byte_type> buffer, size_type offset, size_type size) noexcept {
        static_assert( std::is_unsigned<ReturnType>::value && std::is_integral<ReturnType>::value, "ReturnType needs to be an unsigned integral type");
        const auto start = impl::get_offset(offset);
        const auto end   = impl::get_offset(offset + size - 1);

        // case where size is zero
        if (size == 0) {
            return 0;
        }

        // case where the the entire field is in one byte
        if (start.byte == end.byte) {
            const size_type shift = (ByteSize - (end.bit + 1));
            // NOLINTNEXTLINE - size will always be <= 8 if we are within a byte!
            const size_type mask  = (1u << size) - 1;
            return static_cast<ReturnType>( static_cast<uint8_t>((buffer[start.byte]) >> shift) & mask );
        }

        // case where the field covers 2 or more bytes
        ReturnType value = static_cast<uint8_t>(buffer[start.byte]) & static_cast<uint8_t>(impl::right_mask(start.bit));
        for (size_type i = start.byte + 1; i < end.byte; ++i) {
            value = static_cast<ReturnType>(static_cast<ReturnType>(value << ByteSize) | static_cast<uint8_t>(buffer[i]));
        }
        const ReturnType shifted_end   = static_cast<ReturnType>(static_cast<uint8_t>(buffer[end.byte]) >> (ByteSize - (end.bit+1)));
        const ReturnType shifted_value = static_cast<ReturnType>(value << (end.bit + 1));
        return shifted_value | shifted_end;
    }

    /************************  Template specialization for unpacking  ***************************/

    template <typename T>
    constexpr T get(span<const byte_type> buffer, size_type offset) noexcept;

    /*************************  Template specialization for packing  ****************************/

    template <typename T>
    constexpr void store(span<byte_type> buffer, size_type offset, T value) noexcept;

#if bitpacker_CPP17_OR_GREATER
    /***************************************************************************************************
     * TMP (compile time) pack and unpack functionality (C++17 required)
     ***************************************************************************************************/

    namespace impl {


        struct format_string {
        };

        constexpr bool isFormatMode(char formatChar) noexcept {
            return formatChar == '<' || formatChar == '>';
        }

        constexpr bool isDigit(char ch) noexcept {
            return ch >= '0' && ch <= '9';
        }

        constexpr bool isFormatType(char formatChar) noexcept {
            return formatChar == 'u' || formatChar == 's'
                   || formatChar == 'f' || formatChar == 'b' || formatChar == 't'
                   || formatChar == 'r' || formatChar == 'p' || formatChar == 'P';
        }

        constexpr bool isPadding(char ch) noexcept {
            return ch == 'p' || ch == 'P';
        }

        constexpr bool isByteType(char ch) noexcept {
            return ch == 't' || ch == 'r';
        }

        constexpr bool isFormatChar(char formatChar) noexcept {
            return isFormatMode(formatChar) || isFormatType(formatChar) || isDigit(formatChar);
        }

        template <size_t Size>
        constexpr std::pair< size_t, size_t > consume_number(const char (&str)[Size], size_t offset) {
            size_t num = 0;
            size_t i = offset;
            for(; isDigit(str[i]) && i < Size; i++) {
                num = static_cast<size_t>(num*10 + (str[i] - '0'));
            }
            return {num, i};
        }

        enum class Endian {
            big,
            little
        };

        constexpr size_t bit2byte(size_t bits) noexcept
        {
            return (bits / ByteSize) + (bits % ByteSize ? 1 : 0);
        }

        template <char FormatChar, size_t BitCount>
        using format_type = std::conditional_t<FormatChar == 'u', impl::unsigned_type<BitCount>, 
            std::conditional_t<FormatChar == 's', impl::signed_type<BitCount>, 
                std::conditional_t<FormatChar == 'f', float, 
                    std::conditional_t<FormatChar == 'b', bool, 
                        std::conditional_t< FormatChar == 't', std::array< char, bit2byte(BitCount) >,
                            std::conditional_t< FormatChar == 'r', std::array< byte_type, bit2byte(BitCount) >, 
                                void>>>>>>;

        struct RawFormatType {
            char formatChar;
            size_t count;
            size_t offset;
            impl::Endian endian;
        };

        // Specifying the Big Endian format
        template <char FormatChar, size_t BitCount, impl::Endian BitEndianess>
        struct FormatType {
            static constexpr impl::Endian bit_endian = BitEndianess;
            static constexpr size_t bits = BitCount;        // also used for byte count for 't' and 'r' formats
            static constexpr char format = FormatChar;
            using return_type = format_type<FormatChar, BitCount>;
            using rep_type = impl::unsigned_type<BitCount>;
        };

        template <typename Fmt>
        constexpr bool validate_format(Fmt f) noexcept {
            for(size_t i = 0; i < Fmt::size(); i++) {
                auto currentChar = Fmt::at(i);
                if(impl::isFormatMode(currentChar)) {
                    if(++i == Fmt::size()){
                        break;
                    }
                }

                if(!impl::isFormatType(Fmt::at(i++))) {
                    return false;
                }

                const auto num_and_offset = impl::consume_number(Fmt::value(), i);
                i = num_and_offset.second;
                --i; // to combat the i++ in the loop
                if(num_and_offset.first == 0) {
                    return false;
                }
            }
            return true;
        }

        /// return the format mode of the entire buffer (byte order).
        template <typename Fmt>
        constexpr impl::Endian get_byte_order(Fmt) noexcept {
            // last character is the byte order, big endian if missing
            constexpr auto last_char = Fmt::at(Fmt::size()-1);
            return last_char == '<' ? impl::Endian::little : impl::Endian::big;
        }

        /// count the number of items in the format
        /// @param array_as_one [IN] if true, count byte/char aray as one item (default)
        template <typename Fmt>
        constexpr size_t count_items(Fmt f, const bool array_as_one = true) noexcept
        {
            static_assert(validate_format(Fmt{}), "Invalid Format!");
            size_t itemCount = 0;
            bool   is_bytes = false;

            for(size_t i = 0; i < Fmt::size(); i++) {
                auto currentChar = Fmt::at(i);
                if(impl::isFormatMode(currentChar)) {
                    continue;
                }

                if(impl::isFormatType(currentChar)) {
                    is_bytes = (currentChar == 't' || currentChar == 'r');
                    currentChar = Fmt::at(++i);
                }

                if (impl::isDigit(currentChar)) {
                    const auto num_and_offset = impl::consume_number(Fmt::value(), i);

                    itemCount += is_bytes && !array_as_one ? num_and_offset.first : 1;
                    i = num_and_offset.second;
                    --i; // to combat the i++ in the loop
                }
                is_bytes = false;
            }
            return itemCount;
        }

        template < typename Fmt >
        constexpr std::array< RawFormatType, count_items(Fmt{}) > get_type_array(Fmt f) noexcept
        {
            std::array< RawFormatType, count_items(Fmt{}) > arr{};
            impl::Endian currentEndian = impl::Endian::big;
            size_t currentType = 0;
            size_t offset = 0;

            for (size_t i = 0; i < Fmt::size(); i++) {
                auto currentChar = Fmt::at(i);
                if (impl::isFormatMode(currentChar)) {
                    currentEndian = currentChar == '>' ? impl::Endian::big : impl::Endian::little;
                    continue;
                }

                if (impl::isFormatType(currentChar)) {
                    const auto num_and_offset = impl::consume_number(Fmt::value(), ++i);
                    arr[currentType].formatChar = currentChar;
                    arr[currentType].endian = currentEndian;
                    arr[currentType].count = num_and_offset.first;
                    arr[currentType].offset = offset;
                    offset += num_and_offset.first;

                    ++currentType;

                    i = num_and_offset.second;
                    i--;  // to combat the i++ in the loop
                }
            }
            return arr;
        }

        template < size_t Item, typename Fmt >
        constexpr RawFormatType get_item_type(Fmt f) noexcept
        {
            static_assert(Item < count_items(Fmt{}), "Invalid format item index!");
            constexpr auto type_array = get_type_array(Fmt{});
            return type_array[Item];
        }

    /***************************************************************************************************
     * Compile time unpacking functionality
     ***************************************************************************************************/

        template <typename Fmt, size_t... Items, typename Input>
        constexpr auto unpack(std::index_sequence<Items...>, Input&& packedInput);

        template < typename UnpackedType >
        constexpr auto unpackElement(span< const byte_type > buffer, size_t offset) -> typename UnpackedType::return_type
        {
            // TODO: Implement these formats
            static_assert(UnpackedType::format != 'f', "Unpacking Floats not supported yet...");
            static_assert(UnpackedType::format != 'p' && UnpackedType::format != 'P', "Unpacking padding not supported yet...");

            if constexpr (UnpackedType::format == 'u') {
                const auto v = unpack_from< typename UnpackedType::rep_type >(buffer, offset, UnpackedType::bits);
                if (UnpackedType::bit_endian == impl::Endian::little) {
                    return impl::reverse_bits< decltype(v), UnpackedType::bits >(v);
                }
                return v;
            }
            if constexpr (UnpackedType::format == 's') {
                const auto val = unpack_from< typename UnpackedType::rep_type >(buffer, offset, UnpackedType::bits);
                if (UnpackedType::bit_endian == impl::Endian::little) {
                    const auto rval = impl::reverse_bits< decltype(val), UnpackedType::bits >(val);
                    return impl::sign_extend< decltype(val), UnpackedType::bits >(rval);
                }
                return impl::sign_extend< decltype(val), UnpackedType::bits >(val);
            }
            if constexpr (UnpackedType::format == 'b') {
                const auto val = unpack_from< typename UnpackedType::rep_type >(buffer, offset, UnpackedType::bits);
                return static_cast< bool >(val);
            }
            if constexpr (UnpackedType::format == 'f') {
                static_assert(UnpackedType::bits == 16 || UnpackedType::bits == 32 || UnpackedType::bits == 64,
                              "Expected float size of 16, 32, or 64 bits");
            }
            if constexpr (UnpackedType::format == 't' || UnpackedType::format == 'r') {
                // to remain binary compatable with bitstruct: bitcount is actual bits.
                // Any partial bytes end up in the last byte/char, left aligned.
                constexpr unsigned charsize = 8U;
                constexpr unsigned full_bytes = UnpackedType::bits / 8;
                constexpr unsigned extra_bits = UnpackedType::bits % 8;
                constexpr size_t return_size = bit2byte(UnpackedType::bits);
                typename UnpackedType::return_type buff{};
                
                for (size_t i = 0; i < full_bytes; ++i) {
                    buff[i] = unpack_from< uint8_t >(buffer, offset + (i * charsize), charsize);
                }

                if (extra_bits > 0) {
                    buff[return_size - 1] = unpack_from< uint8_t >(buffer, offset + (full_bytes * charsize), extra_bits);
                    buff[return_size - 1] <<= charsize - extra_bits; 
                }

                // little endian bitwise in bitstruct means the entire length flipped.
                // to simulate this we reverse the order then flip each bytes bit order
                if (UnpackedType::bit_endian == impl::Endian::little) {
                    std::reverse(buff.begin(), buff.end());
                    for (auto &v : buff) {
                        v = impl::reverse_bits< decltype(v), ByteSize >(v);
                    }
                }

                return buff;
            }
            if constexpr (UnpackedType::format == 'p' || UnpackedType::format == 'P') {
                // the bit count is used for the number of char/bytes in a 't' or 'r' format
                //std::array< typename UnpackedType::rep_type, UnpackedType::bits >
            }
        }

        template < typename Fmt, std::size_t... Items >
        constexpr auto FormatTypes_impl(Fmt f, std::index_sequence< Items... >)
        {
            constexpr auto formats = impl::get_type_array(Fmt{});
            using Types = std::tuple< typename impl::FormatType< formats[Items].formatChar, formats[Items].count, formats[Items].endian >... >;
            return Types{};
        }

        template < typename Fmt, typename Indices = std::make_index_sequence< count_items(Fmt{}) > >
        constexpr auto FormatTypes(Fmt f)
        {
            return FormatTypes_impl(f, Indices{});
        }

        template < typename Fmt, std::size_t... Items >
        constexpr auto ReturnTypes_impl(Fmt f, std::index_sequence< Items... >)
        {
            constexpr auto formats = impl::get_type_array(Fmt{});
            using Types = std::tuple< typename impl::FormatType< formats[Items].formatChar, formats[Items].count, formats[Items].endian >::return_type... >;
            return Types{};
        }

        template < typename Fmt, typename Indices = std::make_index_sequence< count_items(Fmt{}) > >
        constexpr auto ReturnTypes(Fmt f)
        {
            return ReturnTypes_impl(f, Indices{});
        }

    } // namespace impl

    /// Count the bits used by the given format
    template < typename Fmt >
    constexpr size_t calcsize(Fmt f)
    {
        constexpr auto type_array = impl::get_type_array(Fmt{});
        const auto last = type_array.back();
        return last.offset + last.count;
    }

    /// Count the number of bytes needed to hold the given format
    template < typename Fmt >
    constexpr size_t calcbytes(Fmt f)
    {
        constexpr auto bits = calcsize(Fmt{});
        return impl::bit2byte(bits);
    }

    template < typename Fmt, typename Input >
    constexpr auto unpack(Fmt, Input &&packedInput)
    {
        return impl::unpack< Fmt >(std::make_index_sequence< count_items(Fmt{}) >(),
                                   std::forward< Input >(packedInput));
    }

    template < typename Fmt, size_t... Items, typename Input >
    constexpr auto impl::unpack(std::index_sequence< Items... >, Input &&packedInput)
    {
        constexpr auto byte_order = impl::get_byte_order(Fmt{});
        static_assert(byte_order == impl::Endian::big, "Unpacking little endian byte order not supported yet...");
        constexpr auto formats = impl::get_type_array(Fmt{});

        using FormatTypes = decltype(FormatTypes<>(Fmt{}));
        using ReturnTypes = decltype(ReturnTypes<>(Fmt{}));

        const auto unpacked = std::make_tuple(
            impl::unpackElement< typename std::tuple_element_t< Items, FormatTypes > >(packedInput, formats[Items].offset)...);
        return unpacked;
    }

} // namespace bitpacker

#define BP_STRING(s) [] { \
    struct S : bitpacker::impl::format_string { \
      static constexpr decltype(auto) value() { return s; } \
      static constexpr size_t size() { return std::size(value()) - 1; }  \
      static constexpr auto at(size_t i) { return value()[i]; }; \
    }; \
    return S{}; \
  }()

#else
} // namespace bitpacker
#endif  // bitpacker_CPP17_OR_GREATER
