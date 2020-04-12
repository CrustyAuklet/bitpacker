#pragma once
#include <string_view>
#include <array>

template <size_t ArrSize>
constexpr bool operator==(const std::array<char, ArrSize>& arr, std::string_view str)
{
    // Assumming that str is a string literal
    if (ArrSize != str.size()) {
        return false;
    }

    for(size_t i = 0; i < ArrSize; i++) {
        if(arr[i] != str[i]) {
            return false;
        }
    }

    return true;
}

template <typename F, typename S, typename F2, typename S2>
constexpr bool operator==(const std::pair<F, S>& lhs, const std::pair<F2, S2>& rhs)
{
    return (lhs.first == static_cast<F>(rhs.first)) &&
           (lhs.second == static_cast<S>(rhs.second));
}




// Crazy workaround because std::string_view.operator== isn't constexpr...
constexpr bool equals(const std::string_view lhs, const std::string_view rhs)
{
    if(lhs.size() != rhs.size()) {
        return false;
    }

    for(size_t i = 0; i < lhs.size(); i++) {
        if(lhs[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}

template <typename Lhs, typename Rhs>
constexpr bool equals(const Lhs& lhs, const Rhs& rhs)
{
    return lhs == rhs;
}

template <typename... Lhs, typename... Rhs, size_t... Is>
constexpr bool equals(const std::tuple<Lhs...>& lhs, const std::tuple<Rhs...>& rhs, std::index_sequence<Is...>)
{
    bool res[] = { equals(std::get<Is>(lhs), std::get<Is>(rhs))... };
    for(bool b : res) {
        if(!b) {
            return false;
        }
    }

    return true;
}

template <typename... Lhs, typename... Rhs>
constexpr bool equals(const std::tuple<Lhs...>& lhs, const std::tuple<Rhs...>& rhs)
{
    if constexpr (sizeof...(Lhs) != sizeof...(Rhs)) {
        return false;
    }

    return equals(lhs, rhs, std::make_index_sequence<sizeof...(Lhs)>());
}

#ifdef WITH_CONSTEXPR_REQUIRE
#define REQUIRE_STATIC(x) REQUIRE(x); \
                          static_assert(x, #x);
#else
#define REQUIRE_STATIC(x) REQUIRE(x);
#endif
