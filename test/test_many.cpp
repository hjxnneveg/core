// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/many.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(many) {
    // todo - unfinished and ill-advised
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:many", argv);
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
