// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/glyph.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(glyph) {
    glyph_t g = glyph::feldspar_msg(0x1234'5678);
    TEST_REPR(g, "fe1d'5d42'1234'5678");

    glyph::material m = glyph::material(0x0000'fedc'ba98'7654);
    TEST(m);
    TEST_REPR(m, "44a7'fedc'ba98'7654");

    glyph::version v(1, 2, 3);
    TEST(v);
    TEST_REPR(v, "fe25'0001'0002'0003");
    TEST_EQ(v.major(), 1);
    TEST_EQ(v.minor(), 2);
    TEST_EQ(v.patch(), 3);

    TEST_EQ(g, g);
    TEST_EQ(v, v);
    TEST_NE(g, v);
    TEST_NE(m, v);

    glyph::color color(1, -1, 0, 254);
    TEST(color);
    TEST_REPR(color, "c012'0000'01ff'00fe");
    TEST_EQ(color.r(),   1);
    TEST_EQ(color.g(), 255);
    TEST_EQ(color.b(),   0);
    TEST_EQ(color.a(), 254);

    glyph::list l(0x1234'5678'9abc);
    TEST(l);
    TEST_REPR(l, "1157'1234'5678'9abc");
    TEST_EQ(l.length(), 0x1234'5678'9abc);

    glyph::object ob(0x42, 0x1234'5678);
    TEST(ob);
    TEST_REPR(ob, "0bc1'0042'1234'5678");
    TEST_EQ(ob.category(), 0x42);
    TEST_EQ(ob.discriminant(), 0x1234'5678);

    glyph::objinfo info(0xffff'ffee'ffff);
    TEST(info);
    TEST_REPR(info, "0b17'ffff'ffee'ffff");
    TEST_EQ(info.mask(), 0xffff'ffee'ffff);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:glyph", argv);
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
