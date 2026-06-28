// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/eslist.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(eslist) {
    // todo
    eslist<float, 4> list{1, 5, 2};
    TEST_EQ(sizeof list, 4 * sizeof(float));
    TEST_EQ(list.size(), 3);
    TEST_REPR(list, "[1 2 5]");
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:eslist", argv);
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
