// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "finale.hpp"
#include "logging.hpp"

#include <cstdint>
#include <vector>

namespace hjx {

DEFINE_LOG(registry, false, LOG_TIMESTAMP | LOG_THREADNAME)

// A registry is an collection of elements, each with a unique id.  Adding
// an element returns a finale which removes the element on destruction.
template <typename T>
class registry {
public:
    struct entry { uint64_t id; T elem; };

private:
    uint64_t idgen = 0;
    std::vector<entry> entries;

public:
    finale add(T &&elem) {
        ASSERT_LT(idgen, MAXVAL(idgen)); // I mean, it'll never happen...
        ASSERT_LT(entries.size(), 500); // yellow alert--if valid,
                                        // consider O(lg N) data structure
        uint64_t id = ++idgen;
        LOG(registry, "adding #" << id << ": " << elem);
        entries.emplace_back(id, std::move(elem));

        return finale([id, this] {
            IF_ASSERTS_ON(bool removed = false);
            size_t sz = entries.size();

            for (size_t i = 0; i < sz; i++) {
                if (entries[i].id == id) {
                    ASSERT(!removed);
                    LOG(registry,
                        "removing #" << id << ": " << entries[i].elem);

                    if (i < sz - 1) {
                        // Just move the last one into this position.
                        // We don't care about order.
                        ASSERT_NE(entries[sz - 1].id, id);
                        entries[i] = std::move(entries[sz - 1]);
                    }

                    entries.pop_back();
                    sz -= 1;
                    IF_ASSERTS_ON(removed = true);
                }
            }

            ASSERT(removed);
        });
    }

    finale add(const T &elem) {
        T copy = elem; // rely on the compiler
        return add(std::move(copy));
    }

    typename std::vector<entry>::const_iterator begin() const {
        return entries.begin();
    }

    typename std::vector<entry>::const_iterator end() const {
        return entries.end();
    }
};

}
