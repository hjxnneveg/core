// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/delta_buffer.hpp"

#include "tests/tests.hpp"

namespace hjx {

#define CHECK_(s, b, c, n)                      \
    do {                                        \
        TEST_REPR(buffer, s);                   \
        TEST_EQ(base, float(b));                \
        TEST_EQ(current, float(c));             \
        TEST_EQ(count, n);                      \
    } while (false)

DEFINE_TEST(delta_buffer) {
    float base = 0, current = 0;
    int count = 0;

    delta_buffer<float, 2> buffer([&](float b, float c) {
        base = b;
        current = c;
        count++;
    });

    TEST_THROW(buffer.ack(42));

    CHECK_("[0/0 (0) ∅]", 0.0, 0.0, 0);

    TEST_EQ(buffer.update(1.1), 1);
    CHECK_("[0/1 (0) 1.1]", 0.0, 1.1, 1);

    buffer.ack(1);
    CHECK_("[1/1 (1.1) ∅]", 0.0, 1.1, 1);

    buffer.ack(1);
    CHECK_("[1/1 (1.1) ∅]", 0.0, 1.1, 1);

    TEST_EQ(buffer.update(2.2), 2);
    buffer.update(3.3);
    CHECK_("[1/3 (1.1) 2.2 3.3]", 1.1, 3.3, 3);

    buffer.ack(3);
    CHECK_("[3/3 (3.3) ∅]", 1.1, 3.3, 3);

    buffer.update(4.4);
    buffer.update(5.5);
    buffer.update(6.6);
    // consider - lost the client--do we want to throw away the base?
    CHECK_("[3/6 (3.3) 6.6]", 3.3, 6.6, 6);

    buffer.ack(5);
    CHECK_("[3/6 (3.3) 6.6]", 3.3, 6.6, 6);

    buffer.ack(6);
    CHECK_("[6/6 (6.6) ∅]", 3.3, 6.6, 6);

    TEST_THROW(buffer.ack(0));
}

#undef CHECK_

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:delta_buffer", argv);
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
