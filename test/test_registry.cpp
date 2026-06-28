// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/registry.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(registry) {
    registry<finale> r;
    bool one = false, two = false, three = false;
    finale sentinel_one   = r.add(finale([&]{ one   = true; }));
    finale sentinel_two   = r.add(finale([&]{ two   = true; }));
    finale sentinel_three = r.add(finale([&]{ three = true; }));

    TEST(!one);
    TEST(!two);
    TEST(!three);

    sentinel_two.detonate();
    TEST(!one);
    TEST(two);
    TEST(!three);

    sentinel_one.detonate();
    TEST(one);
    TEST(two);
    TEST(!three);

    sentinel_three.detonate();
    TEST(one);
    TEST(two);
    TEST(three);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:registry", argv);
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
