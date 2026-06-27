// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "statics.hpp"
#include "reporting.hpp"
#include "entropy.hpp"
#include "archive.hpp"

#include <cstdlib>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>

#include <string.h>

namespace hjx {

inline const char *intern(std::string_view s) {
    using set_t = std::unordered_set<std::string_view>;

    static std::mutex *mut = NEW_STATIC_OBJECT(std::mutex);
    static set_t *symbols = NEW_STATIC_OBJECT(set_t);

    ASSERT(s.data());

    std::lock_guard _(*mut);

    auto it = symbols->find(s);
    if (it != symbols->end()) return it->data();

    const char *p = strndup(s.data(), s.size());
    ASSERT(p);
    auto [elem, added] = symbols->insert(std::string_view(p, s.size()));
    ASSERT(added);
    return elem->data();
}

class token {
    const char *symbol;

    static const char *interntok(std::string_view s) {
        ASSERT(s.data());
        ASSERT_MSG(std::isalpha(s[0]),
                   "'" << s[0] << "' (" << int(s[0]) << ") not alpha");
        return intern(s);
    }

public:
    token(std::nullptr_t): symbol(nullptr) {}
    explicit token(const char *s=nullptr): symbol(s ? interntok(s) : nullptr) {}
    explicit token(std::string_view s): symbol(interntok(s)) {}

    token(const token &o): symbol(o.symbol) {}

    token &operator=(const token &o) {
        symbol = o.symbol;
        return *this;
    }

    const char *sym() const { return symbol; }
    size_t size() const { return symbol ? strlen(symbol) : 0; }

    bool operator==(const token&) const = default;
    auto operator<=>(const token&) const = default;
    explicit operator bool() const { return symbol; }

    uint64_t hash() const { return hjx::hash(reinterpret_cast<uintptr_t>(symbol)); }

    static void serialize(archive &a, token t) {
        a.put_string(t.sym(), t.size());
    }

    template <typename Archive>
    static token deserialize(Archive &a) {
        return token(a.get_string());
    }

    friend std::ostream &operator<<(std::ostream &os, token t) {
        return os << '[' << t.symbol << ']';
    }
};

inline bool operator==(token t, const char *s) {
    return !strcmp(t.sym(), s);
}

inline bool operator==(const char *s, token t) {
    return !strcmp(t.sym(), s);
}

}

namespace std {
template <> struct hash<hjx::token> {
    size_t operator()(const hjx::token &t) const noexcept { return t.hash(); }
};
}
