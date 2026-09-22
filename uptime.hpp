// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "time.hpp"

#include <boost/noncopyable.hpp>

#include <ostream>

namespace hjx::time {

class uptime_t {
    duration d_;

public:
    uptime_t(duration d): d_(d) {}

    operator duration() const { return d_; }

    uint64_t nanos() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(d_).count();
    }

    float seconds() const {
        return std::chrono::duration<float>(d_).count();
    }

    friend std::ostream &operator<<(std::ostream &os, uptime_t t) {
        return os << tickstring(t.d_.count());
    }
};

inline uptime_t operator+(uptime_t t, duration d) {
    return uptime_t(duration(t) + d);
}

inline uptime_t operator+(duration d, uptime_t t) { return t + d; }


class chronologer : boost::noncopyable {
    const time_point dawn_;
    time_point       last_resumed_;
    duration         uptime_acc_;
    bool             paused_;
    std::mutex       mut_;

public:
    chronologer(): dawn_(clock::now()),
                   last_resumed_(dawn_),
                   uptime_acc_(),
                   paused_(false),
                   mut_() {}

    time_point dawn() { return dawn_; }

    void pause() {
        std::lock_guard _(mut_);
        if (paused_) return;
        uptime_acc_ += clock::now() - last_resumed_;
        paused_ = true;
    }

    void unpause() {
        std::lock_guard _(mut_);
        if (!paused_) return;
        last_resumed_ = clock::now();
        paused_ = false;
    }

    bool paused() { std::lock_guard _(mut_); return paused_; }

    time::uptime_t uptime() {
        std::lock_guard _(mut_);
        if (paused_) return time::uptime_t(uptime_acc_);
        return time::uptime_t(uptime_acc_ + (clock::now() - last_resumed_));
    }
};


inline chronologer &get_chronologer() { RETURN_STATIC_OBJECT(chronologer); }


inline clock::time_point game_start() { return get_chronologer().dawn(); }

inline void pause(/*player_id*/) { get_chronologer().pause(); }
inline void unpause() { get_chronologer().unpause(); }
inline bool paused() { return get_chronologer().paused(); }
inline uptime_t uptime() { return get_chronologer().uptime(); }

// fixme - double
inline float uptime_seconds() {
    auto seconds =
        chrono::duration_cast<chrono::duration<float>>(duration(uptime()));
    return seconds.count();
}

}
