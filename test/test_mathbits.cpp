// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/mathbits.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(amp_sum) {
    using math::amp_sum;
    using math::amp_sum_ex;

    TEST_EQ(amp_sum(0, 0), 0);

    TEST_EQFISH(amp_sum(0.01, 0.01), 0.025);
    TEST_EQFISH(amp_sum(0.1, 0.1), 0.25);
    TEST_EQFISH(amp_sum(1, 1), 2.5);
    TEST_EQFISH(amp_sum(10, 10), 25);
    TEST_EQFISH(amp_sum(100, 100), 250);

    TEST_EQFISH(amp_sum(1, 1, 1), 3.33333);
    TEST_EQFISH(amp_sum(1, 1, 9), 11.8182);
    TEST_EQFISH(amp_sum(1, 9, 9), 23.2632);
    TEST_EQ    (amp_sum(9, 9, 9), 54);
    TEST_EQFISH(amp_sum(0, 9, 9), 18);
    TEST_EQ    (amp_sum(0, 0, 9), 9);
    TEST_EQ    (amp_sum(0, 0, 0), 0);

    TEST_EQ    (amp_sum_ex(1, 1, 1), 3.75);
    TEST_EQFISH(amp_sum_ex(1, 1, 9), 12.15);
    TEST_EQFISH(amp_sum_ex(1, 9, 9), 22.15);
    TEST_EQ    (amp_sum_ex(9, 9, 9), 33.75);
    TEST_EQ    (amp_sum_ex(0, 9, 9), 20.25);
    TEST_EQ    (amp_sum_ex(0, 0, 9), 9);
    TEST_EQ    (amp_sum_ex(0, 0, 0), 0);

    TEST_EQ    (amp_sum(1, 2, 3, multiplier{0.0}), 6.0);
    TEST_EQ    (amp_sum(1, 2, 3, multiplier{0.5}), 6.5);
    TEST_EQ    (amp_sum(1, 2, 3, multiplier{1.0}), 7.0);
    TEST_EQ    (amp_sum(1, 2, 3, multiplier{1.5}), 7.5);
    TEST_EQ    (amp_sum(1, 2, 3, multiplier{2.0}), 8.0);

    TEST_EQ    (amp_sum_ex(1, 2, 3, multiplier{0.0}), 6.0);
    TEST_EQFISH(amp_sum_ex(1, 2, 3, multiplier{0.5}), 6.65417);
    TEST_EQFISH(amp_sum_ex(1, 2, 3, multiplier{1.0}), 7.30833);
    TEST_EQFISH(amp_sum_ex(1, 2, 3, multiplier{1.5}), 7.9625);
    TEST_EQFISH(amp_sum_ex(1, 2, 3, multiplier{2.0}), 8.61667);

    TEST_EQ    (amp_sum(   1, 1999, multiplier{0.0}), 2000);
    TEST_EQFISH(amp_sum(   1, 1999, multiplier{0.5}), 2000.5);
    TEST_EQFISH(amp_sum(   1, 1999, multiplier{1.0}), 2001.0);
    TEST_EQFISH(amp_sum(   1, 1999, multiplier{1.5}), 2001.5);
    TEST_EQFISH(amp_sum(   1, 1999, multiplier{2.0}), 2002.0);

    TEST_EQ    (amp_sum(1000, 1000, multiplier{0.0}), 2000);
    TEST_EQ    (amp_sum(1000, 1000, multiplier{0.5}), 2250);
    TEST_EQ    (amp_sum(1000, 1000, multiplier{1.0}), 2500);
    TEST_EQ    (amp_sum(1000, 1000, multiplier{1.5}), 2750);
    TEST_EQ    (amp_sum(1000, 1000, multiplier{2.0}), 3000);

    TEST_EQFISH(amp_sum(1, 1, 1), 3.33333);  // +1⋅sum/9
    TEST_EQFISH(amp_sum(2, 2, 2), 7.33333);  // +2⋅sum/9
    TEST_EQFISH(amp_sum(3, 3, 3), 12);       // +3⋅sum/9
    TEST_EQFISH(amp_sum(4, 4, 4), 17.3333);  // +4⋅sum/9
    TEST_EQFISH(amp_sum(5, 5, 5), 23.3333);  // +5⋅sum/9
    TEST_EQFISH(amp_sum(6, 6, 6), 30);       // +6⋅sum/9
    TEST_EQFISH(amp_sum(7, 7, 7), 37.3333);  // +7⋅sum/9
    TEST_EQFISH(amp_sum(8, 8, 8), 45.3333);  // +8⋅sum/9
    TEST_EQFISH(amp_sum(9, 9, 9), 54);       // +100%

    TEST_EQ    (amp_sum_ex(1, 1, 1), 3.75);  // +sum/4
    TEST_EQ    (amp_sum_ex(2, 2, 2), 7.5);   //    "
    TEST_EQ    (amp_sum_ex(3, 3, 3), 11.25); //    "
    TEST_EQ    (amp_sum_ex(4, 4, 4), 15);    //    "
    TEST_EQ    (amp_sum_ex(5, 5, 5), 18.75); //    "
    TEST_EQ    (amp_sum_ex(6, 6, 6), 22.5);  //    "
    TEST_EQ    (amp_sum_ex(7, 7, 7), 26.25); //    "
    TEST_EQ    (amp_sum_ex(8, 8, 8), 30);    //    "
    TEST_EQ    (amp_sum_ex(9, 9, 9), 33.75); //    "

    TEST_EQFISH(amp_sum(1.0, 1.5, 2.0), 5.16667);
    TEST_EQFISH(amp_sum(1.0, 1.5, multiplier{2.0}), 3.7);
}

