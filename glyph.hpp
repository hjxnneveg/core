// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

// Self-describing 64-bit values

#include "mathbits.hpp"
#include "entropy.hpp"

#include <bit>
#include <concepts>
#include <iomanip>
#include <ostream>
#include <type_traits>

#define GLYPH_COMMON(name, sntl)                                        \
    constexpr static uint64_t sentinel = to_uint(sntl);                 \
                                                                        \
    static bool isa(uint64_t v) {                                       \
        return (v & sentinel_mask(sentinel)) == sentinel;               \
    }                                                                   \
                                                                        \
    name(): glyph_t(0) {}                                               \
                                                                        \
    name(glyph_t g): glyph_t(g.v) {}                                    \
                                                                        \
    name(const name &o): glyph_t(o.v) {}                                \
    name(name &&o): glyph_t(o.v) {}                                     \
                                                                        \
    name &operator=(const name &o) { v = o.v; return *this; }           \
    name &operator=(name &&o) { v = o.v; return *this; }                \
                                                                        \
    explicit operator bool() const { return isa(v); }

#define GLYPH1_(name, sntl, type, param)                                \
    struct name : public glyph_t {                                      \
        GLYPH_COMMON(name, sntl)                                        \
                                                                        \
        explicit name(type param): glyph_t(sentinel | param) {          \
            ASSERT_NOT(sentinel_mask(sentinel) & to_uint(param));       \
        }                                                               \
                                                                        \
        type param() const { return v & ~sentinel_mask(sentinel); }     \
    }

#define GLYPH2_(name, sntl, type1, param1, type2, param2)                      \
    struct name : public glyph_t {                                             \
        GLYPH_COMMON(name, sntl)                                               \
                                                                               \
        name(type1 param1, type2 param2):                                      \
        glyph_t(sentinel | concat(param1, param2))                             \
        {                                                                      \
            ASSERT_NOT(sentinel_mask(sentinel) & concat(param1, param2));      \
        }                                                                      \
                                                                               \
        type1 param1() const {                                                 \
            return (v >> 8 * sizeof(type2)) & mask<type1>();                   \
        }                                                                      \
                                                                               \
        type2 param2() const { return v & mask<type2>(); }                     \
    }

#define GLYPH3_(name, sntl, type1, param1, type2, param2, type3, param3)       \
    struct name : public glyph_t {                                             \
        GLYPH_COMMON(name, sntl)                                               \
                                                                               \
        name(type1 param1, type2 param2, type3 param3):                        \
            glyph_t(sentinel | concat(param1, param2, param3))                 \
        {                                                                      \
            ASSERT_NOT(sentinel_mask(sentinel) &                               \
                       concat(param1, param2, param3));                        \
        }                                                                      \
                                                                               \
        type1 param1() const {                                                 \
            uint64_t shift = 8 * (sizeof(type2) + sizeof(type3));              \
            return v >> shift & mask<type1>();                                 \
        }                                                                      \
        type2 param2() const {                                                 \
            uint64_t shift = 8 * (sizeof(type3));                              \
            return v >> shift & mask<type2>();                                 \
        }                                                                      \
        type3 param3() const {                                                 \
            return v & mask<type3>();                                          \
        }                                                                      \
    }

#define GLYPH4(name, sntl,                                                     \
                type1, param1, type2, param2, type3, param3, type4, param4)    \
    static_assert(hjx::glyph_t::agreeable(to_uint(sntl),                       \
                                          sizeof(type1) +                      \
                                          sizeof(type2) +                      \
                                          sizeof(type3) +                      \
                                          sizeof(type4)));                     \
    struct name : public glyph_t {                                             \
        GLYPH_COMMON(name, sntl)                                               \
                                                                               \
        name(type1 param1, type2 param2, type3 param3, type4 param4):          \
            glyph_t(sentinel | concat(param1, param2, param3, param4))         \
        {                                                                      \
            ASSERT_NOT(sentinel_mask(sentinel) &                               \
                       concat(param1, param2, param3, param4));                \
        }                                                                      \
                                                                               \
        type1 param1() const {                                                 \
            uint64_t shift =                                                   \
                8 * (sizeof(type2) + sizeof(type3) + sizeof(type4));           \
            return v >> shift & mask<type1>();                                 \
        }                                                                      \
        type2 param2() const {                                                 \
            uint64_t shift = 8 * (sizeof(type3) + sizeof(type4));              \
            return v >> shift & mask<type2>();                                 \
        }                                                                      \
        type3 param3() const {                                                 \
            uint64_t shift = 8 * sizeof(type4);                                \
            return v >> shift & mask<type3>();                                 \
        }                                                                      \
        type4 param4() const {                                                 \
            return v & mask<type4>();                                          \
        }                                                                      \
    }

#define GLYPH1(name, sntl, type, param)                                 \
    static_assert(hjx::glyph_t::agreeable<type>(to_uint(sntl)));        \
    GLYPH1_(name, sntl, type, param)

#define GLYPH2(name, sntl, type1, param1, type2, param2)                \
    static_assert(hjx::glyph_t::agreeable(to_uint(sntl),                \
                                          sizeof(type1) +               \
                                          sizeof(type2)));              \
    GLYPH2_(name, sntl, type1, param1, type2, param2)

