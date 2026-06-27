// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "ringbuffer.hpp"

#include <functional>
#include <ostream>

namespace hjx {

// show clients last acked vs current state
// reset if too few acks
// assume T() is cheap, else consider placement new

// todo - right now this isn't defensive (e.g. to problematic acks)
//        deal with this here or elsewhere?
template <typename T, size_t N>
class delta_buffer {
    using buffer_t = ringbuffer<T, N>;
    using process_t = std::function<void(const T&, const T&)>;

    buffer_t outgoing;
    uint64_t last_sent = 0;
    T acked{};
    uint64_t acked_id = 0;
    process_t process;

    uint64_t first_buffered() const noexcept {
        size_t sz = outgoing.size();
        return sz ? last_sent + 1 - sz : 0;
    }

public:
    explicit delta_buffer(process_t process): process(std::move(process)) {}

    delta_buffer(delta_buffer&&) = delete;
    delta_buffer(const delta_buffer&) = delete;
    delta_buffer &operator=(delta_buffer&&) = delete;
    delta_buffer &operator=(const delta_buffer&) = delete;

    // TBD ~delta_buffer()

    uint64_t update(T &&fulldata) {
        process(acked, fulldata);

        if (outgoing.full()) outgoing.clear();
        outgoing.push_back(std::move(fulldata));
        return ++last_sent;
    }

    // allow missing/out-of-order/dup
    void ack(uint64_t id) {
        ASSERT_GT_LE(id, 0, last_sent); // assume < 18 quintillion

        if (id <= acked_id) return;

        while (outgoing.size() && first_buffered() < id)
            outgoing.pop_front();

        if (first_buffered() != id) return;

        acked = std::move(outgoing[0]);
        acked_id = id;
        outgoing.pop_front();
    }

    friend std::ostream &operator<<(std::ostream &os,
                                    const delta_buffer<T, N> &buffer) {
        os << "[" << buffer.acked_id << "/" << buffer.last_sent
           << " (" << buffer.acked << ")";
        for (const T &e : buffer.outgoing) os << " " << e;
        if (buffer.outgoing.empty()) os << " ∅";
        return os << "]";
    }
};

}
