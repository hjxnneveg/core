// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "util.hpp"
#include "statics.hpp"

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace hjx {

inline auto thread_id() { return std::this_thread::get_id(); }

struct threadstuff_t {
    std::unordered_map<std::thread::id, string> names;
    int count = 0;
    std::mutex mut;

    void reg_threadname(const char *name) {
        std::lock_guard _(mut);
        names[thread_id()] = name;
    }

    const char *threadname() {
        std::lock_guard _(mut);
        string &name = names[thread_id()];
        if (name.size() == 0) name = string("t") + TO_STRING(++count);
        return name.c_str();
    }
};

inline threadstuff_t &g_threadstuff() {
    RETURN_STATIC_OBJECT(threadstuff_t);
}

inline void reg_threadname(const char *name) {
    g_threadstuff().reg_threadname(name);
}

inline const char *threadname() { return g_threadstuff().threadname(); }


}
