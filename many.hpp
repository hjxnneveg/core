// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "mathbits.hpp"
#include "reporting.hpp"

#include <concepts>
#include <cstdlib>
#include <cstring>
#include <string_view>

namespace hjx {

// Immutable 16-byte noncopyable discriminated value/array.

static_assert(sizeof(void*) == 8);

class alignas(16) many {
    // --- --- --- --- --- --- --- ---   --- --- --- --- --- --- --- ---
    // v0  v1  v2  v3  v4  v5  v6  v7    v8  v9  v10 v11 v12 v13 len tag
    // [v0   ] [v1   ] [v2   ] [v3   ]   [v4   ] [v5   ] [v6   ] len tag
    // [v0           ] [v1           ]   [v2           ]  -   -  len tag
    // [v0                           ]    -   -   -   -   -   -   -  tag
    // [inline string                                          ] len tag
    // [ptr                          ]   [count                ]  -  tag

    char storage[16];

    bool is_alloc() const noexcept { return storage[15] & 0x80; }

    const void *ptr() const NOTHROW {
        ASSERT_MSG(is_alloc(), "non-alloc " << int(storage[15]));
        void *ret;
        std::memcpy(&ret, storage, 8);
        return ret;
    }

    void *ptr() NOTHROW {
        return const_cast<void*>(static_cast<const many*>(this)->ptr());
    }

    void cleanup() noexcept { if (is_alloc()) std::free(ptr()); }

public:
    enum what : char {
        none = 0,
        schar, uchar,
        sshort, ushort,
        sint, uint,
        slong, ulong,
        sllong, ullong,
        float32, float64,
        str
    };

    many() NOTHROW {
        ASSERT_NOT(uintptr_t(this) & 0x0f);
        std::memset(storage, 0, 16);
    }

    many(const many&) = delete;
    many &operator=(const many&) = delete;

    many(many &&o) NOTHROW {
        ASSERT_NOT(uintptr_t(this) & 0x0f);
        std::memcpy(storage, o.storage, 16);
        o.storage[15] = none;
    }

    many &operator=(many &&o) noexcept {
        if (this != &o) {
            cleanup();
            std::memcpy(storage, o.storage, 16);
            o.storage[15] = none;
        }

        return *this;
    }

    ~many() { cleanup(); }

    many(standard_arithmetic auto v) noexcept {
        ASSERT_NOT(uintptr_t(this) & 0x0f);
        std::memcpy(storage, &v, sizeof v);
        storage[15] = to_tag(v);
    }

    template <standard_arithmetic T>
    T get() const NOTHROW {
        ASSERT_MSG(to_tag<T>() == storage[15], "bad tag " << int(storage[15]));
        T ret;
        std::memcpy(&ret, storage, sizeof ret);
        return ret;
    }

    many(std::string_view s) NOTHROW {
        ASSERT_NOT(uintptr_t(this) & 0x0f);

        if (s.size() <= 14) {
            std::memcpy(storage, s.data(), s.size());
            storage[s.size()] = '\0';
            storage[14] = 14 - s.size();
            storage[15] = what::str;
        }
        else {
            ASSERT_LT(s.size(), 0xffff'ffff'ffff);

            char *p = static_cast<char*>(std::malloc(s.size() + 1));
            if (!p) ERROR("bad alloc");
            std::memcpy(p, s.data(), s.size());
            p[s.size()] = '\0';

            std::memcpy(storage, &p, 8);
            set_count(s.size());
            storage[15] = what::str | char(0x80);
        }
    }

    std::string_view get_string() const NOTHROW {
        ASSERT_MSG((storage[15] & 0x7f) == what::str,
                   "bad tag " << int(storage[15]));

        if (storage[15] == what::str)
            return std::string_view(storage, 14 - storage[14]);

        return std::string_view(static_cast<const char*>(ptr()), get_count());
    }

    static_assert(little_endian()); // for count

    size_t get_count() const noexcept {
        uint64_t repr{};
        std::memcpy(&repr, storage + 8, 6);
        return repr;
    }

    void set_count(size_t n) NOTHROW {
        ASSERT_LT(n, 0xffff'ffff'ffff);
        std::memcpy(storage + 8, &n, 6);
    }

private:
    template <standard_arithmetic T>
    static what to_tag(T=T()) noexcept {
        if constexpr (std::same_as<T, float>)              return what::float32;
        if constexpr (std::same_as<T, double>)             return what::float64;
        if constexpr (std::same_as<T, signed char>)        return what::schar;
        if constexpr (std::same_as<T, unsigned char>)      return what::uchar;
        if constexpr (std::same_as<T, short>)              return what::sshort;
        if constexpr (std::same_as<T, unsigned short>)     return what::ushort;
        if constexpr (std::same_as<T, int>)                return what::sint;
        if constexpr (std::same_as<T, unsigned int>)       return what::uint;
        if constexpr (std::same_as<T, long>)               return what::slong;
        if constexpr (std::same_as<T, unsigned long>)      return what::ulong;
        if constexpr (std::same_as<T, long long>)          return what::sllong;
        if constexpr (std::same_as<T, unsigned long long>) return what::ullong;
        std::unreachable();
    }
};

static_assert(sizeof(many) == 16);

}
