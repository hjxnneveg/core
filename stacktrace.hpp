// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "ckillers.hpp"
#include "util.hpp"

namespace hjx {

inline std::string stacktrace() {
    return TO_STRING(boost::stacktrace::stacktrace(1, 99));
}

}
