// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include <core/hex/algo.hpp>

#include <core/test/tests.hpp>

#include <core/mathbits.hpp>
#include <core/entropy.hpp>

#include <concepts>
#include <numeric>
#include <ostream>

namespace hjx {

class hexed_int {
    uint64_t storage;

public:
    constexpr hexed_int(std::integral auto x): storage(to_uint(x)) {}

    constexpr operator uint64_t() const { return storage; }

    bool operator==(std::integral auto o) const { return storage == to_uint(o); }
    bool operator==(hexed_int o) const { return storage == o.storage; }

    friend std::ostream &operator<<(std::ostream &os, hexed_int x) {
        return os << hexos() << uint64_t(x);
    }
};

DEFINE_TEST(find_greatest) {
    rand_t rand(test::harness::seed());

    constexpr uint16_t width = 5;
    constexpr size_t count = hex::count(width);

    hex::grid<int> g{width};

    std::array<int, count> a;
    std::ranges::iota(a, 1);
    shuffle(a, rand);
    int *it = &a[0];

    g.foreach([&](int &hx) {
        TEST_EQ(hx, 0);
        hx = *it++;
    });

    auto gt = [&](qrs a, qrs b) { return g.to_hex(a) > g.to_hex(b); };
    std::array<qrs, 6> greatest = hex::find_greatest<6>(g.width(), gt);

    uint64_t results = 0;

    for (qrs great : greatest) {
        int n = g.to_hex(great);
        TEST_NOT(results & 1 << n);
        results |= 1 << n;
    }

    TEST_EQ(hexed_int(results), 0xfc000);
}

DEFINE_TEST(islands) {
    constexpr size_t width = 7;
    constexpr size_t count = hex::count(width);

    uint16_t data[count] = {
        /**/    1,0,0,2,    //      west
        /**/   0,0,0,2,0,   //
        /**/  3,0,0,2,0,0,  //
        /**/ 3,0,4,0,0,5,5, // north    south
        /**/  0,0,0,0,0,5,  //
        /**/   0,6,0,7,0,   //
        /**/    6,0,7,0     //      east
    };

    std::array<uint16_t, count> islands;
    for (size_t i = 0; i < count; i++)
        islands[i] = data[i] ? hex::island_flag : 0;

    hex::find_islands(width, [&](qrs pos) {
        return &islands[hex::to_scalar(pos.q(), pos.r(), width)];
    });

    for (size_t i = 0; i < count; i++)
        TEST_MSG(islands[i] == data[i],
                 "i " << i << ", " << islands[i] << " != " << data[i]);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:hex:algo", argv);
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
