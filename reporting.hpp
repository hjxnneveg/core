// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "stacktrace.hpp"
#include "asserts.hpp"
#include "threadname.hpp"
#include "typeutils.hpp"

#include <cmath>
#include <csignal>
#include <iostream>
#include <sstream>
#include <utility>

// +------------+--------+--------+
// |macro\build |debug   |release |
// +------------+--------+--------+
// |ASSERT      |throw   |off     |
// |ERROR       |throw   |raise   |
// |TEST        |throw   |throw   |
// |VERIFY      |throw   |log     |
// |WARNING     |log     |???     | compile-time warning level?
// +------------+--------+--------+
//
// PROVE and REPORT are internal machinery

#define REPORT_WARN  1
#define REPORT_THROW 2
#define REPORT_RAISE 3

#define REPORT_INTERNAL(msg, file, line, report)                               \
    do {                                                                       \
        std::ostringstream MKVARNAME(ss);                                      \
        MKVARNAME(ss) <<                                                       \
            (report == REPORT_WARN ? "\nWARNING:" : "\nERROR:");               \
        MKVARNAME(ss) << ::hjx::threadname() << ": " << msg << "\n";           \
        MKVARNAME(ss) << "Line " << line << ": " << file << "\n";              \
                                                                               \
        if (report != REPORT_WARN && ::hjx::g_stacktrace)                      \
            MKVARNAME(ss) << ::hjx::stacktrace();                              \
                                                                               \
        ::hjx::g_err() << MKVARNAME(ss).str() << std::flush;                   \
                                                                               \
        if constexpr (report == REPORT_RAISE) {                                \
            raise(SIGABRT);                                                    \
            std::unreachable();                                                \
        }                                                                      \
                                                                               \
        if constexpr (report == REPORT_THROW) throw "Boom";                    \
    } while (false)

#define REPORT_MSG(msg, report) REPORT_INTERNAL(msg, __FILE__, __LINE__, report)

#define PROVE_MSG(x, msg, report)               \
    do {                                        \
        if (!(x)) REPORT_MSG(msg, report);      \
    } while (false)

#define CMP_EQ ::hjx::cmp::eq, "==", "!="
#define CMP_NE ::hjx::cmp::ne, "!=", "=="
#define CMP_LT ::hjx::cmp::lt, "<",  "≮"
#define CMP_GT ::hjx::cmp::gt, ">",  "≯"
#define CMP_LE ::hjx::cmp::le, "<=", "≰"
#define CMP_GE ::hjx::cmp::ge, ">=", "≱"

#define PROVE_COMPARE(x, xs, y, ys, cmp, report) \
    PROVE_COMPARE1(x, xs, y, ys, cmp, report)

#define PROVE_COMPARE1(x, rx, y, ry, f, op, trouble, report)                    \
    do {                                                                        \
        auto MKVARNAME(xa) = x;                                                 \
        auto MKVARNAME(ya) = y;                                                 \
        PROVE_MSG((f)(MKVARNAME(xa), MKVARNAME(ya)),                            \
                  TO_STRING("Want " rx " " op " " ry << ", but " <<             \
                            MKVARNAME(xa) << " " trouble " " << MKVARNAME(ya)), \
                  report);                                                      \
    } while (false)

