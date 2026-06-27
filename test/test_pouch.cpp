// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/pouch.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(pouch) {
    using test::journal;

    pouch<int> s;
    TEST_NOT(s.contains(1));
    s.erase(1);
    TEST_NOT(s.contains(1));
    s.insert(1);
    TEST(s.contains(1));
    s.erase(1);
    TEST_NOT(s.contains(1));
    s.erase(1);
    TEST_NOT(s.contains(1));

    {
        journal::reset();
        pouch<journal> j;
        for (size_t i = 1; i < pouch<int>::soft_limit; i++) {
            j.insert(i); // todo - test insertion warning
            TEST(j.contains(1));
        }
    }
    TEST_EQ(journal::cc_, 0);
    TEST_EQ(journal::ca_, 0);

    {
        journal::reset();
        pouch<journal> j;
        journal::check(0, 0, 0, 0, 0, 0);
        j.insert(journal());
        journal::check(1, 1, 0, 1, 0, 0);
    }
    journal::check(1, 2, 0, 1, 0, 0);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:pouch", argv);
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
