// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace hjx::version {

constexpr uint16_t major = 0;
constexpr uint16_t minor = 0;
constexpr uint16_t patch = 1;

constexpr const char *timestamp = __DATE__ " " __TIME__;

inline uint32_t buildnum() {
    std::tm tm = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%b %d %Y %H:%M:%S");

    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return std::chrono::duration_cast<std::chrono::seconds>
        (tp.time_since_epoch()).count() / 6; //™
}

inline const char *get() {
    auto make_version_string = [] {
        std::ostringstream ss;
        ss << major << "." << minor;
        if (patch) ss << "." << patch;
        ss << " #" << buildnum() << " [" << timestamp << "]";
        return new std::string(ss.str());
    };

    static std::string *version_string = make_version_string();
    return version_string->c_str();
}

}