#define PROVE_GE_LE(v, low, high, report)                                       \
    do {                                                                        \
        const auto &MKVARNAME(testval) = v;                                     \
        PROVE_COMPARE(MKVARNAME(testval), #v, low, #low, CMP_GE, report);       \
        PROVE_COMPARE(MKVARNAME(testval), #v, high, #high, CMP_LE, report);     \
    } while (false)

#define PROVE_GE_LT(v, low, high, report)                                       \
    do {                                                                        \
        const auto &MKVARNAME(testval) = v;                                     \
        PROVE_COMPARE(MKVARNAME(testval), #v, low, #low, CMP_GE, report);       \
        PROVE_COMPARE(MKVARNAME(testval), #v, high, #high, CMP_LT, report);     \
    } while (false)

#define PROVE_GT_LE(v, low, high, report)                                       \
    do {                                                                        \
        const auto &MKVARNAME(testval) = v;                                     \
        PROVE_COMPARE(MKVARNAME(testval), #v, low, #low, CMP_GT, report);       \
        PROVE_COMPARE(MKVARNAME(testval), #v, high, #high, CMP_LE, report);     \
    } while (false)

#define PROVE_GT_LT(v, low, high, report)                                       \
    do {                                                                        \
        const auto &MKVARNAME(testval) = v;                                     \
        PROVE_COMPARE(MKVARNAME(testval), #v, low, #low, CMP_GT, report);       \
        PROVE_COMPARE(MKVARNAME(testval), #v, high, #high, CMP_LT, report);     \
    } while (false)

#define PROVE_IMPLY(ifcond, thencond, report)                           \
    do {                                                                \
        auto MKVARNAME(ifa) = ifcond;                                   \
        if (MKVARNAME(ifa)) {                                           \
            auto MKVARNAME(thena) = thencond;                           \
            PROVE_MSG(MKVARNAME(thena),                                 \
                      #ifcond " = " << MKVARNAME(ifa) <<                \
                      ", but " #thencond " = " << MKVARNAME(thena),     \
                      report);                                          \
        }                                                               \
    } while (false)

#define PROVE_XOR(x, y, report)                                                \
    do {                                                                       \
        auto MKVARNAME(xa) = x;                                                \
        auto MKVARNAME(ya) = y;                                                \
        PROVE_MSG(bool(MKVARNAME(xa)) + bool(MKVARNAME(ya)) == 1,              \
                  "XOR failure: " #x " = " << MKVARNAME(xa)                    \
                  << ", " #y " = " << MKVARNAME(ya),                           \
                  report);                                                     \
    } while (false)

#define PROVE_NOT(x, report)                                                         \
    do {                                                                             \
        auto MKVARNAME(xa) = x;                                                      \
        PROVE_MSG(!MKVARNAME(xa),                                                    \
                  "Want !" #x ", but " << MKVARNAME(xa) << " isn't false", report);  \
    } while (false)

#define PROVE_EQFISH(a, b, report)                                                   \
    do {                                                                             \
        float MKVARNAME(af) = a;                                                     \
        float MKVARNAME(bf) = b;                                                     \
        float MKVARNAME(bigger) = std::max(std::fabs(MKVARNAME(af)),                 \
                                           std::fabs(MKVARNAME(bf)));                \
        float MKVARNAME(diff) = std::fabs(MKVARNAME(bf) - MKVARNAME(af));            \
        PROVE_MSG(MKVARNAME(diff) <= std::max(1e-5f * MKVARNAME(bigger), 1e-7f),     \
                  "Want " #a " ≈ " #b ", but " << MKVARNAME(af)                      \
                  << " ≉ " << MKVARNAME(bf) << " (diff " << MKVARNAME(diff) << ")",  \
                  report);                                                           \
    } while (false)


#if ASSERTS_ON

#define ASSERT(x) ASSERT_MSG(x, #x)
#define ASSERT_MSG(x, msg) PROVE_MSG(x, msg, REPORT_THROW)
#define DASSERT(x, msg) PROVE_MSG(x, msg, REPORT_RAISE)

#define ASSERT_EQ(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_EQ, REPORT_THROW)
#define ASSERT_NE(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_NE, REPORT_THROW)
#define ASSERT_LT(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_LT, REPORT_THROW)
#define ASSERT_GT(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_GT, REPORT_THROW)
#define ASSERT_LE(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_LE, REPORT_THROW)
#define ASSERT_GE(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_GE, REPORT_THROW)

#define ASSERT_GE_LE(v, low, high) PROVE_GE_LE(v, low, high, REPORT_THROW)
#define ASSERT_GE_LT(v, low, high) PROVE_GE_LT(v, low, high, REPORT_THROW)
#define ASSERT_GT_LE(v, low, high) PROVE_GT_LE(v, low, high, REPORT_THROW)
#define ASSERT_GT_LT(v, low, high) PROVE_GT_LT(v, low, high, REPORT_THROW)

#define ASSERT_IMPLY(ifcnd, thencnd) PROVE_IMPLY(ifcnd, thencnd, REPORT_THROW)
#define ASSERT_XOR(x, y) PROVE_XOR(x, REPORT_THROW)
#define ASSERT_NOT(x) PROVE_NOT(x, REPORT_THROW)
#define ASSERT_EQFISH(a, b) PROVE_EQFISH(a, b, REPORT_THROW)

#else

#define ASSERT(x)
#define ASSERT_MSG(x, msg)
#define DASSERT(x, msg)

#define ASSERT_EQ(x, y)
#define ASSERT_NE(x, y)
#define ASSERT_LT(x, y)
#define ASSERT_GT(x, y)
#define ASSERT_LE(x, y)
#define ASSERT_GE(x, y)

#define ASSERT_GE_LE(low, v, high)
#define ASSERT_GE_LT(low, v, high)
#define ASSERT_GT_LE(low, v, high)
#define ASSERT_GT_LT(low, v, high)

#define ASSERT_GE_LE(v, low, high)
#define ASSERT_GE_LT(v, low, high)
#define ASSERT_GT_LE(v, low, high)
#define ASSERT_GT_LT(v, low, high)

#define ASSERT_IMPLY(ifcond, thencond)
#define ASSERT_XOR(x, y)
#define ASSERT_NOT(x)
#define ASSERT_EQFISH(a, b)

#endif


#if ASSERTS_ON
#define VERIFY(x, msg, otherwise) PROVE_MSG(x, msg, REPORT_THROW)
#else
// NB: no do-while-false pattern to allow break/continue
#define VERIFY(x, msg, otherwise)                                 \
    {                                                             \
        bool MKVARNAME(success) = bool(x);                        \
        PROVE_MSG(MKVARNAME(success), msg, REPORT_WARN);          \
        if (!MKVARNAME(success)) { otherwise; }                   \
    }
#endif

#if ASSERTS_ON
#define ERROR(msg) REPORT_MSG(msg, REPORT_THROW)
#else
#define ERROR(msg) REPORT_MSG(msg, REPORT_RAISE)
#endif

#define DERROR(msg) REPORT_MSG(msg, REPORT_RAISE)

#define WARNING(msg) REPORT_MSG(msg, REPORT_WARN)

#define SOFTWARN_GROWTH(container, softlimit)                                   \
    do {                                                                        \
        const auto &MKVARNAME(cn) = container;                                  \
        size_t MKVARNAME(sl) = softlimit;                                       \
        size_t MKVARNAME(sz) = MKVARNAME(cn).size();                            \
        if (MKVARNAME(sz) == MKVARNAME(sl) ||                                   \
            (MKVARNAME(sz) > MKVARNAME(sl) &&                                   \
             MKVARNAME(sz) == MKVARNAME(cn).capacity()))                        \
            WARNING("container exceeds " << MKVARNAME(sz) << " elements");      \
    } while (false)

namespace hjx {

// consider threading

inline bool g_stacktrace = true;

inline std::ostream &g_err(std::ostream *os=nullptr) {
    static std::ostream *err = &std::cerr;
    std::ostream *ret = err;
    if (os) err = os;
    return *ret;
}

inline std::ostream &g_err(std::ostream &os) { return g_err(&os); }

class auto_err {
    std::ostream &prev_err;
    bool prev_stacktrace;

public:
    auto_err(std::ostream &err, bool stacktrace):
        prev_err(g_err()),
        prev_stacktrace(g_stacktrace)
    {
        g_err(err);
        g_stacktrace = stacktrace;
    }

    ~auto_err() {
        g_stacktrace = prev_stacktrace;
        g_err(prev_err);
    }
};

struct hexos {
    // todo - testme
    mutable std::ostream *os_;
    mutable std::ios_base::fmtflags flags_;

    hexos(): os_{}, flags_{} {}
    ~hexos() { DASSERT(os_, "no ostream"); os_->flags(flags_); }

    friend std::ostream &operator<<(std::ostream &os, const hexos &h) {
        ASSERT(!h.os_);
        h.os_ = &os;
        h.flags_ = os.flags();
        return os << std::hex;
    }
};

}
