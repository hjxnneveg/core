// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include "core/proxy.hpp"

#include "core/test/tests.hpp"

namespace hjx {

// todo

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:proxy", argv);
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
