// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "statics.hpp"
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


#define ONCE(...) BRIEFLY(1, __VA_ARGS__)

#define BRIEFLY(N, ...)                                                         \
    do {                                                                        \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);      \
        static unsigned MKVARNAME(doit) = N;                                    \
                                                                                \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                      \
        if (MKVARNAME(doit)) {                                                  \
            MKVARNAME(doit)--;                                                  \
            MKVARNAME(lock).unlock();                                           \
            __VA_ARGS__;                                                        \
        }                                                                       \
    } while (false)

#define ONCEPER(t, ...)                                                         \
    do {                                                                        \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);      \
        static std::chrono::steady_clock::time_point MKVARNAME(last);           \
                                                                                \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                      \
        auto MKVARNAME(now) = std::chrono::steady_clock::now();                 \
        if (MKVARNAME(now) > MKVARNAME(last) + (t)) {                           \
            MKVARNAME(last) = MKVARNAME(now);                                   \
            MKVARNAME(lock).unlock();                                           \
            __VA_ARGS__;                                                        \
        }                                                                       \
    } while (false)



#define WATCH_F(x, f)                                                           \
    do {                                                                        \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);      \
        static bool MKVARNAME(first) = true;                                    \
        static std::remove_reference_t<decltype(x)> MKVARNAME(cur){};           \
                                                                                \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                      \
        decltype(x) MKVARNAME(val) = x;                                         \
        if (MKVARNAME(first) || MKVARNAME(val) != MKVARNAME(cur)) {             \
            (f)(MKVARNAME(val));                                                \
            MKVARNAME(first) = false;                                           \
            MKVARNAME(cur) = MKVARNAME(val);                                    \
        }                                                                       \
    } while (false)

#define WATCH_RANGE(x, f)                                                               \
    do {                                                                                \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);              \
        static decltype(x) MKVARNAME(min) = std::numeric_limits<decltype(x)>::max();    \
        static decltype(x) MKVARNAME(max) = std::numeric_limits<decltype(x)>::lowest(); \
                                                                                        \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                              \
        decltype(x) MKVARNAME(val) = x;                                                 \
        bool MKVARNAME(changed) = false;                                                \
                                                                                        \
        if (MKVARNAME(val) < MKVARNAME(min)) {                                          \
            MKVARNAME(min) = MKVARNAME(val);                                            \
            MKVARNAME(changed) = true;                                                  \
        }                                                                               \
                                                                                        \
        if (MKVARNAME(val) > MKVARNAME(max)) {                                          \
            MKVARNAME(max) = MKVARNAME(val);                                            \
            MKVARNAME(changed) = true;                                                  \
        }                                                                               \
                                                                                        \
        if (MKVARNAME(changed)) (f)(MKVARNAME(min), MKVARNAME(max));                    \
    } while (false)


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
