// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "time.hpp"
#include "uptime.hpp"
#include "threadname.hpp"
#include "util.hpp"

#include <cstring>
#include <iostream>
#include <sstream>
#include <type_traits>

#define DUMPI(...)                                                                   \
    do {                                                                             \
        std::ostringstream MKVARNAME(os);                                            \
        hjx::dump_prefix(MKVARNAME(os), __FILE__, __LINE__) << __VA_ARGS__ << '\n';  \
        std::cout << MKVARNAME(os).str() << std::flush;                              \
    } while (false)

#define DUMPV(...) DUMPI(#__VA_ARGS__ << ": " << __VA_ARGS__)

#define DUMPC(...)                                                              \
    do {                                                                        \
        if (!std::is_constant_evaluated()) DUMPI(__VA_ARGS__);                  \
    } while (false)

#define DUMP(...) dumper MKVARNAME(dumper)(__FILE__, __LINE__, TO_STRING(__VA_ARGS__))

#define DUMPF DUMP(__FUNCTION__)


#define TIME(...)                                                       \
    do {                                                                \
        auto MKVARNAME(start) = clock::now();                           \
        __VA_ARGS__;                                                    \
        DUMPI(clock::now() - MKVARNAME(start) << "  " << #__VA_ARGS__); \
    } while (false)


#define LOG0(...)                                               \
    do {                                                        \
        std::ostringstream MKVARNAME(os);                       \
        MKVARNAME(os) << "LOG" << time::uptime();               \
        MKVARNAME(os) << "(" __FILE__ ":" << __LINE__ << ") ";  \
        MKVARNAME(os) << __VA_ARGS__ << "\n";                   \
        std::clog << MKVARNAME(os).str() << std::flush;         \
    } while (false)

#define LOG_BLAND      0x0
#define LOG_TIMESTAMP  0x1
#define LOG_THREADNAME 0x2

#define DEFINE_LOG(name, enabled, flags)                                \
    inline bool LOGDEF_enabled_##name = (enabled);                      \
    inline bool LOGDEF_timestamp_##name = (flags) & LOG_TIMESTAMP;      \
    inline bool LOGDEF_threadname_##name = (flags) & LOG_THREADNAME;

#define LOG(name, x)                                                            \
    do {                                                                        \
        if (LOGDEF_enabled_##name) {                                            \
            std::ostringstream MKVARNAME(os);                                   \
            MKVARNAME(os) << "LOG_" #name;                                      \
            if (LOGDEF_timestamp_##name)                                        \
                MKVARNAME(os) << hjx::time::uptime();                           \
            if (LOGDEF_threadname_##name)                                       \
                MKVARNAME(os) << "[" << hjx::threadname() << "]";               \
            MKVARNAME(os) << "(" << hjx::log::trim_path(__FILE__) << ":"        \
                          << __LINE__ << ") " << x << "\n";                     \
            std::clog << MKVARNAME(os).str() << std::flush;                     \
        }                                                                       \
    } while (false)

#define ENABLE_LOG(name, yes) (LOGDEF_enabled_##name = yes)

//#define AUTO_ENABLE_LOG(name, yes) todo

namespace hjx {

namespace log {

inline const char *trim_path(const char *path) {
    const char *filename = std::strrchr(path, '/'); // fixme - windows
    return filename ? filename + 1 : path;
}

}

inline std::ostream &dump_prefix(std::ostream &os, const char *file, int line);

struct dumper {
    inline static int depth = 0;
    clock::time_point start;
    const char *file;
    int line;

    dumper(const char *file, int line, const string &msg):
        start(clock::now()), file(file), line(line)
    {
        std::ostringstream os;
        dump_prefix(os, file, line) << msg << " {\n";
        std::cout << os.str() << std::flush;
        depth++;
    }

    ~dumper() {
        depth--;
        std::ostringstream os;
        dump_prefix(os, file, line) << "} " << (clock::now() - start) << "\n";
        std::cout << os.str() << std::flush;
    }

    dumper(const dumper&) = delete;
    dumper &operator=(const dumper&) = delete;
};

inline std::ostream &dump_prefix(std::ostream &os, const char *file, int line) {
    os << file << ":";
    os << line << "(";
    os << threadname() << ") ";
    for (int i = 0; i < dumper::depth; i++) os << "- ";

    return os;
}


string demangle(const char *name);

template <typename T>
string type2str() { return demangle(typeid(T).name()); }

template <typename T>
string type2str(const T&) { return type2str<T>(); }

template <typename T>
string type2str(const T& v, bool pv) {
    std::ostringstream ss;
    ss << type2str<T>();
    if (pv) ss << " " << v;
    return ss.str();
}

}
