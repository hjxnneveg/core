// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include <csignal>
#include <iostream>
#include <new>

// fixme - error message
// consider - placement new into static char[]

#define NEW_STATIC_OBJECT(...)                                  \
    ([] noexcept {                                              \
        auto *p = new (std::nothrow) __VA_ARGS__;               \
        if (!p) {                                               \
            std::cerr << "bad alloc" << std::endl;              \
            raise(SIGABRT);                                     \
        }                                                       \
        return p;                                               \
    }())

// fiasco leak
#define RETURN_STATIC_OBJECT(...)                               \
    do {                                                        \
        static auto *ret = NEW_STATIC_OBJECT(__VA_ARGS__);      \
        return *ret;                                            \
    } while (false)
