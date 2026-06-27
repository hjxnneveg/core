// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <boost/stacktrace.hpp>

#include <exception>
#include <iostream>

#define DONT_CALL_THIS_                                                 \
    do {                                                                \
        std::cerr << "--- Don't call this ---\n"                        \
                  << boost::stacktrace::stacktrace(1, 99) << std::endl; \
        std::terminate();                                               \
    } while (false)

extern "C" inline int rand() throw() {
    DONT_CALL_THIS_;
    return 0;
}

inline void abs(auto) { DONT_CALL_THIS_; }

inline void mod(auto, auto) { DONT_CALL_THIS_; }

#undef DONT_CALL_THIS_
