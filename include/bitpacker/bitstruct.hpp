//
// Created by Ethan on 10/5/2020.
//

#ifndef BITPACKER_BITSTRUCT_HPP
#define BITPACKER_BITSTRUCT_HPP

#include "bitpack.hpp"

#if bitpacker_CPP17_OR_GREATER
/***************************************************************************************************
 * TMP (compile time) pack and unpack functionality (C++17 required)
 ***************************************************************************************************/

namespace bitpacker {
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

#endif  // bitpacker_CPP17_OR_GREATER
#endif  //BITPACKER_BITSTRUCT_HPP
