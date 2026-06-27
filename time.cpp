// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#include "time.hpp"
#include "uptime.hpp"

#include <iomanip>
#include <mutex>
#include <sstream>

#include <boost/noncopyable.hpp>

namespace hjx {

namespace {

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

chronologer &get_chronologer() { RETURN_STATIC_OBJECT(chronologer); }

}

namespace time {

time_point game_start() { return get_chronologer().dawn(); }
void pause()            { get_chronologer().pause(); }
void unpause()          { get_chronologer().unpause(); }
bool paused()           { return get_chronologer().paused(); }
uptime_t uptime()       { return get_chronologer().uptime(); }

// fixme - double
float uptime_seconds() {
    auto seconds =
        chrono::duration_cast<chrono::duration<float>>(duration(uptime()));
    return seconds.count();
}

}

std::string tickstring(int64_t n) {
    ASSERT_GE(n, 0);

    int64_t ks = n / 1'000'000'000'000;
    int64_t  s = n / 1'000'000'000 % 1000;
    int64_t ms = n / 1'000'000 % 1000;
    int64_t us = n / 1'000 % 1000;
    int64_t ns = n % 1000;

    std::ostringstream ss;
    ss << std::setfill('0') << '[';
    ss << std::setw(3) << ks << '\'';
    ss << std::setw(3) <<  s << '\'';
    ss << std::setw(3) << ms << '\'';
    ss << std::setw(3) << us << '\'';
    ss << std::setw(3) << ns << ']';
    return ss.str();
}

}
