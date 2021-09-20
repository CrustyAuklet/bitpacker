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
#ifndef BITPACKER_BITPACK_HPP
#define BITPACKER_BITPACK_HPP
#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>

#include "platform.hpp"
#include "bit.hpp"

namespace bitpacker {

    namespace impl {

        struct Offset {
            constexpr Offset(size_type bit_offset) : byte(bit_offset / ByteSize), bit(bit_offset % ByteSize) {}
            size_type byte;
            size_type bit;
        };

        /// returns true if the offset is byte aligned
        constexpr bool is_aligned(const Offset offset) noexcept {
            return offset.bit == 0;
        }

    }

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
        static_assert(std::is_unsigned<ValueType>::value && std::is_integral<ValueType>::value, "bitpacker::insert : ValueType needs to be an unsigned integral type");
        const auto start = impl::Offset(offset);
        const auto end = impl::Offset(offset + size - 1);
        const byte_type startMask = right_mask(start.bit);  // mask of the start byte, 1s where data is
        const byte_type endMask = left_mask(end.bit);       // mask of the end byte, 1s where data is

        // mask off any bits outside the size of the actual field, if size < bits in ValueType
        // NOTE: it is UB to left shift ANY value if the shift is >= the bits in the value!
        // this also takes care of zero size values.
        if (size < sizeof(ValueType) * ByteSize) {
            v &= static_cast<ValueType>((ValueType{0x1U} << size) - 1);
        }

        if (start.byte == end.byte) {
            // case where start and end are in the same byte
            buffer[start.byte] &= static_cast<byte_type>(~(static_cast<uint8_t>(startMask & endMask)));
            buffer[start.byte] |= static_cast<byte_type>(v << (ByteSize - (end.bit + 1)));
        }
        else {
            // case where start and end are in different bytes
            buffer[end.byte] &= static_cast<byte_type>(~endMask);
            // TODO: simpler way to get shift. byte aligned data is a special case (%ByteSize and the ternary)
            buffer[end.byte] |= static_cast<byte_type>(v << (ByteSize - ((end.bit + 1) % ByteSize)) % ByteSize);
            v >>= ((end.bit + 1) % ByteSize) != 0 ? (end.bit + 1) % ByteSize : ByteSize;

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
        static_assert(std::is_unsigned<ReturnType>::value && std::is_integral<ReturnType>::value, "ReturnType needs to be an unsigned integral type");
        const auto start = impl::Offset(offset);
        const auto end = impl::Offset(offset + size - 1);

        // case where size is zero
        if (size == 0) {
            return 0;
        }

        // case where the entire field is in one byte
        if (start.byte == end.byte) {
            const size_type shift = (ByteSize - (end.bit + 1));
            // NOLINTNEXTLINE - size will always be <= 8 if we are within a byte!
            const size_type mask = (1u << size) - 1;
            return static_cast<ReturnType>(static_cast<uint8_t>((buffer[start.byte]) >> shift) & mask);
        }

        // case where the field covers 2 or more bytes
        ReturnType value = static_cast<uint8_t>(buffer[start.byte]) & static_cast<uint8_t>(right_mask(start.bit));
        for (size_type i = start.byte + 1; i < end.byte; ++i) {
            value = static_cast<ReturnType>(static_cast<ReturnType>(value << ByteSize) | static_cast<uint8_t>(buffer[i]));
        }
        const ReturnType shifted_end = static_cast<ReturnType>(static_cast<uint8_t>(buffer[end.byte]) >> (ByteSize - (end.bit + 1)));
        const ReturnType shifted_value = static_cast<ReturnType>(value << (end.bit + 1));
        return shifted_value | shifted_end;
    }

    /************************  Template specialization for unpacking  ***************************/

    template<typename T>
    constexpr T get(span<const byte_type> buffer, size_type offset) noexcept;

    /*************************  Template specialization for packing  ****************************/

    template<typename T>
    constexpr void store(span<byte_type> buffer, size_type offset, T value) noexcept;

}  // namespace bitpacker

#endif  // BITPACKER_BITPACK_HPP