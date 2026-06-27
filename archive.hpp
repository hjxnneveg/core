// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "glyph.hpp"

#include <bit>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <vector>

namespace hjx {

template <typename T>
concept archive_simple = std::is_arithmetic_v<T> || std::is_enum_v<T>;

class archive {
    using byte_t = unsigned char;

    std::vector<byte_t> data_;

    template <typename T>
    constexpr static bool is_simple() { return archive_simple<T>; }

    template <typename T>
    constexpr static void assert_simple() { static_assert(is_simple<T>()); }

    template <trivially_copyable T>
    static auto repr(T v) {
        assert_simple<T>();
        return little_endianize(to_uint(v));
    }

    template <trivially_copyable T, std::integral R>
    static T unrepr(R v) {
        assert_simple<T>();
        return std::bit_cast<T>(little_endianize(v));
    }

    byte_t *alloc(size_t len) {
        size_t old_size = data_.size();
        data_.resize(old_size + len);
        return data_.data() + old_size;
    }

    constexpr static size_t MAX_STRLEN = 10000; // or so?

public:
    archive() {}

    archive(glyph_t header) { put(header); }

    archive(const byte_t *bytes, size_t len): data_(bytes, bytes + len) {}

    archive(archive&&) noexcept = default;
    archive &operator=(archive&&) noexcept = default;

    archive(const archive&) = delete;
    archive &operator=(const archive&) = delete;

    void clear() { data_ = std::vector<byte_t>(); }

    void set_header(glyph_t header) {
        uint64_t repr = header.repr();
        if (data_.size() < 8) data_.resize(8);
        std::memcpy(data_.data(), &repr, 8);
    }

    glyph_t header() const { return glyph_t(peek<uint64_t>(0)); }

    byte_t *prepare(glyph_t header, size_t bodylen) {
        uint64_t repr = header.repr();
        data_.resize(8 + bodylen);
        std::memcpy(data_.data(), &repr, 8);
        return data_.data() + 8;
    }

    const byte_t *data() const { return data_.data(); }
    size_t size() const { return data_.size(); }

    template <typename T>
    archive &operator<<(const T &v) { put(v); return *this; }

    template <typename T>
    archive &put(const T &v) {
        ASSERT_LT(data_.size(), 40'000'000); // big-message alarm ~40MB

        if constexpr (is_simple<T>()) {
            auto u = repr(v);
            std::memcpy(alloc(sizeof(T)), &u, sizeof(T));
        }
        else {
            T::serialize(*this, v);
        }

        return *this;
    }

    archive &put_raw(const void *p, size_t len) {
        ASSERT(p);
        ASSERT(len);
        std::memcpy(alloc(len), p, len);
        return *this;
    }

    // No overloads for put(const char*), put(char[N]), etc because we can't
    // distinguish between implicit and explicit nuls because of how C++
    // deduces types for things like this because reasons:
    //
    // template <typename T>
    // void foo(const T &v) { cout << type2str<T>() << endl; }
    //
    // foo("abc"); // yields "char [4]"
    //
    // consider string_view?

    archive &put_string(const void *s, size_t len) {
        ASSERT_LE(len, MAX_STRLEN);
        ASSERT_IMPLY(len, s);
        if (len < 0xff)
            put<uint8_t>(len);
        else {
            put<uint8_t>(0xff);
            put<uint64_t>(len);
        }
        if (len) std::memcpy(alloc(len), s, len);
        return *this;
    }

    class cursor_t {
        const archive &a;
        size_t i;

    public:
        cursor_t(const archive &a, size_t offset): a(a), i(offset) {}

        size_t remaining() const { return a.size() - i; }

        template <archive_simple T>
        T peek() const {
            ASSERT_GE(remaining(), sizeof(T));
            phony_uint<T> u;
            std::memcpy(&u, a.data() + i, sizeof(T));
            return unrepr<T>(u);
        }

        template <typename T>
        T get() {
            if constexpr (is_simple<T>()) {
                T ret = peek<T>();
                i += sizeof(T);
                return ret;
            }
            else {
                return T::deserialize(*this);
            }
        }

        void get_raw(void *p, size_t len) {
            ASSERT_GE(remaining(), len);
            std::memcpy(p, a.data() + i, len);
            i += len;
        }

        string get_string() {
            size_t len = get<uint8_t>();
            if (len == 0xff) len = get<uint64_t>();
            ASSERT_LE(len, MAX_STRLEN);
            ASSERT_GE(remaining(), len);
            const char *p = reinterpret_cast<const char*>(a.data() + i);
            i += len;
            return string(p, len);
        }

        glyph_t get_glyph() { return get<glyph_t>(); }

        // convenience to avoid keyword `a.template get<type>()`
        float    get_float()  { return get<float>(); }
        double   get_double() { return get<double>(); }
        int8_t   get_int8()   { return get<int8_t>(); }
        uint8_t  get_uint8()  { return get<uint8_t>(); }
        int16_t  get_int16()  { return get<int16_t>(); }
        uint16_t get_uint16() { return get<uint16_t>(); }
        int32_t  get_int32()  { return get<int32_t>(); }
        uint32_t get_uint32() { return get<uint32_t>(); }
        int64_t  get_int64()  { return get<int64_t>(); }
        uint64_t get_uint64() { return get<uint64_t>(); }
        size_t   get_size_t() { return get<size_t>(); }

        template <typename T>
        cursor_t &operator>>(T &v) { v = get<T>(); return *this; }
    };

    cursor_t cursor(size_t offset=0) const { return cursor_t(*this, offset); }

    template <archive_simple T>
    T peek(size_t offset) const { return cursor(offset).peek<T>(); }

    void send(std::ostream &os) {
        os.write(reinterpret_cast<const char*>(data_.data()),
                 data_.size());
        // todo - error handling
    }

    std::ostream &dump(std::ostream &os, unsigned head, unsigned tail) const {
        const unsigned max = head + tail;

        auto output = [&](byte_t byte) {
            os << hexos() << std::setw(2) << std::setfill('0')
               << int(byte) << " ";
        };

        size_t sz = data_.size();
        if (sz <= max) for (byte_t b : data_) output(b);
        else {
            for (unsigned i = 0; i < head; i++) output(data_[i]);
            os << ".. ";
            for (unsigned i = 0; i < tail; i++) output(data_[sz - tail + i]);
        }

        return os << "(" << sz << " bytes)";
    }

    friend std::ostream &operator<<(std::ostream &os, const archive &a) {
        return a.dump(os, 10, 10);
    }
};

template <>
inline archive &archive::put(const string &s) {
    return put_string(s.c_str(), s.size());
}

template <>
inline string archive::cursor_t::get() { return get_string(); }

}
