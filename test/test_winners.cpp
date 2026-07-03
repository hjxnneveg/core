// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include "core/winners.hpp"

#include "core/test/tests.hpp"

namespace hjx {

namespace {

struct elem_t {
    float score = 0;
    float pos = 0;
    int id = 0;
    explicit operator bool() const { return id; }
};

bool near1(const elem_t &a, const elem_t &b) {
    return std::abs(a.pos - b.pos) < 1;
}

int num_members(const auto &m) {
    int count = 0;
    for (const auto &e : m) if (e) count++;
    return count;
}

bool has(const auto &m, int id) {
    for (const auto &e : m) if (e.id == id) return true;
    return false;
}

// instrumented conflict functor: counts copies/moves
struct near1_journal {
    float radius = 1;
    std::vector<int> payload{1, 2, 3}; // move detector
    int *copies, *moves;

    near1_journal(int *c, int *m): copies(c), moves(m) {}

    near1_journal(const near1_journal &o): radius(o.radius),
                                           payload(o.payload),
                                           copies(o.copies),
                                           moves(o.moves) {
        ++*copies;
    }

    near1_journal(near1_journal &&o): radius(o.radius),
                                      payload(std::move(o.payload)),
                                      copies(o.copies),
                                      moves(o.moves) {
        ++*moves;
    }

    bool operator()(const elem_t &a, const elem_t &b) const {
        return std::abs(a.pos - b.pos) < radius;
    }
};

}

DEFINE_TEST(fill_and_evict) {
    auto m = make_winners<elem_t, 3>(near1);

    // fill holes, no conflicts
    m.appraise({5, 0, 1});
    m.appraise({1, 10, 2});
    m.appraise({9, 20, 3});
    TEST_EQ(num_members(m), 3);

    // full, no conflict, evict weakest
    m.appraise({2, 30, 4});
    TEST_EQ(has(m, 2), false);
    TEST_EQ(has(m, 4), true);
    TEST_EQ(has(m, 1), true);
    TEST_EQ(has(m, 3), true);

    // full, no conflict, weak candidate
    m.appraise({1.5, 40, 5});
    TEST_EQ(has(m, 5), false);
    TEST_EQ(num_members(m), 3);
}

DEFINE_TEST(evict_slot_zero) {
    auto m = make_winners<elem_t, 3>(near1);
    m.appraise({1, 0, 1});
    m.appraise({5, 10, 2});
    m.appraise({9, 20, 3});
    m.appraise({2, 30, 4});
    TEST_EQ(has(m, 1), false);
    TEST_EQ(has(m, 4), true);
}

DEFINE_TEST(single_conflict) {
    auto m = make_winners<elem_t, 3>(near1);
    m.appraise({5, 0, 1});

    // weaker incumbent enemy is evicted
    m.appraise({7, 0.5, 2});
    TEST_EQ(has(m, 1), false);
    TEST_EQ(has(m, 2), true);
    TEST_EQ(num_members(m), 1);

    // lose to stronger incumbent enemy
    m.appraise({6, 0.9, 3});
    TEST_EQ(has(m, 3), false);
    TEST_EQ(num_members(m), 1);

    // weakest gets a pass--enemy eviction is preferred
    m.appraise({2, 5, 4});
    m.appraise({8, 10, 5});
    m.appraise({9, 0.6, 6});
    TEST_EQ(has(m, 2), false);
    TEST_EQ(has(m, 6), true);
    TEST_EQ(has(m, 4), true);
}

DEFINE_TEST(double_conflict) {
    // best candidate rejected
    auto m = make_winners<elem_t, 2>(near1);
    m.appraise({3, 0, 1});
    m.appraise({4, 1.5, 2});
    m.appraise({10, 0.75, 3}); // SOL
    TEST_EQ(has(m, 3), false);
    TEST_EQ(has(m, 1), true);
    TEST_EQ(has(m, 2), true);
}

DEFINE_TEST(order_dependent) {
    // same candidates, different order, different winners
    elem_t x{3, 0, 1}, y{4, 1.5, 2}, z{10, 0.75, 3};

    auto a = make_winners<elem_t, 2>(near1);
    a.appraise(x); a.appraise(y); a.appraise(z);
    TEST_EQ(has(a, 3), false);
    TEST_EQ(num_members(a), 2);

    auto b = make_winners<elem_t, 2>(near1);
    b.appraise(z); b.appraise(x); b.appraise(y);
    TEST_EQ(has(b, 3), true);
    TEST_EQ(num_members(b), 1);
}

DEFINE_TEST(func_lvalue_copied) {
    int copies = 0, moves = 0;
    near1_journal pred(&copies, &moves);

    auto m = make_winners<elem_t, 3>(pred);

    // one copy (into ctor param), one move (param => member)
    TEST_EQ(copies, 1);
    TEST_EQ(moves, 1);

    // caller's func not gutted
    TEST_EQ(pred.payload.size(), 3);

    // mutate original, container unaffected
    pred.radius = 100;
    m.appraise({5, 0, 1});
    m.appraise({6, 2, 2});
    TEST_EQ(num_members(m), 2);
}

DEFINE_TEST(func_rvalue_moved) {
    int copies = 0, moves = 0;
    { make_winners<elem_t, 3>(near1_journal(&copies, &moves)); }

    TEST_EQ(copies, 0);
    TEST_EQ(moves, 2);
}

DEFINE_TEST(comtainer_copy) {
    int copies = 0, moves = 0;
    auto a = make_winners<elem_t, 2>(near1_journal(&copies, &moves));
    a.appraise({5, 0, 1});

    auto b = a;
    TEST_EQ(copies, 1);
    b.appraise({9, 10, 2});
    TEST_EQ(has(a, 2), false);
    TEST_EQ(has(b, 2), true);
    TEST_EQ(has(b, 1), true);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:winners", argv);
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
