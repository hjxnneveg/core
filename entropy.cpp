// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#include "entropy.hpp"

#define XXH_INLINE_ALL
#include "external/xxhash.h"

namespace hjx {

uint64_t hash(const void *p, size_t len) noexcept {
    // todo - check performance for small len
    return XXH3_64bits(p, len);
}

}
