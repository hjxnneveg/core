// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include <core/weefloat.hpp>

#include <core/test/tests.hpp>

#include <cmath>
#include <limits>

namespace hjx {

DEFINE_TEST(weefloat) {
    float min_denorm = std::numeric_limits<float>::denorm_min();
    float max_denorm = std::bit_cast<float>(0x007f'ffff); // C++ giving a C+ effort
    float min_norm   = std::numeric_limits<float>::min();
    float max_norm   = std::numeric_limits<float>::max();
    float inf        = std::numeric_limits<float>::infinity();
    float nan        = std::numeric_limits<float>::quiet_NaN();

    weefloat uninit; (void) uninit;

    TEST_EQ(weefloat{min_denorm}, 0.f);
    TEST_GT(weefloat{max_denorm}, 0.f);
    TEST_GT(weefloat{min_norm}, weefloat{max_denorm});
    TEST_GT(weefloat{max_norm}, weefloat{min_norm});
    TEST_THROW(weefloat{inf});
    TEST_THROW(weefloat{nan});
    TEST_THROW(weefloat{-1});

    TEST_EQ(weefloat{1.f}, weefloat{1.f});
    TEST_EQ(weefloat{0.f}, weefloat{0.f});
    TEST_NE(weefloat{0.f}, weefloat{1.f});

    TEST_EQ(weefloat{1.f + weefloat::epsilon()}, 1.f + weefloat::epsilon());
    TEST_EQ(weefloat{1.f + weefloat::epsilon()/2}, 1.f);
    TEST_EQ(weefloat{1.00390625}, 1.00390625f); // 1 + 1/256
    TEST_EQ(weefloat{1.003906}, 1.f);

    TEST_NOT(weefloat{0.f}.raw_());
    TEST_NOT(weefloat{-0.f}.raw_());

    TEST_REPR(weefloat(0), "0");
    TEST_REPR(weefloat(1.5), "1.5");
    TEST_REPR(weefloat(1.004), "1.0039062"); // round trip doesn't need the trailing 5

    TEST_LT(weefloat{1.f}, 1.5f);
    TEST_LT(1.f, weefloat{1.5f});

    for (float x : {0.f,
                    min_denorm,
                    max_denorm,
                    min_norm,
                    1.f,
                    1.004f,
                    1.5f,
                    255.9f,
                    max_norm}) {
        weefloat w{x};
        TEST_LE(w, x);
        TEST_EQ(float(w), w);
        TEST_EQ(weefloat{float(w)}, w);
    }

    for (float x : {0.f, 0.5f, 1.f, 2.f, 1.5f, 1.25f, 256.f})
        TEST_EQ(float(weefloat{x}), x);

    TEST(std::isfinite(float(weefloat{max_norm})));
    TEST_LT(weefloat{max_norm}.raw_(), 0xff00); // never reaches inf encoding

    TEST_THROW(weefloat{1.f} == nan);
    TEST_THROW(weefloat{1.f} == inf);
    TEST_THROW(weefloat{1.f} < -1.f);
    TEST_THROW(nan == weefloat{1.f});
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:weefloat", argv);
        return 0;
    }
    catch (const char *s) {
        if (strcmp(s, FAIL_MSG))
            std::cout << "unexpected exception " << s << std::endl;
    }
    catch (...) {
        std::cout << "unknown exception" << std::endl;
    }

    return 1;
}
