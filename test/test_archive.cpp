// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/archive.hpp"

#include "core/test/tests.hpp"

namespace hjx {

struct point {
    float x;
    float y;

    bool operator==(const point&) const = default;

    static void serialize(archive &a, const point &p) { a.put(p.x); a.put(p.y); }

    static point deserialize(auto &x) { return {x.get_float(), x.get_float()}; }

    friend ostream &operator<<(ostream &os, point p) { return os << p.x << "," << p.y; }
};

DEFINE_TEST(heterogeny) {
    float f = 2.71828;
    char c = 'x';
    uint64_t n = 0x0123'4567'89ab'cdef;
    int16_t i = -6;
    point coords{11, 22};
    char raw[] = "raw";
    string s = "12345";

    archive a;
    auto x = a.cursor();
    TEST_EQ(x.remaining(), 0);

    a.put(f);                         TEST_EQ(x.remaining(),  4);
    a.put(c);                         TEST_EQ(x.remaining(),  5);
    a.put(n);                         TEST_EQ(x.remaining(), 13);
    TEST_EQ(x.get<float>(), f);       TEST_EQ(x.remaining(),  9);
    TEST_EQ(x.get<char>(), c);        TEST_EQ(x.remaining(),  8);
    a.put(i);                         TEST_EQ(x.remaining(), 10);
    a.put(coords);                    TEST_EQ(x.remaining(), 18);
    TEST_EQ(x.get<uint64_t>(), n);    TEST_EQ(x.remaining(), 10);
    TEST_EQ(x.get<int16_t>(), i);     TEST_EQ(x.remaining(),  8);
    a.put_raw(raw, sizeof raw);       TEST_EQ(x.remaining(), 12);
    a.put(s);                         TEST_EQ(x.remaining(), 18);
    TEST_EQ(x.get<point>(), coords);  TEST_EQ(x.remaining(), 10);
    char out[sizeof raw];
    x.get_raw(out, sizeof raw);
    TEST_MSG(!strcmp(out, raw), out); TEST_EQ(x.remaining(),  6);
    TEST_EQ(x.get_string(), s);       TEST_EQ(x.remaining(),  0);
}

DEFINE_TEST(repr) {
    archive a;
    TEST_REPR(a, "(0 bytes)");

    a << 0x0123'4567'89ab'cdef;
    TEST_REPR(a, "ef cd ab 89 67 45 23 01 (8 bytes)");

    a << 0xffff'ffff'ffff'ffff;
    a << 0xfedc'ba98'7654'3210;
    TEST_REPR(a,
              "ef cd ab 89 67 45 23 01 ff ff .. "
              "ff ff 10 32 54 76 98 ba dc fe (24 bytes)");

    a.clear();
    TEST_REPR(a, "(0 bytes)");
}

DEFINE_TEST(message) {
    archive msg(glyph::feldspar_msg(0));
    TEST_REPR(msg, "fe 1d 5d 42 00 00 00 00 (8 bytes)");

    msg << int16_t(6);
    msg.set_header(glyph::feldspar_msg(2));
    TEST_REPR(msg, "fe 1d 5d 42 00 00 00 02 06 00 (10 bytes)");

    msg = archive{};
    TEST_REPR(msg, "(0 bytes)");

    msg.set_header(glyph::feldspar_msg(3));
    TEST_REPR(msg, "fe 1d 5d 42 00 00 00 03 (8 bytes)");
}

template <typename T>
void test_archive_string(const T &v, const char *repr) {
    archive a;
    a << v;
    TEST_REPR(a, repr);
    string out;
    a.cursor() >> out;
    TEST_EQ(out, v);
}

template <typename T>
void test_archive_char(const T &v, size_t n, const char *repr) {
    archive a;
    a.put_string(v, n);
    TEST_REPR(a, repr);
    string out;
    a.cursor() >> out;
    TEST_EQ(out, string(v, n));
}

DEFINE_TEST(archive_strings) {
    test_archive_string(string(254, 'x'),
                        "fe 78 78 78 78 78 78 78 78 78 .. "
                        "78 78 78 78 78 78 78 78 78 78 (255 bytes)");

    test_archive_string(string(255, 'z'),
                        "ff ff 00 00 00 00 00 00 00 7a .. "
                        "7a 7a 7a 7a 7a 7a 7a 7a 7a 7a (264 bytes)");

    test_archive_string(string(""), "00 (1 bytes)");

    test_archive_string(string("␆"), "03 e2 90 86 (4 bytes)"); // UTF-8

    test_archive_char("", 0, "00 (1 bytes)");
    test_archive_char("abc", 3, "03 61 62 63 (4 bytes)");

    test_archive_char((char*)"", 0, "00 (1 bytes)");
    test_archive_char((char*)"abc", 3, "03 61 62 63 (4 bytes)");

    test_archive_char((const char *)"", 0, "00 (1 bytes)");
    test_archive_char((const char *)"xyz", 3, "03 78 79 7a (4 bytes)");

    char azerty[6] = {'a', 'z', 'e', 'r', 't', 'y'};
    char az0rty[6] = {'a', 'z', '\0', 'r', 't', 'y'};
    test_archive_char(azerty, 6, "06 61 7a 65 72 74 79 (7 bytes)");
    test_archive_char(az0rty, 6, "06 61 7a 00 72 74 79 (7 bytes)");

    const char cazerty[7] = {'c', 'a', 'z', 'e', 'r', 't', 'y'};
    const char caz0rty[7] = {'c', 'a', 'z', '\0', 'r', 't', 'y'};
    test_archive_char(cazerty, 7, "07 63 61 7a 65 72 74 79 (8 bytes)");
    test_archive_char(caz0rty, 7, "07 63 61 7a 00 72 74 79 (8 bytes)");
}

DEFINE_TEST(archive_move) {
    archive a;
    a << 6;
    TEST_REPR(a, "06 00 00 00 (4 bytes)");
    archive b = std::move(a);
    TEST_REPR(a, "(0 bytes)");
    TEST_REPR(b, "06 00 00 00 (4 bytes)");
    b << 7;
    TEST_REPR(a, "(0 bytes)");
    TEST_REPR(b, "06 00 00 00 07 00 00 00 (8 bytes)");
    a = std::move(b);
    TEST_REPR(a, "06 00 00 00 07 00 00 00 (8 bytes)");
    TEST_REPR(b, "(0 bytes)");
}

}

int main(int, const char *argv[]) {
    try {
        //set_main_thread(); fixme
        hjx::test::harness::exec("core:archive", argv);
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
