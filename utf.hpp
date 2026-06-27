// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "entropy.hpp"

#include "external/fast_float/fast_float.h"
#include "external/utfcpp/source/utf8.h"

#include <charconv>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace hjx {

inline char32_t to_codepoint(std::string_view s) {
    auto it = s.begin();
    auto end = s.end();
    char32_t c = utf8::next(it, end);
    if (it == end) return c;
    WARNING("bad codepoint " << unsigned(c));
    return 0;
}

struct utf32_t {
    char32_t value;

    constexpr utf32_t(): value(0) {}

    utf32_t(char32_t c): value(c) {
        ASSERT_MSG(c <= 0x10ffff &&
                   !(c >= 0xd800 && c <= 0xdfff),
                   "bad codepoint " << hexos() << uint32_t(c));
    }

    utf32_t(std::string_view s): value(to_codepoint(s)) {}

    char32_t repr() const { return value; }
    operator char32_t() const { return value; }

    bool isalpha() const {
        return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z');
    }

    bool isdigit() const { return value >= '0' && value <= '9'; }

    bool isalnum() const { return isalpha() || isdigit(); }

    bool isspace() const {
        return value == ' ' || value == '\t'
            || value == '\r' || value == '\n'
            || value == '\f' || value == '\v';
    }

#define DIACHECK_(from, to)                     \
    do {                                        \
        if (value < from) return false;         \
        if (value <= to) return true;           \
    } while (false)

    // incomplete, but probably more complete than necessary
    bool diacritic() const {
        DIACHECK_(0x0300, 0x036f); // Combining Diacritical Marks
        DIACHECK_(0x1ab0, 0x1aff); // Combining Diacritical Marks Extended
        DIACHECK_(0x1dc0, 0x1dff); // Combining Diacritical Marks Supplement
        DIACHECK_(0x20d0, 0x20ff); // Combining Diacritical Marks for Symbols
        DIACHECK_(0xfe20, 0xfe2f); // Combining Half Marks

        DIACHECK_(0x1d167, 0x1d169); // Combining Musical Symbols
        DIACHECK_(0x1d17b, 0x1d182);
        DIACHECK_(0x1d185, 0x1d18b);
        DIACHECK_(0x1d1aa, 0x1d1ad);

        return false;
    }

#undef DIACHECK_

    friend std::ostream &operator<<(std::ostream &os, utf32_t c) {
        std::string s;
        utf8::append(c.value, s);
        return os << s;
    }
};

constexpr bool utf8_boundary(char c) { return ((c & 0xc0) != 0x80); }

inline std::string some_of(const char *s) {
    if (!s) return std::string{};

    constexpr int max = 42;

    const char *end = s;
    for (int i = 0; i < max * 4; i++)
        if (!*end) break;
        else end++;

    while (!utf8_boundary(*end)) end--;

    std::string ret;

    int i = 0;
    for (; i < max; i++) {
        if (s >= end) break;
        utf32_t c = utf8::next(s, end);
        if (c == '\n') utf8::append(U'↲', ret);
        else utf8::append(c, ret);
    }

    return i == max ? ret + "…": ret + "␀";
}

namespace utf {

inline const char *skipspace(const char *it, const char *end) {
    ASSERT(it);
    ASSERT_GE(end, it);

    for (;;) {
        utf32_t c = utf8::peek_next(it, end);
        if (!c.isspace()) break;
        utf8::next(it, end);
    }

    return it;
}

// throws on bad utf8
class feed {
    const char *start_;
    const char *cur_;
    const char *end_;
    int line_;
    int col_;

public:
    feed(std::string_view s): start_(&*s.begin()),
                              cur_(&*s.begin()),
                              end_(&*s.end()),
                              line_(1),
                              col_(0) {
        // todo - utf8::starts_with_bom
        ASSERT(start_);
        ASSERT_MSG(utf8_boundary(*start_), int(*start_));
        ASSERT(end_);
        ASSERT_MSG(utf8_boundary(*end_), int(*start_));
    }

    bool done() const { return cur_ == end_; }

    int line() const { return line_; }
    int col() const { return col_; }

    utf32_t peek() const {
        if (done()) return 0;
        utf32_t c = utf8::peek_next(cur_, end_);
        if (c == '\r') return '\n';
        return c;
    }

    utf32_t next() {
        if (done()) return 0;

        utf32_t p = prev();
        utf32_t c = utf8::next(cur_, end_);

        if (c == '\r') {
            c = '\n';
            if (cur_ != end_ && utf8::peek_next(cur_, end_) == '\n')
                utf8::next(cur_, end_);
        }

        if (p == '\n') { line_++; col_ = 1; }
        else if (!c.diacritic()) col_++;

        return c;
    }

    utf32_t prev() const {
        if (cur_ == start_) return 0;
        const char *cur = cur_;
        utf32_t c = utf8::prior(cur, start_);
        if (c == '\r') return '\n';
        return c;
    }

    template <typename... C>
    utf32_t consume(C... c) {
        utf32_t front = peek();
        if (((front != static_cast<utf32_t>(c)) && ...)) return 0;
        return next();
    }

    feed &skipspace() {
        for (;;) {
            utf32_t c = peek();
            if (!c.isspace()) break;
            next();
        }

        return *this;
    }

    void skip_blank_lines() {
        for (auto x = *this;;) {
            utf32_t c = x.next();
            if (c == ' ' || c == '\t' || c == '\f' || c == '\v') continue;
            if (c != '\n') return;
            *this = x;
        }
    }

    bool parse_uint(uint64_t *p) {
        uint64_t value;
        auto result = std::from_chars(cur_, end_, value);
        if (result.ec != std::errc()) return false;
        *p = value;
        col_ += result.ptr - cur_;
        cur_ = result.ptr;
        return true;
    }

    std::optional<uint64_t> parse_uint() {
        uint64_t v;
        if (parse_uint(&v)) return v;
        return {};
    }

    bool parse_double(double *p) {
        double value;
        // fixme - clang
        auto result = fast_float::from_chars(cur_, end_, value);
        if (result.ec != std::errc()) return false;
        *p = value;
        col_ += result.ptr - cur_;
        cur_ = result.ptr;
        return true;
    }

    std::string_view parse_word(auto &&f) {
        if (!peek().isalpha()) return std::string_view{};
        const char *from = iter();
        while (f(peek())) next();
        return std::string_view{from, iter()};
    }

    std::string_view parse_word() {
        return parse_word([](utf32_t c) { return c.isalpha(); });
    }

#if 0
    // fixme - assumes T{} ≡ false/none, not true for decimal
    template <typename T>
    T parse() {
        const char *before = cur_;
        T ret = T::parse(*this);
        (void)before;
        ASSERT_MSG((cur_ == before) == !ret,
                   "bytes: " << (cur_ - before) << " value: " << ret);
    }
#endif

    const char *iter() const { return cur_; }

    friend ostream &operator<<(ostream &os, const feed &x) {
        return os << some_of(x.iter());
    }
};

}

}

template<>
struct std::hash<hjx::utf32_t> {
    constexpr size_t operator()(hjx::utf32_t codepoint) const noexcept {
        return hjx::hash(codepoint.value);
    }
};
