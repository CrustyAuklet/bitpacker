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
#include <algorithm>
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
        constexpr bool is_aligned(const size_type bit_offset) noexcept {
            return (bit_offset % ByteSize) == 0;
        }

        /// returns true if the offset is byte aligned
        constexpr bool is_aligned(const Offset offset) noexcept {
            return offset.bit == 0;
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

        /// finds the smallest fixed width unsigned integer that can fit NumBits bits
        template <size_type NumBits>
        using unsigned_type = std::conditional_t<NumBits <= 8, uint8_t,
                std::conditional_t<NumBits <= 16, uint16_t,
                        std::conditional_t<NumBits <= 32, uint32_t,
                                std::conditional_t<NumBits <= 64, uint64_t,
                                        void >>>>;

        /// finds the smallest fixed width signed integer that can fit NumBits bits
        template <size_type NumBits>
        using signed_type = std::conditional_t<NumBits <= 8, int8_t,
                std::conditional_t<NumBits <= 16, int16_t,
                        std::conditional_t<NumBits <= 32, int32_t,
                                std::conditional_t<NumBits <= 64, int64_t,
                                        void >>>>;
#pragma warning(push)
#pragma warning(disable : 4293)
        /// sign extends an unsigned integral value to prepare for casting to a signed value.
        template<typename T, size_type BitSize>
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

        /// reverses the bits in the value `val`.
        template < typename T, size_type BitSize >
        constexpr auto reverse_bits(std::remove_cv_t< T > val) noexcept
        {
            using val_type = std::remove_reference_t< decltype(val) >;
            static_assert(std::is_integral<val_type>::value, "bitpacker::reverse_bits: val needs to be an integral type");
            using return_type = std::remove_reference_t< std::remove_cv_t< T >>;
            size_type count = BitSize-1;
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

    /**
     * Inserts an unsigned integral value `v` into the byte buffer `buffer`. The value will overwrite
     * the bits from bit `offset` to `offset` + `size` counting from the most significant bit of the first byte
     * in the buffer. Bits adjacent to this field will not be modified.
     * @tparam ValueType Type of the value `v` to insert. Must be an unsigned integral type.
     * @param buffer [IN/OUT] Span of bytes to insert the value `v` into
     * @param offset [IN] the bit offset to insert at. The value `v` will begin at this bit index
     * @param size [IN] the number of bits to use for inserting the value `v`. Must be <= 64.
     * @param v [IN] the value to insert into the byte container
     */
    template<typename ValueType>
    constexpr void insert(span<byte_type> buffer, size_type offset, size_type size, ValueType v) noexcept {
        static_assert( std::is_unsigned<ValueType>::value && std::is_integral<ValueType>::value, "bitpacker::insert : ValueType needs to be an unsigned integral type");
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

    /**
     * Extracts an unsigned integral value from the byte buffer `buffer`. The value will be equal to
     * the bits from bit `offset` to `offset` + `size` counting from the most significant bit of the first byte
     * in the buffer.
     * @tparam ReturnType The return type of this function. Must be an unsigned integral type
     * @param buffer [IN] view of bytes to extract the value from. They will not be modified.
     * @param offset [IN] the bit offset to extract from. The return value will begin at this bit index.
     * @param size [IN] the number of bits to use, starting from `offset`, to construct the return value. Must be <= 64.
     * @return The unsigned integral value contained in `buffer` bit [`offset`, `offset`+`size`-1]. If ReturnType is
     *         not explicitly specified the smalled fixed width unsigned integer that can contain the value will be returned.
     */
    template<typename ReturnType>
    constexpr ReturnType extract(span<const byte_type> buffer, size_type offset, size_type size) noexcept {
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

        template <size_type Size>
        constexpr std::pair< size_type, size_type > consume_number(const char (&str)[Size], size_type offset) {
            size_type num = 0;
            size_type i = offset;
            for(; isDigit(str[i]) && i < Size; i++) {
                num = static_cast<size_type>(num*10 + (str[i] - '0'));
            }
            return {num, i};
        }

        enum class Endian {
            big,
            little
        };

        /// calculate the number of bytes needed to hold the given number of bits
        constexpr size_type bit2byte(size_type bits) noexcept
        {
            return (bits / ByteSize) + ((bits % ByteSize) ? 1 : 0);
        }

        /// figure out the type associated with a given format character
        template <char FormatChar, size_type BitCount>
        using format_type = std::conditional_t<FormatChar == 'u', impl::unsigned_type<BitCount>, 
            std::conditional_t<FormatChar == 's', impl::signed_type<BitCount>, 
                std::conditional_t<FormatChar == 'f', float, 
                    std::conditional_t<FormatChar == 'b', bool, 
                        std::conditional_t< FormatChar == 't', std::array< char, bit2byte(BitCount) >,
                            std::conditional_t< FormatChar == 'r', std::array< byte_type, bit2byte(BitCount) >, 
                                void>>>>>>;

        struct RawFormatType {
            char formatChar;    //< the format character of this items type
            size_type count;       //< number of bits in this item
            size_type offset;      //< offset from start of format in bits
            impl::Endian endian;//< bit endianness of this value
        };

        // Specifying the Big Endian format
        template <char FormatChar, size_type BitCount, impl::Endian BitEndianess>
        struct FormatType {
            static constexpr impl::Endian bit_endian = BitEndianess;
            static constexpr size_type bits = BitCount;        // also used for byte count for 't' and 'r' formats
            static constexpr char format = FormatChar;
            using return_type = format_type<FormatChar, BitCount>;
            using rep_type = impl::unsigned_type<BitCount>;
        };

        /// validates the given format string
        template <typename Fmt>
        constexpr bool validate_format(Fmt /*unused*/) noexcept {
            for(size_type i = 0; i < Fmt::size(); i++) {
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
        constexpr impl::Endian get_byte_order(Fmt /*unused*/) noexcept {
            // last character is the byte order, big endian if missing
            constexpr auto last_char = Fmt::at(Fmt::size()-1);
            return last_char == '<' ? impl::Endian::little : impl::Endian::big;
        }

        /**
         * @tparam Fmt The static format string created with macro BP_STRING
         * @param f [IN] instance of Fmt for auto template deduction
         * @param count_padding [IN] if true count padding type formats
         * @param count_normal [IN] if true count non-padding type formats
         * @return count of items
         */
        template <typename Fmt>
        constexpr size_type count_fmt_items(Fmt /*unused*/, const bool count_padding = true, const bool count_normal = true) noexcept
        {
            static_assert(validate_format(Fmt{}), "Invalid Format!");
            size_type itemCount = 0;
            bool count_item = false;

            for(size_type i = 0; i < Fmt::size(); i++) {
                auto currentChar = Fmt::at(i);
                if(impl::isFormatMode(currentChar)) {
                    continue;
                }

                if(impl::isFormatType(currentChar)) {
                    count_item = (isPadding(currentChar) && count_padding) || (!isPadding(currentChar) && count_normal);
                    currentChar = Fmt::at(++i);
                }

                if (impl::isDigit(currentChar)) {
                    const auto num_and_offset = impl::consume_number(Fmt::value(), i);

                    itemCount += count_item ? 1 : 0;
                    i = num_and_offset.second;
                    --i; // to combat the i++ in the loop
                }
                count_item = false;
            }
            return itemCount;
        }

        /// count the number of items in the format
        template <typename Fmt>
        constexpr size_type count_all_items(Fmt /*unused*/) noexcept
        {
            return count_fmt_items(Fmt{}, true, true);
        }

        /// count the number of non-padding items in the format
        template <typename Fmt>
        constexpr size_type count_non_padding(Fmt /*unused*/) noexcept
        {
            return count_fmt_items(Fmt{}, false, true);
        }

        /// count the number of padding type items in the format
        template <typename Fmt>
        constexpr size_type count_padding(Fmt /*unused*/) noexcept
        {
            return count_fmt_items(Fmt{}, true, false);
        }

        /// parse the given format string to a homogenous array of objects that describe each type
        template < typename Fmt>
        constexpr auto get_type_array(Fmt /*unused*/) noexcept
        {
            std::array< RawFormatType, count_all_items(Fmt{}) > arr{};
            impl::Endian currentEndian = impl::Endian::big;
            size_type currentType = 0;
            size_type offset = 0;

            for (size_type i = 0; i < Fmt::size(); i++) {
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

/****************************************************************************************************
 * Compile time unpacking implementation
 **************************************************************************************************/

#if bitpacker_CPP20_OR_GREATER
        // constexpr in c++20 and greater
        using std::reverse;
        using std::copy;
#else
        // copied from cppref but constexpr, we KNOW its trivial types
        template < class BidirIt >
        constexpr void reverse(BidirIt first, BidirIt last)
        {
            while ((first != last) && (first != --last)) {
                const auto tmp = *first;
                *first = *last;
                *last = tmp;
                ++first;
            }
        }

        // copied from cppref but constexpr
        template<class InputIt, class OutputIt>
        constexpr OutputIt copy(InputIt first, InputIt last, OutputIt d_first)
        {
            while (first != last) {
                *d_first++ = *first++;
            }
            return d_first;
        }
#endif

        template <typename Fmt, size_type... Items, typename Input>
        constexpr auto unpack(std::index_sequence<Items...> /*unused*/, Input&& packedInput, const size_type start_bit);

        /// does the work of unpacking each type, based on the type passed to UnpackedType
        template < typename UnpackedType >
        constexpr auto unpackElement(span< const byte_type > buffer, size_type offset) -> typename UnpackedType::return_type
        {
            // TODO: Implement float unpacking
            static_assert(UnpackedType::format != 'f', "Unpacking Floats not supported yet...");
            static_assert(!isPadding(UnpackedType::format), "Something is wrong :( Padding types shouldn't get here!");

            if constexpr (UnpackedType::format == 'u' || UnpackedType::format == 's') {
                static_assert(UnpackedType::bits <= 64, "Integer types must be 64 bits or less");
                auto val = extract< typename UnpackedType::rep_type >(buffer, offset, UnpackedType::bits);
                if (UnpackedType::bit_endian == impl::Endian::little) {
                    val = impl::reverse_bits< decltype(val), UnpackedType::bits >(val);
                }
                if constexpr (UnpackedType::format == 's') {
                    return impl::sign_extend< decltype(val), UnpackedType::bits >(val);
                }
                return val;
            }
            if constexpr (UnpackedType::format == 'b') {
                static_assert(UnpackedType::bits <= 64, "Boolean types must be 64 bits or less");
                const auto val = extract< typename UnpackedType::rep_type >(buffer, offset, UnpackedType::bits);
                return static_cast< bool >(val);
            }
            if constexpr (UnpackedType::format == 'f') {
                static_assert(UnpackedType::bits == 16 || UnpackedType::bits == 32 || UnpackedType::bits == 64,
                              "Expected float size of 16, 32, or 64 bits");
            }
            if constexpr (isByteType(UnpackedType::format)) {
                // to remain binary compatible with bitstruct: bitcount is actual bits.
                // Any partial bytes end up in the last byte/char, left aligned.
                constexpr unsigned charsize = 8U;
                constexpr unsigned full_bytes = UnpackedType::bits / charsize;
                constexpr unsigned extra_bits = UnpackedType::bits % charsize;
                constexpr size_type return_size = bit2byte(UnpackedType::bits);
                typename UnpackedType::return_type buff{};
                
                for (size_type i = 0; i < full_bytes; ++i) {
                    buff[i] = extract< uint8_t >(buffer, offset + (i * charsize), charsize);
                }

                if (extra_bits > 0) {
                    buff[return_size - 1] = extract< uint8_t >(buffer, offset + (full_bytes * charsize), extra_bits);
                    buff[return_size - 1] <<= charsize - extra_bits; 
                }

                // little endian bitwise in bitstruct means the entire length flipped.
                // to simulate this we reverse the order then flip each bytes bit order
                if (UnpackedType::bit_endian == impl::Endian::little) {
                    bitpacker::impl::reverse(buff.begin(), buff.end());
                    for (auto &v : buff) {
                        v = impl::reverse_bits< decltype(v), ByteSize >(v);
                    }
                }

                return buff;
            }
        }

        /// given an array from `get_type_array()`, remove all the types that represent padding
        template <size_type N>
        constexpr auto remove_padding(const std::array< RawFormatType, N > &types) noexcept
        {
            std::array< RawFormatType, N > buff{};
            size_type insert_idx = 0;
            for (const auto& t : types) {
                if (!isPadding(t.formatChar)) {
                    buff[insert_idx++] = t;
                }
            }
            return buff;
        }

/***************************************************************************************************
* Compile time packing implementation
***************************************************************************************************/

        /// given an array from `get_type_array()`, remove all the types that DON'T represent padding
        template <size_type N>
        constexpr auto remove_non_padding(const std::array< RawFormatType, N > &types) noexcept
        {
            std::array< RawFormatType, N > buff{};
            size_type insert_idx = 0;
            for (const auto& t : types) {
                if (isPadding(t.formatChar)) {
                    buff[insert_idx++] = t;
                }
            }
            return buff;
        }

        /// basic conversion function. Allows for custom specialization in the future?
        template <typename RepType, typename T>
        constexpr RepType convert_for_pack(const T& val)
        {
            return static_cast<RepType>(val);
        }

        /// does the work of packing `elem`, based on the type passed to PackedType
        template <typename PackedType, typename InputType>
        constexpr int packElement(span<byte_type> buffer, size_type offset, InputType elem)
        {
            // TODO: Implement float packing
            static_assert(PackedType::format != 'f', "Unpacking Floats not supported yet...");

            if constexpr (PackedType::format == 'u' || PackedType::format == 's') {
                static_assert(PackedType::bits <= 64, "Integer types must be 64 bits or less");
                auto val = convert_for_pack< typename PackedType::rep_type >(elem);
                if (PackedType::bit_endian == impl::Endian::little) {
                    val = impl::reverse_bits< decltype(val), PackedType::bits >(val);
                }
                insert(buffer, offset, PackedType::bits, val);
            }
            if constexpr (PackedType::format == 'b') {
                static_assert(PackedType::bits <= 64, "Boolean types must be 64 bits or less");
                // cast to a bool and then to rep type to ensure:
                //   -  value is 1 or 0 for binary compatibility with python
                //   -  bitpacker::insert gets an unsigned integer instead of a bool to avoid warnings for shifting bools
                insert(buffer, offset, PackedType::bits, static_cast<typename PackedType::rep_type>(static_cast<bool>(elem)));
            }
            if constexpr (isPadding(PackedType::format)) {
                static_assert(PackedType::bits <= 64, "Padding fields must be 64 bits or less");
                if(PackedType::format == 'P') {
                    constexpr auto val = static_cast< unsigned_type<PackedType::bits> >(-1);
                    insert(buffer, offset, PackedType::bits, val);
                }
                else {
                    insert(buffer, offset, PackedType::bits, 0U);
                }
            }
            if constexpr (PackedType::format == 'f') {
                static_assert(PackedType::bits == 16 || PackedType::bits == 32 || PackedType::bits == 64,
                              "Expected float size of 16, 32, or 64 bits");
            }
            if constexpr (isByteType(PackedType::format)) {
                // to remain binary compatible with bitstruct: bitcount is actual bits.
                // Any partial bytes end up in the last byte/char, left aligned.
                constexpr unsigned charsize = 8U;
                constexpr unsigned full_bytes = PackedType::bits / charsize;
                constexpr unsigned extra_bits = PackedType::bits % charsize;
                constexpr unsigned byte_count = bit2byte(PackedType::bits);

                if(PackedType::bit_endian == impl::Endian::little) {
                    constexpr auto size = std::extent_v<decltype(elem)>;
                    std::array<uint8_t, byte_count> arr{};
                    bitpacker::impl::copy(&elem[0], &elem[0]+byte_count, arr.begin());

                    // little endian bitwise in bitstruct means the entire length flipped.
                    // to simulate this we reverse the order then flip each bytes bit order
                    bitpacker::impl::reverse(std::begin(arr), std::end(arr));
                    for(auto &v : arr) {
                        v = impl::reverse_bits< decltype(v), ByteSize >(v);
                    }

                    for(int bits = PackedType::bits, idx = 0; bits > 0; bits -= charsize) {
                        const auto field_size = bits < charsize ? bits : charsize;
                        insert<uint8_t>(buffer, offset + (idx * charsize), charsize, arr[idx]);
                        ++idx;
                    }
                }
                else {
                    for(int bits = PackedType::bits, idx = 0; bits > 0; bits -= charsize) {
                        const auto field_size = bits < charsize ? bits : charsize;
                        insert<uint8_t>(buffer, offset + (idx * charsize), charsize, elem[idx]);
                        ++idx;
                    }
                }
            }
            return 0;
        }

        /// Helper function to insert padding fields into the buffer for `pack_into()`
        template <typename Fmt, size_type... Items>
        constexpr auto insert_padding(span<byte_type> buffer, const size_type start_bit, std::index_sequence<Items...> /*unused*/)
        {
            constexpr auto formats_only_pad = impl::remove_non_padding(impl::get_type_array(Fmt{}));
            using FormatTypes = std::tuple< typename impl::FormatType< formats_only_pad[Items].formatChar,
                                                                       formats_only_pad[Items].count,
                                                                       formats_only_pad[Items].endian >... >;
            int _[] = { 0, packElement< std::tuple_element_t<Items, FormatTypes> >(buffer, formats_only_pad[Items].offset + start_bit, 0)... };
            (void)_; // _ is a dummy for pack expansion
        }

        /// helper function to pack types into the given buffer
        template <typename Fmt, size_type N, size_type... Items, typename... Args>
        constexpr void pack(std::array<byte_type, N>& output, const size_type start_bit, std::index_sequence<Items...> /*unused*/, Args&&... args)
        {
            static_assert(sizeof...(args) == sizeof...(Items), "pack expected items for packing != sizeof...(args) passed");
            constexpr auto byte_order = impl::get_byte_order(Fmt{});
            static_assert(byte_order == impl::Endian::big, "Unpacking little endian byte order not supported yet...");
            constexpr auto formats_no_pad   = impl::remove_padding(impl::get_type_array(Fmt{}));

            using FormatTypes = std::tuple< typename impl::FormatType< formats_no_pad[Items].formatChar,
                                                                       formats_no_pad[Items].count,
                                                                       formats_no_pad[Items].endian >... >;

            impl::insert_padding<Fmt>( output, start_bit, std::make_index_sequence<impl::count_padding(Fmt{})>());
            int _[] = { 0, packElement< std::tuple_element_t<Items, FormatTypes> >(output, formats_no_pad[Items].offset + start_bit, args)... };
            (void)_; // _ is a dummy for pack expansion
        }

    }   // namespace impl

/***************************************************************************************************
* Compile time python-like interface
***************************************************************************************************/

    /**
    * get the number of bits used by a given format
    * @param fmt [IN] format string created with macro `BP_STRING()`
    * @return the number of bits in given format string Fmt
    */
    template < typename Fmt >
    constexpr size_type calcsize(Fmt /*unused*/)
    {
        constexpr auto type_array = impl::get_type_array(Fmt{});
        const auto last = type_array.back();
        return last.offset + last.count;
    }

    /**
     * get the number of bytes needed to hold a given format
     * @param fmt [IN] format string created with macro `BP_STRING()`
     * @return the number of bytes in given format string Fmt
     */
    template < typename Fmt >
    constexpr size_type calcbytes(Fmt /*unused*/)
    {
        constexpr auto bits = calcsize(Fmt{});
        return impl::bit2byte(bits);
    }

    /**
     * Unpack packedInput (container of bytes) according to given
     * format string fmt. The result is a tuple even if it contains exactly one item.
     * @param fmt [IN] format string created with macro `BP_STRING()`
     * @param packedInput [IN] container of byte types
     * @return tuple of results according to format string
     */
    template < typename Fmt, typename Input >
    constexpr auto unpack(Fmt /*unused*/, Input &&packedInput)
    {
        return impl::unpack< Fmt >(std::make_index_sequence< impl::count_non_padding(Fmt{}) >(),
                                   std::forward< Input >(packedInput), 0);
    }

    /**
     * Unpack packedInput (container of bytes) according to
     * given format string fmt, starting at given bit offset offset.
     * The result is a tuple even if it contains exactly one item.
     * @param fmt [IN] format string created with macro `BP_STRING()`
     * @param packedInput [IN] container of byte types
     * @param offset [IN] bit index to start unpacking from
     * @return tuple of results according to format string
     */
    template < typename Fmt, typename Input >
    constexpr auto unpack_from(Fmt /*unused*/, Input &&packedInput, const size_type offset)
    {
        return impl::unpack< Fmt >(std::make_index_sequence< impl::count_non_padding(Fmt{}) >(),
                                   std::forward< Input >(packedInput), offset);
    }

    template < typename Fmt, size_t... Items, typename Input >
    constexpr auto impl::unpack(std::index_sequence< Items... > /*unused*/, Input &&packedInput, const size_t start_bit)
    {
        constexpr auto byte_order = impl::get_byte_order(Fmt{});
        static_assert(byte_order == impl::Endian::big, "Unpacking little endian byte order not supported yet...");
        constexpr auto formats = impl::remove_padding(impl::get_type_array(Fmt{}));

        using FormatTypes = std::tuple< typename impl::FormatType< formats[Items].formatChar, formats[Items].count, formats[Items].endian >... >;

        const auto unpacked = std::make_tuple(
            impl::unpackElement< typename std::tuple_element_t< Items, FormatTypes > >(
                // NOLINTNEXTLINE - this is the standard implementation of std::as_bytes() from c++20
                {reinterpret_cast<const byte_type*>(std::data(packedInput)), std::size(packedInput)},
                formats[Items].offset+start_bit)...);
        return unpacked;
    }

    /**
     * Pack Args... into an array of bytes according to given format string fmt.
     * @param fmt [IN] format string created with macro `BP_STRING()`
     * @param args... [IN] list of arguments to pack into the format string
     * @return std::array of bytes holding the packed data
     */
    template < typename Fmt, typename... Args >
    constexpr auto pack(Fmt /*unused*/, Args&&... args)
    {
        std::array<byte_type, calcbytes(Fmt{})> output{};
        impl::pack< Fmt >(output, 0, std::make_index_sequence< impl::count_non_padding(Fmt{}) >(), std::forward< Args >(args)...);
        return output;
    }

    /**
     * Pack Args... into data, starting at given bit offset offset, according to given format string fmt.
     * @param fmt [IN] format string created with macro `BP_STRING()`
     * @param data [IN/OUT] reference to existing std::array of bytes to pack into
     * @param offset [IN] bit index to start unpacking from
     * @param args... [IN] list of arguments to pack into the format string
     */
    template < typename Fmt, size_type N, typename... Args >
    constexpr void pack_into(Fmt /*unused*/, std::array<byte_type, N>& data, const size_type offset, Args&&... args)
    {
        static_assert(calcbytes(Fmt{}) <= N, "bitpacker::pack_into : format larger than given array, not even counting the offset!");
        impl::pack< Fmt >(data, offset, std::make_index_sequence< impl::count_non_padding(Fmt{}) >(), std::forward< Args >(args)...);
    }

} // namespace bitpacker

#define BP_STRING(s) [] { \
    struct S : bitpacker::impl::format_string { \
      static constexpr decltype(auto) value() { return s; } \
      static constexpr bitpacker::size_type size() { return std::size(value()) - 1; }  \
      static constexpr auto at(bitpacker::size_type i) { return value()[i]; }; \
    }; \
    return S{}; \
  }()

#else
} // namespace bitpacker
#endif  // bitpacker_CPP17_OR_GREATER
