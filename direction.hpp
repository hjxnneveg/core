// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"

namespace hjx {

// unique values for
// edges: NORTH NE SE SOUTH SW NW
// vertices: NE EAST SE SW WEST NW
enum class direction : uint8_t {
    none  = 0,
    north = 1, ne = 2, east  = 1, se = 3,
    south = 4, sw = 5, west  = 4, nw = 6,
    end = 7
};

constexpr bool valid(direction d) { return d < direction::end; }

//           -----           //
//          /     \          //
//     -----   1   -----     //
//    /     6     2     \    //
//   -   6   -----   2   -   //
//    \     /     \     /    //
//     - 4 -   0   - 1 -     //
//    /     \     /     \    //
//   -   5   -----   3   -   //
//    \     5     3     /    //
//     -----   4   -----     //
//          \     /          //
//           -----           //

constexpr direction clockwise_vert(direction d) {
    return direction((0x2465130 >> (uint32_t(d) << 2)) & 7);
}

inline std::ostream &operator<<(std::ostream &os, direction d) {
    switch (d) {
    case direction::none:  return os << "none";
    case direction::north: return os << "N or E";
    case direction::ne:    return os << "NE";
    case direction::se:    return os << "SE";
    case direction::south: return os << "S or W";
    case direction::sw:    return os << "SW";
    case direction::nw:    return os << "NW";
    default:               return os << "bad direction " << int(d);
    }
}

inline std::string edgestr(direction d) {
    switch (int(d)) {
    case 0: return "none";
    case 1: return "north";
    case 2: return "ne";
    case 3: return "se";
    case 4: return "south";
    case 5: return "sw";
    case 6: return "nw";
    default: return TO_STRING("bad dir " << int(d));
    }
}

inline std::string vertstr(direction d) {
    switch (int(d)) {
    case 0: return "none";
    case 1: return "east";
    case 2: return "ne";
    case 3: return "se";
    case 4: return "west";
    case 5: return "sw";
    case 6: return "nw";
    default: return TO_STRING("bad dir " << int(d));
    }
}

constexpr direction NORTH = direction::north;
constexpr direction NE    = direction::ne;
constexpr direction EAST  = direction::east;
constexpr direction SE    = direction::se;
constexpr direction SOUTH = direction::south;
constexpr direction SW    = direction::sw;
constexpr direction WEST  = direction::west;
constexpr direction NW    = direction::nw;

}
