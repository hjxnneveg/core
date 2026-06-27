// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/managed_cstr.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(managed_cstr) {
    managed_cstr s;
    TEST_EQ(s.ptr(), nullptr);
    TEST_NOT(bool(s));

    s = managed_cstr::dup("one");
    TEST_EQ(s.ptr(), string("one"));
    TEST(bool(s));

    s = managed_cstr::dup("");
    TEST_EQ(s.ptr(), string(""));
    TEST(bool(s));

    s = managed_cstr::dup(nullptr);
    TEST_EQ(s.ptr(), nullptr);
    TEST_NOT(bool(s));

    s = managed_cstr::ref("two");
    TEST_EQ(s.ptr(), string("two"));
    TEST(bool(s));

    s = managed_cstr::ref("");
    TEST_EQ(s.ptr(), string(""));
    TEST(bool(s));

    s = managed_cstr::ref(nullptr);
    TEST_EQ(s.ptr(), nullptr);
    TEST_NOT(bool(s));

    const char *text =
        "Dull sublunary lovers' love "
        "(Whose soul is sense) cannot admit "
        "Absence, because it doth remove "
        "Those things which elemented it.";
    const char *p = text + 17;

    s = managed_cstr::ref(p);
    TEST_EQ(s.ptr(), p);

    auto d = managed_cstr::dup(text);
    auto r = managed_cstr::ref(text);
    TEST_NE(d.ptr(), r.ptr());

    p = d.ptr();
    r = std::move(d);
    TEST_NOT(d.ptr());
    TEST_EQ(r.ptr(), p);

    r = managed_cstr::ref(text);
    p = r.ptr();
    d = std::move(r);
    TEST_NOT(r.ptr());
    TEST_EQ(d.ptr(), p);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:managed_cstr", argv);
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