#define GLYPH3(name, sntl, type1, param1, type2, param2, type3, param3) \
    static_assert(hjx::glyph_t::agreeable(to_uint(sntl),                \
                                          sizeof(type1) +               \
                                          sizeof(type2) +               \
                                          sizeof(type3)));              \
    GLYPH3_(name, sntl, type1, param1, type2, param2, type3, param3)

#define GLYPH48(name, sntl, param)                                      \
    static_assert(!(hjx::glyph_t::sentinel_mask(sntl) &                 \
                    0x0000'ffff'ffff'ffff));                            \
    GLYPH1_(name, sntl, uint64_t, param)

#define GLYPH848(name, sntl, param1, param2)                            \
    static_assert(!(hjx::glyph_t::sentinel_mask(sntl) &                 \
                    0x00ff'ffff'ffff'ffff));                            \
                                                                        \
    struct name : public glyph_t {                                      \
        constexpr static uint64_t sentinel = to_uint(sntl);             \
                                                                        \
        static bool isa(uint64_t v) {                                   \
            return (v & sentinel_mask(sentinel)) == sentinel;           \
        }                                                               \
                                                                        \
        name(uint8_t param1, uint64_t param2):                          \
            glyph_t(sentinel | (uint64_t(param1) << 48) | param2)       \
        {                                                               \
            ASSERT_LE(param2, 0xffff'ffff'ffffu);                       \
        }                                                               \
                                                                        \
        name(glyph_t g): glyph_t(g.v) {}                                \
                                                                        \
        uint8_t param1() const { return (v >> 48) & 0xff; }             \
        uint64_t param2() const { return v & 0xffff'ffff'ffff; }        \
                                                                        \
        explicit operator bool() const { return isa(v); }               \
    }


namespace hjx {

struct glyph_t {
    glyph_t(): v(0) {}

    explicit glyph_t(uint64_t v): v(v) {} // todo - check vs registered

    uint64_t v;

    // fixme - having both serialize and repr is clumsy
    uint64_t repr() const { return big_endianize(v); }

    //bool matches(glyph_t o) const { todo; }

    bool operator==(glyph_t o) const { return v == o.v; }

    template <std::integral T>
    constexpr static uint64_t mask() {
        return uint64_t(-1) >> (64 - 8 * sizeof(T));
    }

    constexpr static uint64_t sentinel_mask(uint64_t sentinel) {
        ASSERT(sentinel);
        return uint64_t(-1) << (std::countr_zero(sentinel) & ~7);
    }

    template <std::integral A, unsigned64 S>
    constexpr static bool agreeable(S sentinel) {
        return !(sentinel_mask(sentinel) & std::make_unsigned_t<A>(-1));
    }

    template <unsigned64 S>
    constexpr static bool agreeable(S sentinel, size_t payload) {
        VERIFY(1u <= payload && payload <= 7u,
               "payload " << payload,
               return false);

        return !(sentinel_mask(sentinel) &
                 (uint64_t(-1) >> (64 - payload * 8)));
    }

    static void serialize(auto &a, const glyph_t &g) {
        uint64_t u = big_endianize(g.v);
        a.put_raw(&u, 8);
    }

    static glyph_t deserialize(auto &a) {
        uint64_t u;
        a.get_raw(&u, 8);
        glyph_t g;
        g.v = big_endianize(u);
        return g;
    }

    friend std::ostream &operator<<(std::ostream &os, glyph_t g) {
        uint64_t u = g.repr();
        uint8_t *p = reinterpret_cast<uint8_t*>(&u);
        for (unsigned i = 0; i < 8; i++) {
            os << hexos() << std::setw(2) << std::setfill('0')
               << int(p[i]);
            if (i == 1 || i == 3 || i == 5) os << "'";
        }
        return os;
    }
};

static_assert(sizeof(glyph_t) == 8); // in-memory and serialized

namespace glyph {

GLYPH2 (ability,      0xab11'0000'0000'0000, uint8_t, kind,
                                             uint32_t, id);
GLYPH4 (color,        0xc012'0000'0000'0000, uint8_t, r,
                                             uint8_t, g,
                                             uint8_t, b,
                                             uint8_t, a);
GLYPH1 (feldspar_msg, 0xfe1d'5d42'0000'0000, uint32_t, size);
GLYPH48(list,         0x1157'0000'0000'0000, length);
GLYPH48(material,     0x44a7'0000'0000'0000, id);
GLYPH2 (object,       0x0bc1'0000'0000'0000, uint16_t, category,
                                             uint32_t, discriminant);
GLYPH48(objinfo,      0x0b17'0000'0000'0000, mask);
GLYPH1 (player,       0xd1a2'0000'0000'0000, uint32_t, id);
GLYPH3 (version,      0xfe25'0000'0000'0000, uint16_t, major,
                                             uint16_t, minor,
                                             uint16_t, patch);
GLYPH1 (worldmap,     0x44ab'0000'0000'0000, uint16_t, width);

//GLYPH1(buildid,      0xb11d'0000'0000'0000, uint32_t, buildnum);



}
}

// fixme - this doesn't generalize to glyph_t
template<>
struct std::hash<hjx::glyph::object> {
    size_t operator()(const hjx::glyph::object &g) const noexcept {
        return hjx::hash(g.v);
    }
};
