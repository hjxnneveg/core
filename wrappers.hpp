// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "entropy.hpp"

#include <type_traits>

// named arguments at home
// e.g. amp_sum(1.1, 2.2, multiplier{1.5})
// vs   amp_sum(1.1, 2.2, 3.3)

#define PRIMITIVE_WRAPPER_TYPE_(name, type)                                    \
    namespace hjx {                                                            \
    struct name {                                                              \
        type value{};                                                          \
                                                                               \
        constexpr operator type() const noexcept { return value; }             \
                                                                               \
        static void serialize(auto &a, name v) NOTHROW {                       \
            a.put(v.value);                                                    \
        }                                                                      \
                                                                               \
        static name deserialize(auto &a) noexcept {                            \
            return name(a.template get<type>());                               \
        }                                                                      \
    };                                                                         \
                                                                               \
    static_assert(std::is_trivially_copyable_v<name>);                         \
    }                                                                          \
                                                                               \
    template<>                                                                 \
    struct std::hash<hjx::name> {                                              \
        constexpr size_t operator()(hjx::name v) const noexcept {              \
            return hjx::hash(v.value);                                         \
        }                                                                      \
    };

PRIMITIVE_WRAPPER_TYPE_(multiplier, float)

#undef PRIMITIVE_WRAPPER_TYPE_
