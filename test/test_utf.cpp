// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/utf.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(utf) {
    {
        utf::feed x("");
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 0);
        TEST_EQ(x.peek(), U'\0');
        TEST_EQ(x.next(), U'\0');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 0);
    }
    {
        utf::feed x("a\n");
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 0);
        TEST_EQ(x.next(), U'a');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 1);
        TEST_EQ(x.next(), U'\n');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 2);
        TEST_EQ(x.next(), U'\0');
    }
    {
        utf::feed x("\na");
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 0);
        TEST_EQ(x.next(), U'\n');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 1);
        TEST_EQ(x.next(), U'a');
        TEST_EQ(x.line(), 2);
        TEST_EQ(x.col(), 1);
        TEST_EQ(x.next(), U'\0');
    }
    {
        TEST_EQ(strlen("¿"), 2);
        TEST_EQ(strlen("₿"), 3);
        TEST_EQ(strlen("👽"), 4);

        utf::feed x("¿₿👽?");
        TEST_EQ(x.next(), U'¿');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 1);
        TEST_EQ(x.next(), U'₿');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 2);
        TEST_EQ(x.next(), U'👽');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 3);
        TEST_EQ(x.next(), U'?');
        TEST_EQ(x.line(), 1);
        TEST_EQ(x.col(), 4);
    }

    // todo
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:utf", argv);
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
