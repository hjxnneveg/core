// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "statics.hpp"

#include <functional> // fixme
#include <limits>
#include <mutex>
#include <ostream>
#include <ranges>
#include <vector>
#include <unordered_map>

namespace hjx {

// consider when proxy needs to be threadsafe

// A proxy is a wrapper around an ID mapping to a keyed, pre-registered Proto.
template <typename Index, typename Key, typename Proto>
class proxy {
    struct entry { Key key{}; Proto proto{}; };

    // private because not thread safe
    // (Key key) -> (Index id) -> (Proto&)

    using map_t = std::unordered_map<Key, Index>;

    static map_t &keys() { RETURN_STATIC_OBJECT(map_t); }

    static std::vector<entry> &protos_() {
        static auto *ret = ([]() {
            auto *v = new (std::nothrow) std::vector<entry>;
            v->reserve(8); // assume 7+ entries
            v->emplace_back(Key{}, Proto{}); // dummy entry 0 considered "null"
            return v;
        })();

        return *ret;
    }

    static auto protos() { return protos_() | std::views::drop(1); }

    // reconsider mutex
    static std::mutex &mut() { RETURN_STATIC_OBJECT(std::mutex); }

    Index id_;

public:
    using index_t = Index;
    using proto_t = Proto;

    proxy(Index id=0): id_(id) { ASSERT_GE_LT(id, 0, protos_().size()); }

    proxy(Key key) {
        std::lock_guard _(mut());
        id_ = keys()[key];
        ASSERT_MSG(id_, "can't find " << key);
    }

    static Index add(Key key, const Proto &proto) { // todo - rvalue/forwarding
        std::lock_guard _(mut());
        ASSERT_GE_LT(protos_().size(), 1u, std::numeric_limits<Index>::max());

        Index &index = keys()[key];
        ASSERT_EQ(index, 0);
        index = protos_().size();
        protos_().emplace_back(key, proto);
        return index;
    }

    static size_t count() { std::lock_guard _(mut()); return keys().size(); }

    Index id() const { return id_; }

    explicit operator bool() const { return id(); }

    bool operator<(const proxy<Index, Key, Proto> &o) const {
        return id() < o.id();
    }

    bool operator==(const proxy<Index, Key, Proto> &o) const {
        return id() == o.id();
    }

    Key key() const {
        std::lock_guard _(mut());
        return protos_()[id()].key;
    }

    Key name() const { return key(); } // for token Keys, mostly

    template <typename T>
    T inspect(const std::function<T(const Proto&)> &f) const {
        std::lock_guard _(mut());
        return f(protos_()[id()].proto);
    }

    Proto copy_proto() const { // return copy for thread safety
        std::lock_guard _(mut());
        return protos_()[id()].proto;
    }

    static void foreach(auto &&f) {
        std::lock_guard _(mut());
        for (const entry &e : protos()) f(e.key, e.proto);
    }

    friend std::ostream &operator<<(std::ostream &os,
                                    const proxy<Index, Key, Proto> &p) {
        return p ? os << p.key() : os << "(none)";
    }

    static std::ostream &dump(std::ostream &os) {
        foreach([&](Key key, const Proto &proto) {
            os << '[' << key.sym() << " : " << proto << ']' << std::endl;
        });

        return os;
    }
};

#define PROTO_GET(type, suffix) \
    (this->inspect<type>([](const proto_t &p) { return p.suffix; }))

}