DEFINE_TEST(bits) {
    TEST_EQ(leftmost_bit<char>, 0x80);
    TEST_EQ(leftmost_bit<int8_t>, 0x80);
    TEST_EQ(leftmost_bit<uint8_t>, 0x80);

    TEST_EQ(leftmost_bit<int16_t>, 0x8000);
    TEST_EQ(leftmost_bit<uint16_t>, 0x8000);

    TEST_EQ(leftmost_bit<int32_t>, 0x8000'0000);
    TEST_EQ(leftmost_bit<uint32_t>, 0x8000'0000);

    TEST_EQ(leftmost_bit<int64_t>, 0x8000'0000'0000'0000);
    TEST_EQ(leftmost_bit<uint64_t>, 0x8000'0000'0000'0000);

    TEST_EQ(leftmost_bit<float>, 0x8000'0000);
    TEST_EQ(leftmost_bit<double>, 0x8000'0000'0000'0000);
}

DEFINE_TEST(concat) {
    uint64_t u = concat('z');
    TEST_EQ(u, int('z'));

    char c{};
    unconcat(u, &c);
    TEST_EQ(int(c), int('z'));

    c = 'x';
    unconcat(u, 1, &c);
    TEST_EQ(int(c), 0);

    u = concat(int64_t(0x0123'4567'89ab'cdef));
    TEST_EQ(u, 0x0123'4567'89ab'cdef);

    int64_t i64{};
    unconcat(u, &i64);
    TEST_EQ(i64, 0x0123'4567'89ab'cdef);

    u = concat(int8_t(-2), uint32_t(0xaabbccdd), int16_t(0xeeff));
    TEST_EQ(u, 0xfe'aabb'ccdd'eeff);

    int8_t i8{};
    uint32_t u32{};
    int16_t i16{};
    unconcat(u, &i16, &u32, &i8);
    TEST_EQ(int(i8), -2);
    TEST_EQ(u32, 0xaabbccdd);
    TEST_EQ(i16, int16_t(0xeeff));

    u32 = 0;
    i8 = 0;
    unconcat(u, 2, &u32, &i8);
    TEST_EQ(u32, 0xaabbccdd);
    TEST_EQ(int(i8), int(int8_t(0xfe)));
}

template <typename T>
void test_abs(T in, T out) {
    T res = math::abs(in);
    TEST_EQ(res, out);
    TEST((std::is_same_v<decltype(res), T>));
}

#define TEST_AVERAGE(T)                                                         \
    TEST_MSG(math::average(T(i), T(j)) == gold(T(i), T(j)),                     \
             "average(" << int(T(i)) << ", " << int(T(j)) << ") == "            \
             << int(math::average(T(i), T(j)))                                  \
             << ", but gold(" << int(T(i)) << ", " << int(T(j)) << ") == "      \
             << int(gold(T(i), T(j))))

void test_average() {
    auto gold = [](auto a, auto b) {
        if (a < b) return a + (b - a) / 2;
        return b + (a - b) / 2;
    };

    for (int i = 0; i <= 255; i++)
        for (int j = 0; j <= 255; j++) {
            TEST_AVERAGE(uint8_t);
            TEST_AVERAGE(int8_t);
        }
}

float random_normal(auto &&rand) {
    using namespace math;

    for (;;) {
        uint32_t bits = rand() >> 32;
        if ((bits & exponent_mask_32) == exponent_mask_32) continue;
        if ((bits & exponent_mask_32) == 0 && (bits & mantissa_mask_32)) continue;
        return std::bit_cast<float>(bits);
    }
}

using math::interval;

void test_affine_map() {
#if 0
    float
        a = 4.3456065e+26,
        b = 4.345607e+26,
        c = 1.6263166e-21,
        d = -2.1960062e-12;
    math::affine_map map{{a, b}, {c, d}};
    DUMPV(map);
    DUMPV(frepr(a));
    DUMPV(frepr(b));
    DUMPV(honest_float(map(a)));
    DUMPV(honest_float(map(b)));
    TEST_MSG(map(a) == c, map);
    TEST_MSG(map(b) == d, map);
#else
    math::affine_map map{{0, 1}, {5, 10}};
    TEST_EQ(map(-0.5), 2.5);
    TEST_EQ(map(0), 5);
    TEST_EQ(map(0.5), 7.5);
    TEST_EQ(map(1), 10);
    TEST_EQ(map(1.5), 12.5);

    map = {{1, -2}, {5, 10}};
    TEST_EQ(map(1), 5);
    TEST_EQ(map(-0.5), 7.5);
    TEST_EQ(map(-2), 10);

    map = {{1, 2}, {98.6, 98.6}};
    TEST_EQ(map(0.5), 98.6f);
    TEST_EQ(map(1),   98.6f);
    TEST_EQ(map(1.5), 98.6f);
    TEST_EQ(map(2),   98.6f);
    TEST_EQ(map(2.5), 98.6f);

    map = {{0, 1e-10}, {-1e31, 0}};
    TEST_NOT(map.valid());

    rand_t rand(test::harness::seed());

    for (uint64_t i = 0; i < 9999; i++) {
        honest_float
            a = random_normal(rand),
            b = random_normal(rand),
            c = random_normal(rand),
            d = random_normal(rand);
        math::affine_map map{{a, b}, {c, d}};
        //DUMPI(a << " " << b << " " << c << " " << d);
        if (!map.valid()) continue;
        TEST_MSG(map(a) == c, map);
        TEST_MSG(map(b) == d, map);
    }
#endif
}


DEFINE_TEST(math) {
    using namespace math;

    test_abs(0ull, 0ull);
    test_abs(0ll, 0ll);
    test_abs(0ul, 0ul);
    test_abs(0u, 0u);
    test_abs(-6, 6);

    test_abs(uint64_t(-1), uint64_t(-1));
    test_abs(uint32_t(-1), uint32_t(-1));
    test_abs(uint16_t(-1), uint16_t(-1));
    test_abs(uint8_t(-1), uint8_t(-1));

    TEST_THROW(abs(int64min));
    TEST_THROW(abs(int32min));

    TEST_EQ(mod(3, 2), 1);
    TEST_EQ(mod(-3, 2), 1);

    TEST_EQ(scale(100,   0, 100),   0);
    TEST_EQ(scale(100,   1, 100),   1);
    TEST_EQ(scale(100,   1,   3),  33);
    TEST_EQ(scale(100,   1,   2),  50);
    TEST_EQ(scale(100,   2,   3),  66);
    TEST_EQ(scale(100,  99, 100),  99);
    TEST_EQ(scale(100, 100, 100), 100);

    TEST_EQ(scale(0xffff'ffff, 0xffff'ffff, 1), 0xffff'ffffull * 0xffff'ffff);
    TEST_EQ(scale(0xffff'ffff'ffff'ffff,
                  0xffff'ffff'ffff'ffff,
                  0xffff'ffff'ffff'ffff),
            0xffff'ffff'ffff'ffff);

    TEST_EQ(scale(3, 0xffff'ffff'ffff'ffff, 4), 0xbfff'ffff'ffff'ffff);
    TEST_EQ(scale(0xffff'ffff'ffff'ffff, 3, 4), 0xbfff'ffff'ffff'ffff);
    // expect to lose the lsb:
    TEST_EQ(scale(0xbfff'ffff'ffff'ffff, 4, 3), 0xffff'ffff'ffff'fffe);
    TEST_EQ(scale(4, 0xbfff'ffff'ffff'ffff, 3), 0xffff'ffff'ffff'fffe);

    TEST_EQ(memoryless(0), 0);
    TEST_EQ(memoryless(0.00000002), 0);
    TEST_GT(memoryless(0.00000003), 0); // exact threshold not important
    TEST_EQ(memoryless(0.5), logf(2));
    TEST(std::isfinite(memoryless(std::nexttoward(1.f, 0.f))));
    TEST(std::isinf(memoryless(1)));

    test_average();

    test_affine_map();

    TEST_EQ    (azsquash<25>(-999), 0);
    TEST_EQFISH(azsquash<25>(-0.5), 0.0000073);
    TEST_EQFISH(azsquash<25>( 0.0), 0.04);
    TEST_EQFISH(azsquash<25>( 0.5), 0.5);
    TEST_EQFISH(azsquash<25>( 1.0), 0.96);
    TEST_EQFISH(azsquash<25>( 1.5), 0.99999);

    TEST_EQ(bilinear_pow(-1, 0.01), 0);
    TEST_EQ(bilinear_pow( 0, 0.01), 1);
    TEST_EQ(bilinear_pow( 1, 0.01), inf);
    TEST_EQ(bilinear_pow(-1, 1), 0);
    TEST_EQ(bilinear_pow( 0, 1), 1);
    TEST_EQ(bilinear_pow( 1, 1), inf);
    TEST_EQ(bilinear_pow(-1, 100), 0);
    TEST_EQ(bilinear_pow( 0, 100), 1);
    TEST_EQ(bilinear_pow( 1, 100), inf);

    TEST_EQ(squircle(0,   -1), 0);
    TEST_EQ(squircle(0.5, -1), 0);
    TEST_EQ(squircle(1,   -1), 0);
    TEST_EQ(squircle(0,    0), 0);
    TEST_EQ(squircle(0.5,  0), 0.5);
    TEST_EQ(squircle(1,    0), 1);
    TEST_EQ(squircle(0,    1), 1);
    TEST_EQ(squircle(0.5,  1), 1);
    TEST_EQ(squircle(1,    1), 1);
}

DEFINE_TEST(phony_uint) {
    rand_t rand(test::harness::seed());

    for (int i = 0; i < 9999; i++) {
        float f = std::bit_cast<float>(uint32_t(rand()));
        auto u = to_uint(f);
        if (!std::isnan(f)) TEST_EQ(f, std::bit_cast<float>(u));
    }

    struct S { uint16_t x = 0x1234, y = 0x5678; };
    TEST_EQ(to_uint(S()), little_endian() ? 0x56781234 : 0x12345678);
}

void test_round_both(unsigned value,
                     unsigned multiple,
                     unsigned down,
                     unsigned up) {
    TEST_EQ(math::round_down(value, multiple), down);
    TEST_EQ(math::round_up(value, multiple), up);
}

DEFINE_TEST(rounding) {
    TEST_THROW(math::round_up(1u, 0u));
    TEST_THROW(math::round_down(1u, 0u));

    test_round_both(0, 1, 0, 0);
    test_round_both(1, 1, 1, 1);
    test_round_both(2, 1, 2, 2);

    test_round_both(0, 2, 0, 0);
    test_round_both(1, 2, 0, 2);
    test_round_both(2, 2, 2, 2);
    test_round_both(3, 2, 2, 4);

    test_round_both(0, 3, 0, 0);
    test_round_both(1, 3, 0, 3);
    test_round_both(2, 3, 0, 3);
    test_round_both(3, 3, 3, 3);
    test_round_both(4, 3, 3, 6);
}

void test_pow_log(int64_t e, double l) {
    using namespace math;

    TEST_EQ(zpow2(e), l);
    TEST_EQ(zlog2(l), e);
    TEST_EQ(azpow2(e), l);
    TEST_EQ(azlog2(l), e);
}

DEFINE_TEST(zpow) {
    using namespace math;

    auto clear_mantissa = [](double x) {
        return std::bit_cast<double>(std::bit_cast<uint64_t>(x) &
                                     ~mantissa_mask_64);
    };

    test_pow_log(1024, inf);
    test_pow_log(1023, clear_mantissa(max_normal_64));
    test_pow_log(0, 1);
    test_pow_log(-1022, min_normal_64);
    test_pow_log(-1023, 0);

    TEST_EQ(zlog2(max_normal_64), 1023);
    TEST_EQ(zlog2(max_denorm_64), -1023);

    rand_t rand(test::harness::seed());

    for (int i = 0; i < 9999; i++) { // fixme - just test the full range
        double x = doublestud(rand());

        int64_t n = x * 2047 - 1023; // [-1023, 1024)
        double goldpow = std::pow(2.0, n);
        double actpow = azpow2(n);
        TEST_GE(actpow, goldpow);

        double goldlog = std::log2(x * 1e100);
        double actlog = azlog2(x * 1e100);
        TEST_LE(actlog, goldlog);
    }

    TEST_EQ(azpow2( 3),   8);
    TEST_EQ(azpow2( 2.5), 6);
    TEST_EQ(azpow2( 2),   4);
    TEST_EQ(azpow2( 1.5), 3);
    TEST_EQ(azpow2( 1),   2);
    TEST_EQ(azpow2( 0.5), 1.5);
    TEST_EQ(azpow2( 0),   1);
    TEST_EQ(azpow2(-0.5), 0.75);

    TEST_GT(azpow2(std::nextafter(-1023.0, 0.0)), 0);
    TEST_EQ(azpow2(-1023), 0);
    // todo
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:mathbits", argv);
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
