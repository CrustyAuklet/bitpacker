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
#ifndef BITPACKER_BIT_HPP
#define BITPACKER_BIT_HPP
#include <climits>
//#include <cstddef>
#include <cstdint>
//#include <limits>
//#include <tuple>
#include <type_traits>

#include "platform.hpp"

namespace bitpacker {

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

    /**
     * Checks if the given bit offset is aligned with respect to the systems byte representation
     * shall return the equivalent of `bit_offset % CHAR_BIT == 0`
     * @param bit_offset the bit offset
     * @return true if the bit offset is aligned with respect to CHAR_BIT, false otherwise
     */
    constexpr bool is_aligned(const size_type bit_offset) noexcept {
        return (bit_offset % ByteSize) == 0;
    }

    /**
     * creates a mask for an integer where the n most significant bits are cleared
     * in other words, creates mask from the nth bit from the MSB to bit 0 (inclusive)
     * @param n the number of significant bits to clear
     * @return mask of with the n most significant bits cleared
     */
    constexpr byte_type right_mask(const size_type n) noexcept {
        return static_cast<byte_type>((1U << (ByteSize - n)) - 1);
    }

    /**
     * creates a mask for an integer where the n most significant bits are cleared.
     * in other words, creates mask from the nth bit from the MSB to bit 8 (inclusive)
     * @param n the number of significant bits to set, minus 1
     * @return mask of with the n+1 most significant bits set
     */
    constexpr byte_type left_mask(const size_type n) noexcept {
        return static_cast<byte_type>(~static_cast<byte_type>(right_mask(n) >> 1U));
    }

    /// finds the smallest fixed width unsigned integer that can fit NumBits bits
    template<size_type NumBits>
    using unsigned_type = std::conditional_t<
        NumBits <= 8, uint8_t, std::conditional_t<NumBits <= 16, uint16_t, std::conditional_t<NumBits <= 32, uint32_t, std::conditional_t<NumBits <= 64, uint64_t, void>>>>;

    /// finds the smallest fixed width signed integer that can fit NumBits bits
    template<size_type NumBits>
    using signed_type = std::conditional_t<
        NumBits <= 8, int8_t, std::conditional_t<NumBits <= 16, int16_t, std::conditional_t<NumBits <= 32, int32_t, std::conditional_t<NumBits <= 64, int64_t, void>>>>;

#pragma warning(push)
#pragma warning(disable : 4293)  // If a shift count is negative or too large, the behavior of the resulting image is undefined.
    /// sign extends an unsigned integral value to prepare for casting to a signed value.
    template<typename T, size_type BitSize>
    constexpr signed_type<BitSize> sign_extend(T val) noexcept {
        using return_type = signed_type<BitSize>;
        static_assert(std::is_unsigned<T>::value && std::is_integral<T>::value, "ValueType needs to be an unsigned integral type");
        // warning disabled for shifts bigger than type, since this if statement avoids that case.
        // if constexpr would work too, but trying to keep this section c++14 compatable
        if (BitSize < (sizeof(T) * ByteSize)) {
            const T upper_mask = static_cast<T>(~((static_cast<return_type>(1U) << BitSize) - 1));
            const T msb = static_cast<return_type>(1U) << (BitSize - 1);
            if (val & msb) {
                return static_cast<return_type>(val | upper_mask);
            }
        }
        return static_cast<return_type>(val);
    }
#pragma warning(pop)

    template<typename T>
    constexpr T reverse_bits(std::remove_cv_t<T> value);

    template<>
    constexpr uint8_t reverse_bits(uint8_t value) {
        value = ((value & 0xAAU) >> 1U) | ((value & 0x55U) << 1U);
        value = ((value & 0xCCU) >> 2U) | ((value & 0x33U) << 2U);
        value = ((value & 0xF0U) >> 4U) | ((value & 0x0FU) << 4U);
        return value;
    }

    template<>
    constexpr uint16_t reverse_bits(uint16_t value) {
        value = ((value & 0xAAAAU) >> 1U) | ((value & 0x5555U) << 1U);
        value = ((value & 0xCCCCU) >> 2U) | ((value & 0x3333U) << 2U);
        value = ((value & 0xF0F0U) >> 4U) | ((value & 0x0F0FU) << 4U);
        value = (value >> 8U) | (value << 8U);
        return value;
    }

    template<>
    constexpr uint32_t reverse_bits(uint32_t value) {
        value = ((value & 0xAAAAAAAAU) >> 1U) | ((value & 0x55555555U) << 1U);
        value = ((value & 0xCCCCCCCCU) >> 2U) | ((value & 0x33333333U) << 2U);
        value = ((value & 0xF0F0F0F0U) >> 4U) | ((value & 0x0F0F0F0FU) << 4U);
        value = ((value & 0xFF00FF00U) >> 8U) | ((value & 0x00FF00FFU) << 8U);
        value = (value >> 16U) | (value << 16U);
        return value;
    }

    template<>
    constexpr uint64_t reverse_bits(uint64_t value) {
        value = ((value & 0xAAAAAAAAAAAAAAAAU) >> 1U) | ((value & 0x5555555555555555U) << 1U);
        value = ((value & 0xCCCCCCCCCCCCCCCCU) >> 2U) | ((value & 0x3333333333333333U) << 2U);
        value = ((value & 0xF0F0F0F0F0F0F0F0U) >> 4U) | ((value & 0x0F0F0F0F0F0F0F0FU) << 4U);
        value = ((value & 0xFF00FF00FF00FF00U) >> 8U) | ((value & 0x00FF00FF00FF00FFU) << 8U);
        value = ((value & 0xFFFF0000FFFF0000U) >> 16U) | ((value & 0x0000FFFF0000FFFFU) << 16U);
        value = (value >> 32U) | (value << 32U);
        return value;
    }

    /// reverses the bits in the value `val`.
    template<typename T, size_type BitSize>
    constexpr auto reverse_bits(std::remove_cv_t<T> val) noexcept {
        using val_type = std::remove_reference_t<std::remove_cv_t<T>>;
        static_assert(std::is_integral<val_type>::value, "bitpacker::reverse_bits: val needs to be an integral type");
        using underlying_type = unsigned_type<sizeof(val_type) * ByteSize>;
        val_type retval = val;
        return reverse_bits<underlying_type>(static_cast<underlying_type>(retval)) >> (sizeof(underlying_type) * ByteSize - BitSize);
    }

}  // namespace bitpacker

#endif  // BITPACKER_BIT_HPP
