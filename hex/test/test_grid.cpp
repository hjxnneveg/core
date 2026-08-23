// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include <core/hex/grid.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(hexgrid) {
    hex::grid<int> g{5};
    TEST_REPR(g, "[hexgrid int 5]");
    TEST_EQ(g.count(), 19);

    g.foreach([&](qrs pos, int &hex) {
        TEST_MSG(&g.to_hex(pos) == &hex, pos << " " << hex);
        TEST_MSG(g.to_qrs(hex) == pos, pos << " " << hex);
    });

    g.foreach([&](qrs pos, int &hex) { hex = g.to_scalar(pos) + 100; });

    int i = 100;
    g.foreach([&](int &hex) { TEST_EQ(hex, i++); });

    i = 100;
    for (int &hex : g) TEST_EQ(hex, i++);

    g.wipe();
    g.foreach([&](int &hex) { TEST_EQ(hex, 0); });

    hex::grid<int> big{555};
    TEST_REPR(big, "[hexgrid int 555]");
    TEST_EQ(big.count(), 231019);

    TEST_THROW(hex::grid<int>{6});
}

DEFINE_TEST(bounds) {
    hex::grid<int> g{5};

    for (float q = -3; q <= 3; q++)
        for (float r = -3; r <= 3; r++) {
            qrs pos{q, r};

            if (pos.manhattan() <= 2) {
                TEST_MSG(g.in_bounds(pos), pos);

                if (pos.manhattan() == 2)
                    TEST_MSG(g.border(pos), pos);
                else
                    TEST_MSG(!g.border(pos), pos);
            }
            else {
                TEST_MSG(!g.in_bounds(pos), pos);
                TEST_THROW(g.to_scalar(pos));
            }
        }
}

#define NEIGHBOR_TESTS                                  \
    do {                                                \
        check(3, C,     {NW, SW, N, S, NE, SE});        \
        check(3, N,     {SW, S, SE});                   \
        check(3, NE,    {NW, SW, S});                   \
        check(3, SE,    {NW, SW, N});                   \
        check(3, S,     {NW, N, NE});                   \
        check(3, SW,    {N, NE, SE});                   \
        check(3, NW,    {S, NE, SE});                   \
                                                        \
        check(5, C,     {NW, SW, N, S, NE, SE});        \
        check(5, N,     {NW, SW, N, S, NE, SE});        \
        check(5, NE,    {NW, SW, N, S, NE, SE});        \
        check(5, SE,    {NW, SW, N, S, NE, SE});        \
        check(5, S,     {NW, SW, N, S, NE, SE});        \
        check(5, SW,    {NW, SW, N, S, NE, SE});        \
        check(5, NW,    {NW, SW, N, S, NE, SE});        \
                                                        \
        check(5, N+N,   {SW, S, SE});                   \
        check(5, N+NE,  {NW, SW, S, SE});               \
        check(5, NE+NE, {NW, SW, S});                   \
        check(5, NE+SE, {NW, SW, N, S});                \
        check(5, SE+SE, {NW, SW, N});                   \
        check(5, SE+S,  {NW, SW, N, NE});               \
        check(5, S+S,   {NW, N, NE});                   \
        check(5, S+SW,  {NW, N, NE, SE});               \
        check(5, SW+SW, {N, NE, SE});                   \
        check(5, SW+NW, {N, S, NE, SE});                \
        check(5, NW+NW, {S, NE, SE});                   \
        check(5, NW+N,  {SW, S, NE, SE});               \
    } while (false)

DEFINE_TEST(foreach_neighbor) {
    qrs C  = {0, 0};
    qrs N  = qrs::north_unit();
    qrs NE = qrs::ne_unit();
    qrs SE = qrs::se_unit();
    qrs S  = qrs::south_unit();
    qrs SW = qrs::sw_unit();
    qrs NW = qrs::nw_unit();

    auto check = [](int width, qrs offset, std::initializer_list<qrs> seq) {
        size_t i = 0;

        hex::foreach_neighbor(width, offset, [&](qrs neighbor) {
            TEST_MSG(i < seq.size(), "extra neighbor " << neighbor << " at i == " << i);

            if (i < seq.size())
                TEST_MSG(neighbor == offset + seq.begin()[i],
                         neighbor << " != " << seq.begin()[i] << ", i == " << i);

            i++;
        });

        TEST_MSG(i == seq.size(), "index " << i << " vs " << seq.size());
    };

    NEIGHBOR_TESTS;
}

DEFINE_TEST(neighbors) {
    qrs C  = {0, 0};
    qrs N  = qrs::north_unit();
    qrs NE = qrs::ne_unit();
    qrs SE = qrs::se_unit();
    qrs S  = qrs::south_unit();
    qrs SW = qrs::sw_unit();
    qrs NW = qrs::nw_unit();

    auto check = [](int width, qrs offset, std::initializer_list<qrs> seq) {
        auto r = hex::neighbors(width, offset);

        auto pass = [&](const char *label) {
            size_t i = 0;

            for (qrs neighbor : r) {
                TEST_MSG(i < seq.size(), label << " pass extra neighbor " << neighbor);
                TEST_MSG(neighbor == offset + seq.begin()[i],
                         label << " pass " << neighbor << " != " << seq.begin()[i]
                         << ", i == " << i);
                i++;
            }

            TEST_MSG(i == seq.size(), label << ": index " << i << " vs " << seq.size());
        };

        pass("first");
        pass("second");

        TEST_EQ(std::ranges::distance(hex::neighbors(width, offset)), seq.size());
    };

    NEIGHBOR_TESTS;
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:hex:grid", argv);
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
