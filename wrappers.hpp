// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include <type_traits>

// named arguments at home
// e.g. amp_sum(1.1, 2.2, multiplier{1.5})
// vs   amp_sum(1.1, 2.2, 3.3)

#define PRIMITIVE_WRAPPER_TYPE_(name, type)                     \
    struct name {                                               \
        type value{};                                           \
                                                                \
        constexpr operator type() const { return value; }       \
                                                                \
        static void serialize(auto &a, name v) {                \
            a.put(v.value);                                     \
        }                                                       \
                                                                \
        static name deserialize(auto &a) {                      \
            return name(a.template get<type>());                \
        }                                                       \
    };                                                          \
                                                                \
    static_assert(std::is_trivially_copyable_v<name>);


namespace hjx {

PRIMITIVE_WRAPPER_TYPE_(multiplier, float)

}

#undef PRIMITIVE_WRAPPER_TYPE_
