// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua Marshall

#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <utility>

namespace hjx {

#define FWD(name) std::forward<decltype(name)>(name)

#define IS_CALLABLE(f, T) IS_CALLABLE_DEF(f).operator()<T>()

#define IS_CALLABLE_DEF(f) \
    []<typename T>() { return requires { f(std::declval<T>()); }; }

template<typename F, typename R, typename... Args>
concept invocable_r = std::is_invocable_r_v<R, F, Args...>;

template <typename T>
concept standard_integral =
    std::same_as<std::remove_cv_t<T>, signed char> ||
    std::same_as<std::remove_cv_t<T>, unsigned char> ||
    std::same_as<std::remove_cv_t<T>, short> ||
    std::same_as<std::remove_cv_t<T>, unsigned short> ||
    std::same_as<std::remove_cv_t<T>, int> ||
    std::same_as<std::remove_cv_t<T>, unsigned int> ||
    std::same_as<std::remove_cv_t<T>, long> ||
    std::same_as<std::remove_cv_t<T>, unsigned long> ||
    std::same_as<std::remove_cv_t<T>, long long> ||
    std::same_as<std::remove_cv_t<T>, unsigned long long>;

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>; // fixme - remove

template <typename T>
concept standard_arithmetic =
    std::same_as<std::remove_cv_t<T>, float> ||
    std::same_as<std::remove_cv_t<T>, double> ||
    std::same_as<std::remove_cv_t<T>, long double> ||
    standard_integral<T>;

template<typename T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

template<typename T> concept unsigned64 = std::same_as<T, uint64_t>;


template <arithmetic T>
T MAXVAL(const T&) { return std::numeric_limits<T>::max(); }


namespace cmp {

#define CMP_FUNC_(name, cmp, op)                                                \
    constexpr bool name(const standard_integral auto &a,                        \
                        const standard_integral auto &b) {                      \
        return std::cmp(a, b);                                                  \
    }                                                                           \
                                                                                \
    constexpr bool name(const auto &a, const auto &b) { return a op b; }

CMP_FUNC_(eq, cmp_equal, ==)
CMP_FUNC_(ne, cmp_not_equal, !=)
CMP_FUNC_(ge, cmp_greater_equal, >=)
CMP_FUNC_(gt, cmp_greater, >)
CMP_FUNC_(le, cmp_less_equal, <=)
CMP_FUNC_(lt, cmp_less, <)

#undef CMP_FUNC_

}

}
