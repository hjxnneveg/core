// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"

namespace hjx {

template <typename T, size_t N>
class ringbuffer {
public:
    constexpr static size_t capacity = N;

    T arr_[N] = {};
    T *begin_;
    size_t size_;

public:
    ringbuffer() noexcept: begin_(arr_), size_(0) {}

    const T *to_pointer(size_t index) const {
        ASSERT_LE(index, N);

        if (index == N) return arr_ + N;
        if (begin_ - arr_ < ptrdiff_t(N - index)) return begin_ + index;
        return begin_ - (N - index);
    }

    T *to_pointer(size_t index) {
        return const_cast<T*>(std::as_const(*this).to_pointer(index));
    }

    struct iterator {
        const ringbuffer<T, N> &buf;
        size_t index;

        iterator(const ringbuffer<T, N> &buf, size_t index): buf(buf), index(index) {
            ASSERT_GE_LE(index, 0, N);
        }

        const T &operator*() const { return *buf.to_pointer(index); }

        bool operator==(const iterator &o) const {
            ASSERT_EQ(&buf, &o.buf);
            return &buf == &o.buf && index == o.index;
        }

        iterator &operator++() {
            ASSERT_LT(index, buf.size());
            if (++index == buf.size()) index = N;
            return *this;
        }

        iterator operator++(int) {
            iterator ret = *this;
            ++(*this);
            return ret;
        }
    };

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == N; }

    iterator begin() const { return empty() ? end() : iterator(*this, 0); }
    iterator end() const { return iterator(*this, N); }

    T &operator[](size_t index) {
        ASSERT_LT(index, size());
        return *to_pointer(index);
    }

    const T &operator[](size_t index) const {
        ASSERT_LT(index, size());
        return *to_pointer(index);
    }

    void push_back(const T &t) {
        ASSERT_LT(size_, N);
        *to_pointer(size_++) = t;
    }

    void push_back(T &&t) {
        ASSERT_LT(size_, N);
        *to_pointer(size_++) = std::move(t);
    }

    void pop_front() {
        ASSERT(size_);
        *begin_ = T();
        if (++begin_ == arr_ + N) begin_ = arr_;
        size_--;
    }

    void clear() { while (size()) pop_front(); }

    friend std::ostream &operator<<(std::ostream &os, const ringbuffer<T, N> &b) {
        os << "[";
        const char *sep = "";
        for (const T &e : b) { os << sep << e; sep = " "; }
        return os << "]";
    }
};

}
