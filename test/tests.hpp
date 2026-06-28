// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "core/logging.hpp"
#include "core/cli.hpp"
#include "core/version.hpp"
#include "core/time.hpp"
#include "core/token.hpp"

#define TEST(x) TEST_MSG(x, #x)
#define TEST_MSG(x, msg) PROVE_MSG(x, msg, REPORT_THROW)

#define TEST_EQ(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_EQ, REPORT_THROW)
#define TEST_NE(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_NE, REPORT_THROW)
#define TEST_LT(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_LT, REPORT_THROW)
#define TEST_GT(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_GT, REPORT_THROW)
#define TEST_LE(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_LE, REPORT_THROW)
#define TEST_GE(x, y) PROVE_COMPARE(x, #x, y, #y, CMP_GE, REPORT_THROW)

#define TEST_GE_LE(v, low, high) PROVE_GE_LE(v, low, high, REPORT_THROW)
#define TEST_GE_LT(v, low, high) PROVE_GE_LT(v, low, high, REPORT_THROW)
#define TEST_GT_LE(v, low, high) PROVE_GT_LE(v, low, high, REPORT_THROW)
#define TEST_GT_LT(v, low, high) PROVE_GT_LT(v, low, high, REPORT_THROW)

#define TEST_IMPLY(ifcnd, thencnd) PROVE_IMPLY(ifcnd, thencnd, REPORT_THROW)
#define TEST_XOR(x, y) PROVE_XOR(x, REPORT_THROW)
#define TEST_NOT(x) PROVE_NOT(x, REPORT_THROW)
#define TEST_EQFISH(a, b) PROVE_EQFISH(a, b, REPORT_THROW)

#define TEST_REPR(a, b) TEST_EQ(TO_STRING(a), TO_STRING(b))

#if ASSERTS_ON
#define TEST_THROW(...)                         \
    do {                                        \
        bool fail = false;                      \
        try {                                   \
            std::ostringstream ss;              \
            auto_err MKVARNAME(_)(ss, false);   \
            __VA_ARGS__;                        \
        }                                       \
        catch (...) { fail = true; }            \
        TEST(fail);                             \
    } while (false)
#else
#define TEST_THROW(...)
#endif


#define DEFINE_TEST(name)                                                      \
    void test_##name();                                                        \
    int MKVARNAME(dummy) =                                                     \
        (test::harness::add(#name, static_cast<void(*)()>(test_##name)), 0);   \
    void test_##name()

#define FAIL_MSG "Boom"

namespace hjx::test {

// fixme - moving this to core.hpp fails
template <typename T>
std::string to_string(const T &x) {
    std::ostringstream ss;
    ss << x;
    return ss.str();
}

class harness {
    struct test {
        token name;
        std::function<void()> f;
    };

    static std::vector<test> &tests() NOTHROW {
        RETURN_STATIC_OBJECT(std::vector<test>);
    }

public:
    static inline bool verbose = false;
    static inline bool timing = false;

    static void exec(const char *suite, const char *argv[]) {
        cli(suite).eval(argv);
    }

    static uint64_t seed() { static uint64_t ret = new_seed(); return ret; }

    static void add(const char *name, const std::function<void()> &f) {
        tests().emplace_back(token(name), f);
    }

#define OUTPUT_(x) (std::cout << TO_STRING(x) << std::endl)
#define SUITE_ "suite:" << std::left << std::setw(6) << suite

    struct cli : hjx::cli {
        const char *suite;

        cli(const char *suite): suite(suite) {
            option("time", nullptr, "timing", [](const char*) {
                timing = true;
            });

            option("v", nullptr, "verbose", [](const char*) {
                verbose = true;
            });
        }

        void run(test &t) {
            if (verbose)
                OUTPUT_(suite << " test " << t.name.sym());

            time_point begin_test = clock::now();
            t.f();
            time_point end_test = clock::now();

            if (timing)
                OUTPUT_(suite << " test " << t.name.sym() <<
                        " finished in " << to_string(end_test - begin_test));
        }

        void args(const std::vector<const char*> &args) override {
            OUTPUT_(SUITE_ << " <starting> " << version::timestamp
                    << " ∵" << seed());

            time_point begin_suite = clock::now();

            for (test &t : tests())
                if (args.empty()) run(t);
                else for (const char *arg : args) if (t.name == arg) run(t);

            OUTPUT_(SUITE_ << " <complete> elapsed " <<
                    std::right << std::setw(6) <<
                    to_string(clock::now() - begin_suite));
        }
    };

#undef SUITE_
#undef OUTPUT_

};

struct journal {
    inline static bool verbose = false;
    int x;

    void log(const char *s) {
        if (verbose) std::cout << this << '\t' << s << x << std::endl;
    }

    static inline int c_;
    journal() noexcept: x(0) { c_++; log("c  "); }
    journal(int x) noexcept: x(x) { c_++; log("ci "); }

    static inline int d_;
    ~journal() { d_++; log("d  "); }

    static inline int cc_;
    journal(const journal &o) noexcept: x(o.x) { cc_++; log("cc "); }

    static inline int mc_;
    journal(journal &&o) noexcept: x(o.x) { mc_++; log("mc "); }

    static inline int ca_;
    journal &operator=(const journal &o) noexcept {
        ca_++;
        x = o.x;
        log("ca ");
        return *this;
    }

    static inline int ma_;
    journal &operator=(journal &&o) noexcept {
        ma_++;
        x = o.x;
        log("ma ");
        return *this;
    }

    static void reset() { c_ = 0; d_ = 0; cc_ = 0; mc_ = 0; ca_ = 0; ma_ = 0; }

    static void check(int c, int d, int cc, int mc, int ca, int ma) {
        TEST_MSG(c_ == c && d_ == d && cc_ == cc &&
                 mc_ == mc && ca_ == ca && ma_ == ma,
                 " Want c:" << c << " d:" << d << " cc:" << cc << " mc:" << mc
                 << " ca:" << ca << " ma:" << ma << "  Got c:" << c_ << " d:"
                 << d_ << " cc:" << cc_ << " mc:" << mc_ << " ca:" << ca_ <<
                 " ma:" << ma_);
    }

    static void report() {
        std::cout << "c:" << c_ << " d:" << d_
                  << " cc:" << cc_ << " mc:" << mc_
                  << " ca:" << ca_ << " ma:" << ma_ << std::endl;
    }

    auto operator==(const journal &o) const noexcept { return x == o.x; }

    friend ostream &operator<<(ostream &os, const journal &j) {
        return os << j.x;
    }
};

}
