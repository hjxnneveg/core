// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/time.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(time) {
    // todo
    clock::time_point();
    clock::duration();
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:time", argv);
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
