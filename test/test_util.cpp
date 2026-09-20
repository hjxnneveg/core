// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include <core/util.hpp>

#include <core/test/tests.hpp>

namespace hjx {

// fixme - get rid of util.hpp

DEFINE_TEST(apply_nonempty) {
    HJX_APPLY_NONEMPTY(abc, (), xyz);

    int acc = 0;
    auto f = [&](int n) { acc += n; };

    HJX_APPLY_NONEMPTY(f, ());
    TEST_EQ(acc, 0);

    HJX_APPLY_NONEMPTY(f, (6));
    TEST_EQ(acc, 6);

    (HJX_APPLY_NONEMPTY(f, (10)));
    TEST_EQ(acc, 16);

    HJX_APPLY_NONEMPTY(f, (100), ;) HJX_APPLY_NONEMPTY(f, (200));
    TEST_EQ(acc, 316);


    TEST_EQ(2, HJX_APPLY_NONEMPTY(([&](int, int n) { return n; }),
                                  (1, 2)));

    auto twice = [&](int n) { return 2 * n; };
    int arr[2] = {
        HJX_APPLY_NONEMPTY(twice, (1), ,)
        HJX_APPLY_NONEMPTY(twice, (), ,)
        HJX_APPLY_NONEMPTY(twice, (3))
    };
    TEST_EQ(arr[0], 2);
    TEST_EQ(arr[1], 6);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:util", argv);
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
