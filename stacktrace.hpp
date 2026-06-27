// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "util.hpp"

//#define BOOST_STACKTRACE_USE_ADDR2LINE
#define BOOST_STACKTRACE_USE_BACKTRACE // fixme - clang
#include <boost/stacktrace.hpp>

namespace hjx {

inline std::string stacktrace() {
    return TO_STRING(boost::stacktrace::stacktrace(1, 99));
}

}
