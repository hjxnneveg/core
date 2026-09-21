// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

/// no project includes

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <ostream>
#include <string_view>
#include <sstream>
#include <type_traits>


#define COMMA ,

#define HJX_UNGROUP(...) __VA_ARGS__

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)

#define MKNAME3(name, line) name ## _ ## line ## _
#define MKNAME2(name, line) MKNAME3(name, line)
#define MKNAME(name) MKNAME2(name, __LINE__)

#define MKVARNAME MKNAME

// fixme - doesn't work for STL temporaries
#define TO_STRING(x) (static_cast<std::ostringstream&&>(std::ostringstream() << x).str())


// HJX_APPLY_NONEMPTY(f, (args))        => f(args)
// HJX_APPLY_NONEMPTY(f, ())            =>
// HJX_APPLY_NONEMPTY(f, (args), ;)     => f(args) ;
// HJX_APPLY_NONEMPTY(f, (args), COMMA) => f(args) ,
// HJX_APPLY_NONEMPTY(f, (args), ,)     => f(args) ,  # heh
// HJX_APPLY_NONEMPTY(f, (), ;)         =>            # separator suppressed

#define HJX_APPLY_NONEMPTY(f, group, ...) \
    HJX_APPLY_NONEMPTY1(f, (__VA_ARGS__), HJX_UNGROUP group)

#define HJX_APPLY_NONEMPTY1(f, sep, ...) \
    __VA_OPT__(f(__VA_ARGS__) HJX_UNGROUP sep)


namespace hjx {

using std::string;
using std::string_view;
using std::vector;

}
