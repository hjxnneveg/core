// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#pragma once

#include "entropy.hpp"
#include "reporting.hpp"

// generational array
// contiguousish storage, fast lookup, no reused IDs

namespace hjx {

class genid {
public:
    // [----] [----] [----] [----] [----] [----] [----] [----]
    // [generation        ] [TBD ] [nonzero index            ]
    //
    // generation is odd for every issued id (see genarray)

    static constexpr uint32_t GEN_MAX = 0xff'ffff;

private:
    uint64_t storage = 0;

    genid(uint32_t index, uint32_t gen): storage(uint64_t(gen) << 40 | index) {
        ASSERT(index);
        ASSERT_LE(gen, GEN_MAX);
    }

    template <typename T> friend class genarray;

public:
    genid() = default;

    explicit operator bool() const { return storage; }
    friend bool operator==(genid, genid) = default;

    uint32_t index() const { return storage & 0xffff'ffff; }

    uint32_t generation() const {
        ASSERT(*this);
        return storage >> 40;
    }

    uint64_t raw() const { return storage; }

    friend ostream &operator<<(ostream &os, genid id) {
        if (!id) return os << "[genid nil]";
        return os << "[genid " << id.index() << " #" << id.generation() << "]";
    }
};

}

template<>
struct std::hash<hjx::genid> {
    size_t operator()(const hjx::genid &id) const noexcept { return hjx::hash(id.raw()); }
};

namespace hjx {

// The union holds the payload while occupied and the freelist while free.  Occupancy
// is the generation's low bit: even => free, odd => occupied.  Insert and erase each
// increment, so every reuse changes generation and stale ids fail compare in get().
//
// The freelist is FIFO to spread reuse evenly.  Slot 0 is reserved for nil.  A slot
// whose generation reaches GEN_MAX is retired.

template <typename T>
class genarray {
    static_assert(std::is_nothrow_move_constructible_v<T>); // for vector

    struct slot {
        uint32_t gen = 0; // low bit: 1 = occupied, persists while free

        union {
            uint32_t next; // if free (next free index, 0 = end of list)
            T payload;     // if occupied
        };

        bool occupied() const { return gen & 1; }

        slot(): next(0) {}

        slot(slot &&o) noexcept: gen(o.gen) {
            if (o.occupied()) new (&payload) T(std::move(o.payload));
            else next = o.next;
        }

        ~slot() { if (occupied()) payload.~T(); }

        slot(const slot&) = delete;
        slot &operator=(const slot&) = delete;
        slot &operator=(slot&&) = delete;
    };

    vector<slot> slots; // slots[0] reserved
    uint32_t free_head = 0;
    uint32_t free_tail = 0;
    size_t live_ = 0;
    size_t retired_ = 0;

    static constexpr uint32_t GEN_MAX = genid::GEN_MAX;

    genid id_at(uint32_t idx) const {
        ASSERT_LT(idx, slots.size());
        ASSERT_MSG(slots[idx].gen & 1, "non-odd gen " << slots[idx].gen);
        return genid(idx, slots[idx].gen);
    }

public:
    genarray(): slots(1) {}

    size_t live() const { return live_; }
    size_t retired() const { return retired_; }

    void reserve(size_t n) { slots.reserve(n + 1); }

    template <typename ...Args>
    genid emplace(Args &&...args) {
        uint32_t idx;

        if (free_head) {
            idx = free_head;
            free_head = slots[idx].next;
            if (!free_head) free_tail = 0;
        }
        else {
            ASSERT_LT(slots.size(), 0xffff'ffff);
            slots.emplace_back();
            idx = uint32_t(slots.size() - 1);
        }

        slot &s = slots[idx];
        new (&s.payload) T(std::forward<Args>(args)...);
        s.gen++; // even -> odd: occupied, new generation
        live_++;
        return genid(idx, s.gen);
    }

    genid insert(T x) { return emplace(std::move(x)); }

    // nil or stale id is ok, if right pool
    const T *get(genid id) const {
        if (!id) return nullptr;

        uint32_t idx = id.index();
        ASSERT_MSG(idx < slots.size(), id << " vs " << slots.size());

        const slot &s = slots[idx];
        ASSERT_LE(id.generation(), s.gen);

        if (s.gen != id.generation()) return nullptr;

        return &s.payload;
    }

    T *get(genid id) { return const_cast<T*>(std::as_const(*this).get(id)); }

    // false if id is nil or stale
    bool erase(genid id) {
        T *p = get(id);
        if (!p) return false;

        uint32_t idx = id.index();
        slot &s = slots[idx];
        s.payload.~T();
        s.gen++; // odd -> even: free/stale ids now miss
        live_--;

        if (s.gen < GEN_MAX) {
            // FIFO
            s.next = 0;
            if (free_tail) slots[free_tail].next = idx;
            else free_head = idx;
            free_tail = idx;
        }
        else {
            retired_++; // no wrapping, just leak
        }

        return true;
    }

    template <bool Const>
    class iter {
        using List = std::conditional_t<Const, const genarray, genarray>;
        List *list;
        uint32_t idx;

        void seek() {
            while (idx < list->slots.size() && !list->slots[idx].occupied())
                idx++;
        }

        friend genarray;
        iter(List *list, uint32_t idx) : list(list), idx(idx) { seek(); }

    public:
        auto &operator*() const { return list->slots[idx].payload; }
        auto *operator->() const { return &list->slots[idx].payload; }

        genid id() const { return list->id_at(idx); }

        iter &operator++() {
            idx++;
            seek();
            return *this;
        }

        friend bool operator==(iter, iter) = default;
    };

    auto begin() { return iter<false>(this, 1); }
    auto end() { return iter<false>(this, uint32_t(slots.size())); }
    auto begin() const { return iter<true>(this, 1); }
    auto end() const { return iter<true>(this, uint32_t(slots.size())); }

    friend ostream &operator<<(ostream &os, const genarray &a) {
        size_t free = a.slots.size() - 1 - a.live() - a.retired();
        os << "[genarray live " << a.live() << " free " << free;
        if (a.retired()) os << " retired " << a.retired();
        return os << "]";
    }
};

}
