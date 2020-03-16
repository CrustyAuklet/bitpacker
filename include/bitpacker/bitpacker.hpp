#pragma once

#include <cstdint>
#include <cstddef>
#include <climits>
#include <type_traits>

#include "span-lite.hpp"

namespace bitpacker {
    using nonstd::span;
    using nonstd::as_bytes;

    namespace {
        using byte_type = uint8_t;
        constexpr unsigned ByteSize = sizeof(byte_type) * CHAR_BIT;

        static_assert(CHAR_BIT == 8, "The target system has bytes that are not 8 bits!");
        static_assert(static_cast<unsigned>(-1) == ~0U, "The target system is not 2's compliment! Default pack specializations will not work!");

        struct Offset {
            std::size_t byte;
            std::size_t bit;
        };

        bool operator==(const Offset& lhs, const Offset& rhs) {
            return lhs.byte == rhs.byte && lhs.bit == rhs.bit;
        }
        bool operator!=(const Offset& lhs, const Offset& rhs) {
            return !(lhs == rhs);
        }

        /// get the byte and bit offset of a given bit number
        constexpr Offset get_offset(std::size_t offset) noexcept {
            return { (offset / ByteSize), (offset % ByteSize) };
        }
        /// create mask from the nth bit from the MSB to bit 0 (inclusive)
        constexpr byte_type right_mask(const std::size_t n) noexcept {
            return static_cast<byte_type>((1U << (8 - n)) - 1);
        }
        /// create mask from the nth bit from the MSB to bit 8 (inclusive)
        constexpr byte_type left_mask(const std::size_t n) noexcept {
            return static_cast<byte_type>(~(right_mask(n) >> 1U));
        }


        template<size_t Bytes>
        struct unsigned_integer;

        template<>
        struct unsigned_integer<1> {
            using type = uint8_t;
        };

        template<>
        struct unsigned_integer<2> {
            using type = uint16_t;
        };

        template<>
        struct unsigned_integer<4> {
            using type = uint32_t;
        };

        template<>
        struct unsigned_integer<8> {
            using type = uint64_t;
        };

        template<typename T>
        using integer_type = std::conditional_t<std::is_signed_v<T>,
                typename std::make_signed_t<typename unsigned_integer<sizeof(T)>::type>,
                typename unsigned_integer<sizeof(T)>::type>;
    }   // implementation namespace

    template<typename ValueType>
    constexpr void pack_into(span<byte_type> buffer, std::size_t offset, std::size_t size, ValueType v) noexcept {
        static_assert( std::is_unsigned_v<ValueType> && std::is_integral_v<ValueType>, "ValueType needs to be an unsigned integral type");
        const auto start = get_offset(offset);
        const auto end   = get_offset(offset + size - 1);
        const uint8_t startMask   = right_mask(start.bit);    // mask of the start byte, 1s where data is
        const uint8_t endMask     = left_mask(end.bit);       // mask of the end byte, 1s where data is

        // mask off any bits outside the size of the actual field, if size < bits in ValueType
        // NOTE: it is UB to left shift ANY value if the shift is >= the bits in the value!
        // this also takes care of zero size values.
        if( size < sizeof(ValueType)*ByteSize ) {
            v &= static_cast<ValueType>(( ValueType{0x1U} << size) - 1);
        }

        if (start.byte == end.byte) {
            // case where start and end are in the same byte
            buffer[start.byte] &= static_cast<byte_type>(~(startMask & endMask));
            buffer[start.byte] |= static_cast<byte_type>(v << (ByteSize - (end.bit + 1)));
        }
        else {
            // case where start and end are in different bytes
            buffer[end.byte] &= static_cast<byte_type>(~endMask);
            // TODO: simpler way to get shift. byte aligned data is a special case (%chunksize and the ternary)
            buffer[end.byte] |= static_cast<byte_type>(v << (ByteSize - ((end.bit+1)%ByteSize) ) % ByteSize);
            v >>= ((end.bit+1)%ByteSize) != 0 ? (end.bit+1)%ByteSize : ByteSize;

            for (std::size_t i = end.byte - 1; i > start.byte; --i) {
                buffer[i] = static_cast<byte_type>(v);
                v >>= ByteSize;
            }

            buffer[start.byte] &= static_cast<byte_type>(~startMask);
            buffer[start.byte] |= static_cast<byte_type>(v);
        }
    }

    template<typename ReturnType>
    constexpr ReturnType unpack_from(span<const byte_type> buffer, std::size_t offset, std::size_t size) noexcept {
        static_assert( std::is_unsigned_v<ReturnType> && std::is_integral_v<ReturnType>, "ReturnType needs to be an unsigned integral type");
        const auto start = get_offset(offset);
        const auto end   = get_offset(offset + size - 1);

        // case where size is zero
        if (size == 0) {
            return 0;
        }

        // case where the the entire field is in one byte
        if (start.byte == end.byte) {
            const auto shift = (ByteSize - (end.bit + 1));
            const auto mask  = (1u << size) - 1;
            return static_cast<ReturnType>( (static_cast<uint8_t>(buffer[start.byte]) >> shift) & mask );
        }

        // case where the field covers 2 or more bytes
        ReturnType value = static_cast<uint8_t>(buffer[start.byte]) & right_mask(start.bit);
        for (std::size_t i = start.byte + 1; i < end.byte; ++i) {
            value = static_cast<ReturnType>((value << ByteSize) | static_cast<uint8_t>(buffer[i]));
        }
        const ReturnType shifted_end   = static_cast<ReturnType>(static_cast<uint8_t>(buffer[end.byte]) >> (ByteSize - (end.bit+1)));
        const ReturnType shifted_value = static_cast<ReturnType>(value << (end.bit + 1));
        return shifted_value | shifted_end;
    }

