// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include <core/entropy.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(hash) {
    auto hashc = [](const char *s) { return hash(s, strlen(s)); };

    // change detector
    TEST_EQ(hashc("X."),               0x66f9'2898'fbad'1e02);
    TEST_EQ(hashc(".X"),               0x1304'c864'f094'c4e5);
    TEST_EQ(hashc("XX.."),             0x8da6'e982'5492'eae2);
    TEST_EQ(hashc("..XX"),             0x710f'8657'0e29'3b5d);
    TEST_EQ(hashc("XXXX...."),         0x7b47'ca4b'e253'cf18);
    TEST_EQ(hashc("....XXXX"),         0x82ad'90aa'a166'9355);
    TEST_EQ(hashc("XXXXXXXX........"), 0x650f'2206'0523'c744);
    TEST_EQ(hashc("........XXXXXXXX"), 0x1321'c66c'8ea8'b5f4);

    int16_t a16 = -45, b16 = 23;
    int64_t a64 = -45, b64 = 23;
    uint64_t h16 = hash(a16, b16);
    uint64_t h64 = hash(a64, b64);

    TEST_EQ(hash(a16), hash(a64));
    TEST_EQ(hash(b16), hash(b64));

    // yellow flags
    TEST_NE(h16, hash(a16));
    TEST_NE(h16, hash(b16));
    TEST_NE(h64, hash(a16));
    TEST_NE(h64, hash(b16));

    TEST_NE(h16, hash(b16, a16));
    TEST_NE(h16, hash(a64, b64));
    TEST_NE(h16, hash(b64, a64));
    TEST_NE(hash(a64, b64), hash(b64, a64));
}

DEFINE_TEST(strhash) {
    std::unordered_map<uint64_t, size_t> hashes;
    constexpr size_t N = 6000;
    char zeros[N] = {};

    for (size_t i = 0; i < N; i++) {
        uint64_t h = hash(zeros, i);
        auto [_, inserted] = hashes.insert_or_assign(h, i);
        // Not necessary for these to be distinct, but investigate collisions.
        TEST_MSG(inserted, "collision @ length " << i << " : " << h);
    }
}

DEFINE_TEST(randint) {
    rand_t rand(test::harness::seed());

    int acc[10] = {};
    for (int i = 0; i < 10000; i++) {
        int n = randint(rand, 1, 10);
        TEST_GE_LE(n, 1, 10);
        acc[n - 1]++;
    }
    for (int n : acc) TEST_GE_LE(n, 800, 1200);

    int nacc[10] = {};
    for (int i = 0; i < 10000; i++) {
        int n = randint(rand, -5, 4);
        TEST_GE_LE(n, -5, 4);
        nacc[n + 5]++;
    }
    for (int n : nacc) TEST_GE_LE(n, 800, 1200);
}

DEFINE_TEST(scale_seed) {
    rand_t rand(test::harness::seed());
    short res[1729] = {};
    unsigned N = randint(rand, countof(res) / 2, countof(res));

    for (uint64_t i = 0; i <= 0xffff'ffff; i += 0xffff) {
        uint32_t scaled = scale_seed_32(i << 32, N);
        TEST_LT(scaled, N);
        res[scaled]++;
    }

    // uniformity
    int a = 0, b = 0;
    for (unsigned i = 0; i < N; i++) {
        TEST_MSG(res[i] > 0, i << " => " << res[i]);
        if (a == 0)
            a = res[i];
        else if (res[i] != a && b == 0)
            b = res[i];
        else
            TEST_MSG(res[i] == a || res[i] == b,
                     res[i] << " vs " << a << ", " << b);
    }
}

template <typename Container>
string dumpc(const Container &c) {
    std::ostringstream ss;
    const char *sep = "";
    for (const auto &v : c) { ss << sep << v; sep = ","; }
    return ss.str();
}

DEFINE_TEST(shuffle) {
    rand_t rng;
    vector<int> v = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };
    shuffle(v, rng);
    // don't care what it is, just want to know if anything changes
    vector<int> gold = { 8,12,10,20,13,7,4,3,14,19,5,6,18,16,15,9,17,11,2,1 };
    TEST_MSG(v == gold, dumpc(v));
}

DEFINE_TEST(splitmix) {
    using entropy::splitmix;

    uint64_t s = test::harness::seed();
    for (uint64_t x = 0; x < 9999; x++) {
        TEST_EQ(s + x, splitmix::unhash(splitmix::hash(s + x)));
        TEST_EQ(s + x, splitmix::hash(splitmix::unhash(s + x)));
    }
}

void test_stud_OT() {
    TEST_EQ(stud(0), 0);
    TEST_LT(stud(0xffff'ffff'ffff'ffff), 1);
    TEST_GT(1 - stud(0xffff'ffff'ffff'ffff), 0);
    TEST_EQ(stud(0x8000'0000'0000'0000), 0.5);

    auto test = [](uint64_t u) {
        float f = stud(u);
        TEST_GE_LT(f, 0, 1);
        TEST_MSG(0 <= f && f < 1, u << " => " << frepr(f));
        TEST_MSG(f == 0 || std::isnormal(f), u << " => " << frepr(f));
        if (u) TEST_MSG(f >= stud(u - 1), u << " => " << frepr(f));
    };

    test(0x0000'0000'0000'0000);
    test(0x0000'0000'0000'0001);
    test(0x7fff'ffff'ffff'ffff);
    test(0x8000'0000'0000'0000);
    test(0x8000'0000'0000'0001);
    test(0xffff'ffff'ffff'fffe);
    test(0xffff'ffff'ffff'ffff);

    test(1ull << 40);
    TEST_EQ(stud(1ull << 40), 0);
    test(1ull << 41);
    TEST_NE(stud(1ull << 41), 0);
    test(1ull << 42);

    rand_t rng(test::harness::seed());
    for (int i = 0; i < 9999; i++) test(rng());
}

// fixme - long test
// consistent uniformity across stud and stud_oc
void test_stud_NT(const auto &f, bool reverse=false) {
    for (uint64_t i = 0;; i += STUD_ULP) {
        TEST_EQ(f(i), f(i + (STUD_ULP - 1)));

        if (i > 0) {
            if (reverse) TEST_LT(f(i), f(i - 1));
            else         TEST_GT(f(i), f(i - 1));
        }

        if (i == STUD_LOWMAX) break;
    }
}

void test_stud_bounds(auto &&f)  {
    TEST_EQ(f(0, false), 1);
    TEST_EQ(std::nexttoward(f(0, true), 0.f), 1);
    TEST_EQ(std::nexttoward(f(0xffff'ffff'ffff'ffff, false), 3.f), 2);
    TEST_EQ(f(0xffff'ffff'ffff'ffff, true), 2);
}

void test_club(auto &&rand) {
    bool needlow = true, needhigh = true;

#if 1
    needlow = false;
    needhigh = false;
#endif

    for (int i = 0; i < 9999 || needlow || needhigh; i++) {
        float result = club_gap(rand());
        TEST_NE(result, 0);
        TEST_GE_LE(result, -1, 1);
        if (result == -1) needlow = false;
        if (result == 1) needhigh = false;
    }
}

DEFINE_TEST(stud) {
    test_stud_bounds(uniform12_);
    test_stud_bounds(uniform12d_);

    // todo - doublestud

    test_stud_OT();

    test_stud_NT([](uint64_t v) { return stud(v); });
    test_stud_NT([](uint64_t v) { return stud_oc(v); });
    test_stud_NT([](uint64_t v) { return 1 - stud_oc(v); }, true);
    test_stud_NT([](uint64_t v) { return 1 - stud(v); }, true);

    for (uint64_t i = 0;; i += STUD_ULP) {
        TEST_EQ(stud(i + STUD_ULP), stud_oc(i));
        if (i == STUD_LOWMAX - STUD_ULP) break;
    }

    rand_t rand(test::harness::seed());

    for (int i = 0; i < 9999; i++) {
        uint64_t u = 0xffff'ffff'ffff'ffff;
        while (u > 0xffff'ffff'ffff'ffff - STUD_ULP) u = rand();
        TEST_EQ(stud(u + STUD_ULP), stud_oc(u));
    }

    test_club(rand);
}

struct legit_rng {
    int invoked = 0;
    legit_rng(uint64_t) {}
    uint64_t operator()() { invoked++; return 9; }
};

DEFINE_TEST(permutation) {
    {
        permutation<0> perm(6);
        for (unsigned i : perm) { (void)i; TEST(false); }
        TEST_THROW(perm());
    }

    {
        permutation<4, legit_rng> perm(6);

        TEST_EQ(perm.rng_().invoked, 0);
        TEST_EQ(perm(), 0);
        TEST_EQ(perm.rng_().invoked, 1);
        TEST_EQ(perm(), 1);
        TEST_EQ(perm.rng_().invoked, 2);
        TEST_EQ(perm(), 2);
        TEST_EQ(perm.rng_().invoked, 3);
        TEST_EQ(perm(), 3);
        TEST_EQ(perm.rng_().invoked, 4);
        TEST_THROW(perm());
    }

    {
        permutation<3, legit_rng> perm(6);

        for (unsigned i : perm) {
            TEST_EQ(i, 0);
            TEST_EQ(perm.rng_().invoked, 1);
            break;
        }

        TEST_EQ(perm.rng_().invoked, 1);
    }

    {
        permutation<3, legit_rng> perm(6);
        TEST_EQ(perm(), 0);
        unsigned t = 1;
        for (unsigned i : perm) TEST_EQ(i, t++);
        TEST_THROW(perm());
    }
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:entropy", argv);
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
