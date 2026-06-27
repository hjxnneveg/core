// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/ringbuffer.hpp"

#include "tests/tests.hpp"

namespace hjx {

void test_rb_ptrs(const auto &b, size_t shift) {
    size_t N = b.capacity;
    size_t elemsz = sizeof(b[0]);

    for (size_t i = 0; i < N; i++) {
        uintptr_t exp = uintptr_t(&b) + ((i + shift) % N * elemsz);
        uintptr_t act = uintptr_t(b.to_pointer(i));

        TEST_MSG(act == exp,
                 hexos() << "exp " << exp << " act " << act
                 << " in " << &b << "[" << i << "] / " << N << " >> " << shift);
    }

    TEST_EQ(uintptr_t(b.to_pointer(N)), uintptr_t(&b) + N * elemsz);
}

void test_ringbuffer_ptrs() {
    ringbuffer<int, 3> buffer;
    test_rb_ptrs(buffer, 0);

    buffer.push_back(1);
    test_rb_ptrs(buffer, 0);

    buffer.push_back(2);
    test_rb_ptrs(buffer, 0);

    buffer.pop_front();
    test_rb_ptrs(buffer, 1);

    buffer.pop_front();
    test_rb_ptrs(buffer, 2);
}

void test_rb_iter(const ringbuffer<int, 4> &buffer,
                  const  char *rep, int first, int len) {
    TEST_REPR(buffer, rep);
    int i = 0;
    for (int e : buffer) TEST_EQ(e, first + i++);
    TEST_EQ(i, len);
}

void test_ringbuffer_iter() {
    ringbuffer<int, 4> buffer;
    test_rb_iter(buffer, "[]", 0, 0);

    buffer.push_back(1);
    test_rb_iter(buffer, "[1]", 1, 1);

    buffer.push_back(2);
    test_rb_iter(buffer, "[1 2]", 1, 2);

    buffer.push_back(3);
    test_rb_iter(buffer, "[1 2 3]", 1, 3);

    buffer.pop_front();
    test_rb_iter(buffer, "[2 3]", 2, 2);

    buffer.push_back(4);
    test_rb_iter(buffer, "[2 3 4]", 2, 3);

    buffer.push_back(5);
    test_rb_iter(buffer, "[2 3 4 5]", 2, 4);

    TEST_THROW(buffer.push_back(6));

    buffer.pop_front();
    test_rb_iter(buffer, "[3 4 5]", 3, 3);

    buffer.push_back(6);
    test_rb_iter(buffer, "[3 4 5 6]", 3, 4);

    buffer.pop_front();
    test_rb_iter(buffer, "[4 5 6]", 4, 3);

    buffer.pop_front();
    test_rb_iter(buffer, "[5 6]", 5, 2);

    buffer.pop_front();
    test_rb_iter(buffer, "[6]", 6, 1);

    buffer.pop_front();
    test_rb_iter(buffer, "[]", 0, 0);

    TEST_THROW(buffer.pop_front());

    buffer.push_back(7);
    test_rb_iter(buffer, "[7]", 7, 1);
}

void test_ringbuffer_moves() {
    using test::journal;

    journal::reset();
    ringbuffer<journal, 4> buffer;
    journal::check(4, 0, 0, 0, 0, 0);
    TEST_EQ(buffer.size(), 0);
    TEST_THROW(buffer[0]);
    TEST_THROW(buffer[1]);
    TEST_THROW(buffer[2]);
    TEST_THROW(buffer[3]);
    TEST_THROW(buffer[4]);
    TEST_THROW(buffer.pop_front());

    buffer.push_back(journal(11));
    journal::check(5, 1, 0, 0, 0, 1);
    TEST_EQ(buffer.size(), 1);
    TEST_EQ(buffer[0].x, 11);
    TEST_THROW(buffer[1]);

    journal j22(22);
    buffer.push_back(j22);
    journal::check(6, 1, 0, 0, 1, 1);
    TEST_EQ(buffer.size(), 2);
    TEST_EQ(buffer[0].x, 11);
    TEST_EQ(buffer[1].x, 22);
    TEST_THROW(buffer[2]);

    buffer.push_back(journal(33));
    buffer.pop_front();
    journal::check(8, 3, 0, 0, 1, 3);
    TEST_EQ(buffer.size(), 2);
    TEST_EQ(buffer[0].x, 22);
    TEST_EQ(buffer[1].x, 33);
    TEST_THROW(buffer[2]);

    buffer.push_back(journal(44));
    buffer.push_back(journal(55));
    TEST_THROW(buffer.push_back(journal(66)));
}

DEFINE_TEST(ringbuffer) {
    test_ringbuffer_ptrs();
    test_ringbuffer_iter();
    test_ringbuffer_moves();
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:ringbuffer", argv);
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
