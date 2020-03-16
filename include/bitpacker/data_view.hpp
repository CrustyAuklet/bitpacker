#pragma once

#include <cstdint>
#include <cstddef>
//#include <type_traits>
//#include <algorithm>
#include "span.hpp"

struct data_view {
    typedef std::size_t index_type;

    typedef T element_type;
    typedef typename std::remove_cv< T >::type value_type;

    typedef T & reference;
    typedef T * pointer;
    typedef T const * const_pointer;
    typedef T const & const_reference;

    typedef pointer       iterator;
    typedef const_pointer const_iterator;

    typedef std::reverse_iterator< iterator >       reverse_iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

    typedef typename std::iterator_traits< iterator >::difference_type difference_type;

    constexpr data_view() : bytes{}

    nonstd::span<std::byte> bytes;
};