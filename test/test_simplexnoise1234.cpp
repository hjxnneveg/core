// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include <core/simplexnoise1234.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(simplex) {
    rand_t rand(test::harness::seed());
    unsigned char perm[512];
    stegu::gen_permutation(perm, rand);

    for (int i = 0; i < 256; i++) TEST_EQ(+perm[i], +perm[i + 256]);

    stegu::snoise1(perm, 1.f);
    stegu::snoise2(perm, -10.f, 100.f);
    stegu::snoise3(perm, -1000.f, 10000.f, -100'000.f);
    stegu::snoise4(perm, 1'000'000.f, -10'000'000.f, 100'000'000.f, -1'000'000'000.f);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:simplexnoise1234", argv);
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
