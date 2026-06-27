// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "archive.hpp"

#include <cmath>
#include <limits>

namespace hjx {

// Assert we stay in range, saturate if we don't.
struct mass_t {
    constexpr static char SEP = '.';
    enum grams_flag { GRAMS };
    using type = uint32_t; // max ~4000 metric tons

    static constexpr type repr_min() { return std::numeric_limits<type>::min(); }

    static constexpr mass_t min() NOTHROW { return mass_t(repr_min(), GRAMS); }

    static constexpr type repr_max() { return std::numeric_limits<type>::max(); }

    static constexpr mass_t max() NOTHROW { return mass_t(repr_max(), GRAMS); }

private:
    type grams_;

public:
    constexpr mass_t() noexcept: grams_(0) {}

    explicit constexpr mass_t(double g, grams_flag) NOTHROW {
        ASSERT_GE_LE(g, repr_min(), repr_max());
        grams_ = g;
    }

    type grams() const { return grams_; }

    bool operator==(const mass_t&) const = default;
    auto operator<=>(const mass_t&) const = default;

    mass_t operator+(const mass_t &o) const {
        ASSERT_LE(grams_, max().grams_ - o.grams_);
        type sum = grams_ + o.grams_;
        sum |= -(sum < grams_);
        return mass_t(sum, GRAMS);
    }

    mass_t operator-(const mass_t &o) const {
        ASSERT_GE(grams_, o.grams_);
        type diff = grams_ - o.grams_;
        diff &= -(grams_ >= o.grams_);
        return mass_t(diff, GRAMS);
    }

    mass_t &operator+=(const mass_t &o) {
        ASSERT_LE(o, max() - *this);
        return *this = *this + o;
    }

    std::ostream &dumpfix(std::ostream &os) const { // fixme - i18n
        if (grams() < 1000) // 123 => 0.123kg
            return os << "0."
                      << std::setw(3) << std::setfill('0')
                      << grams() << "kg";

        if (grams() < 10000) // 1234 => 1.234kg
            return os << grams() / 1000 << "."
                      << std::setw(3) << std::setfill('0')
                      << grams() % 1000 << "kg";

        if (grams() < 100'000) // 12'345 => 12.34kg
            return os << grams() / 1000 << "."
                      << std::setw(2) << std::setfill('0')
                      << grams() % 1000 / 10 << "kg";

        if (grams() < 1'000'000) // 123'456 => 123.4kg
            return os << grams() / 1000 << "."
                      << grams() % 1000 / 100 << "kg";

        if (grams() < 10'000'000) // 1'234'567 => 1234.kg
            return os << grams() / 1000 << ".kg";

        if (grams() < 100'000'000) // 12'345'678 => 12345kg
            return os << grams() / 1000 << "kg";

        WARNING(grams() << " grams is too many"); // 99'999.999 kg max

        return os << "hrairkg";
    }

    static void serialize(archive &a, mass_t m) {
        a.put(m.grams());
    }

    static mass_t deserialize(auto &a) {
        mass_t::type grams;
        a >> grams;
        return mass_t(grams, mass_t::GRAMS);
    }

    friend std::ostream &operator<<(std::ostream &os, mass_t m) {
        os << m.grams() / 1000;

        if (type frac = m.grams() % 1000) {
            char buf[4];
            sprintf(buf, "%03u", unsigned(frac));

            if (buf[2] == '0') {
                buf[2] = '\0';
                if (buf[1] == '0') buf[1] = '\0';
            }

            os << SEP << buf;
        }

        return os << "kg";
    }
};

inline mass_t operator*(mass_t m, double val) {
    ASSERT_GE(val, 0);
    ASSERT_MSG(m.grams() * val < mass_t::repr_max(), m << " * " << val);
    return mass_t(m.grams() * val, mass_t::GRAMS);
}

inline mass_t operator*(double val, mass_t m) { return m * val; }

inline mass_t operator ""_kg(long double kg) {
    return mass_t(std::round(kg * 1000), mass_t::GRAMS);
}

inline mass_t operator ""_kg(unsigned long long kg) {
    return mass_t(kg * 1000, mass_t::GRAMS);
}

}
