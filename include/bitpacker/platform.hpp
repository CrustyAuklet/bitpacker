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

// pattern from https://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations#example
#ifndef __has_include
#  define __has_include(x) 0
#endif

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

// test for std::span, or find a common alternative implementation
#if __has_include(<version>)
#  include <version>
#elif __has_include(<span>)
#  include <span>
#endif
#if __cpp_lib_span >= 202002L
#  define bitpacker_HAVE_STD_SPAN 1
#else
#  define bitpacker_HAVE_STD_SPAN 0
#endif

// if there is no std::span find an alternative implementation
#if !bitpacker_HAVE_STD_SPAN
#if __has_include(<gsl/gsl-lite.hpp>)
#define bitpacker_HAVE_GSL_LITE_SPAN 1
#else
#define bitpacker_HAVE_GSL_LITE_SPAN 0
#endif

#if __has_include(<nonstd/span.hpp>)
#define bitpacker_HAVE_SPAN_LITE 1
#else
#define bitpacker_HAVE_SPAN_LITE 0
#endif

#if __has_include(<gsl/gsl>)
#define bitpacker_HAVE_GSL_SPAN 1
#else
#define bitpacker_HAVE_GSL_SPAN 0
#endif

#endif  // !bitpacker_HAVE_STD_SPAN

// test if ew have std::is_constant_evaluated
#if __has_include(<version>)
#  include <version>
#if __cpp_lib_is_constant_evaluated >= 201811L
#  define bitpacker_HAVE_IS_CONSTANT_EVALUTATED 1
#else
#  define bitpacker_HAVE_IS_CONSTANT_EVALUTATED 0
#endif  // __cpp_lib_is_constant_evaluated >= 201811L
#endif  // __has_include(<version>)


#endif  //BITPACKER_PLATFORM_HPP
