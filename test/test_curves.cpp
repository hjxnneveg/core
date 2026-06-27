// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/curves.hpp"

#include "tests/tests.hpp"

#include "core/coords.hpp"

namespace hjx {

DEFINE_TEST(curves) {
    bezier2<qrs> b2(qrs{0, 0}, qrs{0, 1}, qrs{1, 1});
    TEST_REPR(b2(0.6), "[0.36,0.84,-1.2]");

    bezier3<qrs> b3(qrs{0, 0}, qrs{0, 1}, qrs{1, 0}, qrs{1, 1});
    TEST_REPR(b3(0.6), "[0.648,0.504,-1.152]");

    bezier3<xy> b3xy(xy{qrs{0, 0}}, xy{qrs{0, 1}}, xy{qrs{1, 0}}, xy{qrs{1, 1}});
    TEST_REPR(qrs{b3(0.6)}, "[0.648,0.504,-1.152]");
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:curves", argv);
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
