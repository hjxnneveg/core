// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#include "reporting.hpp"

#include <cxxabi.h>
#include <memory>
#include <string>

namespace hjx {

std::string demangle(const char *name) {
    int status = -1;

    std::unique_ptr<char, void(*)(void*)> ret {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    ASSERT_EQ(status, 0);

    return ret.get();
}

}
