// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include <core/genarray.hpp>

#include <tests/tests.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace hjx {

DEFINE_TEST(id) {
    genid nil;

    TEST_NOT(nil);
    TEST_EQ(nil.index(), 0);
    TEST(nil == genid{});
    TEST_REPR(nil, "[genid nil]");
}

DEFINE_TEST(lists) {
    genarray<int> list1;
    TEST_NOT(list1.get(genid{}));

    genarray<int> list2;

    TEST_EQ(list1.live(), 0);
    genid a = list1.insert(6);
    TEST_EQ(list1.live(), 1);
    TEST(a);
    TEST_EQ(a.index(), 1);
    TEST_EQ(a.generation(), 1);
    TEST_REPR(a, "[genid 1 #1]");

    genid b = list2.emplace(6);
    TEST_REPR(b, "[genid 1 #1]");
    TEST_EQ(a, b); // so be careful

    a = list1.insert(6);
    TEST_REPR(a, "[genid 2 #1]");
    TEST_NE(a, b);
    TEST_THROW(list2.get(a));

    list2.erase(b);
    a = list2.insert(7);
    TEST_REPR(a, "[genid 1 #3]");
    TEST(list2.get(a));
    TEST_NOT(list2.get(b));
    TEST_THROW(list1.get(a));
    TEST_EQ(*list2.get(a), 7);
}

DEFINE_TEST(erase) {
    genarray<int> list;
    genid id = list.insert(1);

    TEST(list.erase(id));
    TEST_EQ(list.live(), 0);
    TEST_NOT(list.get(id)); // stale handle misses safely
    TEST_NOT(list.erase(id)); // double-erase is a no-op
    TEST_NOT(list.erase(genid{})); // as is erasing nil
}

DEFINE_TEST(fifo) {
    genarray<int> list;
    genid a = list.insert(1);
    genid b = list.insert(2);
    genid c = list.insert(3);
    list.erase(b);
    list.erase(c);
    list.erase(a);

    // reuse order matches erase order
    TEST_EQ(list.insert(4).index(), 2);
    TEST_EQ(list.insert(5).index(), 3);
    TEST_EQ(list.insert(6).index(), 1);
    TEST_EQ(list.insert(7).index(), 4);
}

struct probe {
    static inline int ctors = 0;
    static inline int dtors = 0;
    static int alive() { return ctors - dtors; }

    std::string name; // non-trivial

    probe(std::string name): name(std::move(name)) { ctors++; }
    probe(probe &&o) noexcept: name(std::move(o.name)) { ctors++; }

    ~probe() { dtors++; }
};

DEFINE_TEST(life_balance) {
    probe::ctors = probe::dtors = 0;

    {
        genarray<probe> list;
        genid a = list.emplace("a");
        list.emplace("b");
        genid c = list.emplace("c");
        list.erase(a);
        list.erase(c);
        list.emplace("d"); // reuse a's slot
        TEST_EQ(probe::alive(), list.live());
    }

    TEST_EQ(probe::alive(), 0);
}

DEFINE_TEST(growth) {
    probe::ctors = probe::dtors = 0;
    genarray<probe> list; // no reserve

    genid ids[100];
    for (int i = 0; i < 100; i++)
        ids[i] = list.emplace(std::string(1, char('A' + i % 26)));

    // holes so growth also moves free slots
    list.erase(ids[12]);
    list.erase(ids[17]);

    for (int i = 0; i < 900; i++) list.emplace("more");

    TEST_EQ(list.live(), 998);
    TEST_EQ(probe::alive(), 998);
    TEST_NOT(list.get(ids[12]));
    TEST_NOT(list.get(ids[17]));
    TEST_EQ(list.get(ids[99])->name, "V"); // survived the moves intact
}

DEFINE_TEST(iteration) {
    genarray<int> list;
    genid ids[5];
    for (int i = 0; i < 5; i++) ids[i] = list.insert(i);
    list.erase(ids[0]);
    list.erase(ids[2]);
    list.erase(ids[4]);

    int expected[] = {1, 3};
    int *it = expected;
    for (int v : list) TEST_EQ(v, *it++);
    TEST(it == expected + countof(expected));

    genarray<int> empty;
    TEST(empty.begin() == empty.end());

    int count = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        count++;
        TEST_EQ(it.id(), ids[*it]);
    }
    TEST_EQ(count, 2);

    for (int &v : list) v *= 2;
    TEST_EQ(*list.get(ids[1]), 2);
    TEST_EQ(*list.get(ids[3]), 6);
}

// longish test
DEFINE_TEST(retire) {
    genarray<int> list;

    for (uint32_t i = genid::GEN_MAX / 2; i; i--)
        TEST(list.erase(list.emplace(6)));

    TEST_EQ(list.live(), 0);
    TEST_EQ(list.retired(), 0);

    TEST(list.erase(list.emplace(6)));

    TEST_EQ(list.live(), 0);
    TEST_EQ(list.retired(), 1);
}

DEFINE_TEST(list_repr) {
    genarray<int> list;
    TEST_REPR(list, "[genarray live 0 free 0]");
    genid a = list.insert(1);
    list.insert(2);
    TEST_REPR(list, "[genarray live 2 free 0]");
    list.erase(a);
    TEST_REPR(list, "[genarray live 1 free 1]");
}

DEFINE_TEST(fuzz) {
    genarray<int> list;

    std::unordered_map<genid, int> live;
    std::vector<genid> dead;

    rand_t rand(test::harness::seed());

    for (int op = 0; op < 999; op++) {
        uint64_t r = rand();

        if (r % 3 && live.size()) {
            auto it = live.begin();
            std::advance(it, r / 8 % live.size());
            genid id = it->first;
            TEST_EQ(*list.get(id), it->second);

            if (r % 2) {
                TEST(list.erase(id));
                dead.push_back(id);
                live.erase(it);
            }
        }
        else {
            int v = r & 0xff;
            genid id = list.insert(v);
            live[id] = v;
        }

        // stale ids stay dead forever
        for (genid id : dead) TEST_NOT(list.get(id));
        TEST_EQ(list.live(), live.size());
    }
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:genarray", argv);
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