    template <typename T>
    constexpr T get(span<const byte_type> buffer, std::size_t offset) noexcept;

    template <>
    constexpr uint8_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return unpack_from<uint8_t>(buffer, offset, 8);
    }

    template <>
    constexpr int8_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return static_cast<int8_t>(unpack_from<uint8_t>(buffer, offset, 8));
    }

    template <>
    constexpr uint16_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return unpack_from<uint16_t>(buffer, offset, 16);
    }

    template <>
    constexpr int16_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return static_cast<int16_t>(unpack_from<uint16_t>(buffer, offset, 16));
    }

    template <>
    constexpr uint32_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return unpack_from<uint32_t>(buffer, offset, 32);
    }

    template <>
    constexpr int32_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return static_cast<int32_t>(unpack_from<uint32_t>(buffer, offset, 32));
    }

    template <>
    constexpr uint64_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return unpack_from<uint64_t>(buffer, offset, 64);
    }

    template <>
    constexpr int64_t get(span<const byte_type> buffer, std::size_t offset) noexcept {
        return static_cast<int64_t>(unpack_from<uint64_t>(buffer, offset, 64));
    }

    /*************************  Template specialization for packing  ****************************/

    template <typename T>
    constexpr void store(span<byte_type> buffer, std::size_t offset, T value) noexcept;

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, uint8_t value) noexcept {
        pack_into(buffer, offset, ByteSize, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, int8_t value) noexcept {
        pack_into(buffer, offset, ByteSize, static_cast<uint8_t>(value));
    }

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, uint16_t value) noexcept {
        pack_into(buffer, offset, 16, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, int16_t value) noexcept {
        pack_into(buffer, offset, 16, static_cast<uint16_t>(value));
    }

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, uint32_t value) noexcept {
        pack_into(buffer, offset, 32, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, int32_t value) noexcept {
        pack_into(buffer, offset, 32, static_cast<uint32_t>(value));
    }

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, uint64_t value) noexcept {
        pack_into(buffer, offset, 64, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, std::size_t offset, int64_t value) noexcept {
        pack_into(buffer, offset, 64, static_cast<uint64_t>(value));
    }

    template <typename ElementType>
    class BitStream {
        std::size_t m_pos;
        span <ElementType> m_buff;
    public:
        using element_type = ElementType;
        using reference = ElementType&;
        using pointer = ElementType*;
        using const_pointer = const ElementType*;
        using const_reference = const ElementType&;

        using size_type = std::size_t;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator< iterator >;
        using const_reverse_iterator = std::reverse_iterator< const_iterator >;

        BitStream() = delete;
        constexpr BitStream(const BitStream& other) = default;
        constexpr BitStream(BitStream&& other) = default;
        constexpr BitStream& operator=( const BitStream& other) noexcept = default;
        constexpr BitStream& operator=(BitStream&& other) noexcept = default;
        ~BitStream() = default;

        template <std::size_t N>
        constexpr BitStream( std::array<element_type, N>& arr ) noexcept : m_pos{}, m_buff{arr}
        {}

        template <std::size_t N>
        constexpr BitStream( const std::array<element_type, N>& arr ) noexcept : m_pos{}, m_buff{arr}
        {}

        template <std::size_t N>
        constexpr BitStream( element_type (&arr)[N] ) noexcept : m_pos{}, m_buff{arr}
        {}

        /**
         *
         * @return
         */
        [[nodiscard]] constexpr const_pointer data() const noexcept {
            return m_buff.data();
        }

        /**
         *
         * @param pos
         * @return
         */
        constexpr void seek(const size_type pos) {
            m_pos = pos;
        }

        /**
         *
         * @return
         */
        [[nodiscard]] constexpr size_type size() const noexcept {
            return m_buff.size_bytes() * ByteSize;
        }

        /**
         *
         * @return
         */
        [[nodiscard]] constexpr size_type position() const noexcept {
            return m_pos;
        }

        /**
         *
         * @param n
         * @return
         */
        constexpr void pad_to(const unsigned n) {
            static_assert(!std::is_const_v<ElementType>, "BitStreamWriter data must be non-constant");
            m_pos += ((n*8)-(m_pos % (n*8)));
        }

        /**
         *
         * @tparam ValueType
         * @param start
         * @param bits
         * @return
         */
        template<typename ValueType>
        constexpr ValueType unpack_field(const size_type start, const size_type bits) const {
            return unpack_from<ValueType>(m_buff, start, bits);
        }

        /**
         *
         * @tparam ValueType
         * @param start
         * @param bits
         * @param value
         * @return
         */
        template<typename ValueType>
        constexpr void pack_field(const size_type start, const size_type bits, const ValueType value) {
            static_assert(!std::is_const_v<ElementType>, "BitStreamWriter data must be non-constant");
            pack_into(m_buff, start, bits, static_cast< std::make_unsigned_t<ValueType> >(value));
        }

    };

} // namespace bitpacker