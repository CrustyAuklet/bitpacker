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
#ifndef BITPACKER_PLATFORM_HPP
#define BITPACKER_PLATFORM_HPP

#ifndef bitpacker_CPLUSPLUS
#if defined(_MSVC_LANG) && !defined(__clang__)
#define bitpacker_CPLUSPLUS (_MSC_VER == 1900 ? 201103L : _MSVC_LANG)
#else
#define bitpacker_CPLUSPLUS __cplusplus
#endif  // defined(_MSVC_LANG) && !defined(__clang__)
#endif  // ifndef bitpacker_CPLUSPLUS

#define bitpacker_CPP98_OR_GREATER (bitpacker_CPLUSPLUS >= 199711L)
#define bitpacker_CPP11_OR_GREATER (bitpacker_CPLUSPLUS >= 201103L)
#define bitpacker_CPP14_OR_GREATER (bitpacker_CPLUSPLUS >= 201402L)
#define bitpacker_CPP17_OR_GREATER (bitpacker_CPLUSPLUS >= 201703L)
#define bitpacker_CPP20_OR_GREATER (bitpacker_CPLUSPLUS > 201703L)

// pattern from https://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations#example
#ifndef __has_include
#define __has_include(x) 0
#endif

// test if we have std::is_constant_evaluated
#if __has_include(<version>)
#include <version>
#elif __has_include(<type_traits>)
#include <type_traits>
#endif
#if __cpp_lib_is_constant_evaluated >= 201811L
#define BITPACKER_HAVE_IS_CONSTANT_EVALUTATED 1
#else
#define BITPACKER_HAVE_IS_CONSTANT_EVALUTATED 0
#endif  // end test for std::is_constant_evaluated

/// check if we have constexpr algorithms
#if __has_include(<version>)
#include <version>
#elif __has_include(<algorithm>)
#include <algorithm>
#endif
#if __cpp_lib_constexpr_algorithms >= 201806L
#define BITPACKER_CONSTEXPR_ALGO 1
#else
#define BITPACKER_CONSTEXPR_ALGO 0
#endif  // end test for constexpr algorithms

// check for c++20 bit header and operations
#if __has_include(<bit>)
#include <bit>
#endif
#if __cpp_lib_bitops >= 201907L
#define BITPACKER_BIT_OPERATIONS 1
#else
#define BITPACKER_BIT_OPERATIONS 0
#endif

// test for std::span, or find a common alternative implementation
#if __has_include(<version>)
#include <version>
#elif __has_include(<span>)
#include <span>
#endif
#if __cpp_lib_span >= 202002L
#define bitpacker_HAVE_STD_SPAN 1
#include <span>
namespace bitpacker {
    using std::as_bytes;
    using std::span;
}  // namespace bitpacker
#else
#define bitpacker_HAVE_STD_SPAN 0
#endif

// if there is no std::span find an alternative implementation
#if !bitpacker_HAVE_STD_SPAN
#if __has_include(<gsl/gsl-lite.hpp>)
#define bitpacker_HAVE_GSL_LITE_SPAN 1
#include <gsl/gsl-lite.hpp>
namespace bitpacker {
    using gsl::span;
}
#else
#define bitpacker_HAVE_GSL_LITE_SPAN 0
#endif

#if __has_include(<nonstd/span.hpp>)
#define bitpacker_HAVE_SPAN_LITE 1
#include <nonstd/span.hpp>
namespace bitpacker {
    using nonstd::span;
}
#else
#define bitpacker_HAVE_SPAN_LITE 0
#endif

#if __has_include(<gsl/gsl>)
#define bitpacker_HAVE_GSL_SPAN 1
#include <gsl/gsl>
namespace bitpacker {
    using gsl::span;
}
#else
#define bitpacker_HAVE_GSL_SPAN 0
#endif
#endif  // !bitpacker_HAVE_STD_SPAN

namespace bitpacker {

#if defined(BITPACKER_USE_STD_BYTE)
#if !bitpacker_CPP17_OR_GREATER
#error "Requested std::byte but not using C++17 or later"
#endif
    using byte_type = std::byte;
#else
    using byte_type = uint8_t;
#endif

#if BITPACKER_BIT_OPERATIONS
    constexpr bool little_endian_system = (std::endian::native == std::endian::little);
#else
    constexpr bool little_endian_system = true;
#endif

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
}  // namespace bitpacker

#endif  // BITPACKER_PLATFORM_HPP
