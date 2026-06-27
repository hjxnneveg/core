// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#ifndef ASSERTS_ON
#define ASSERTS_ON true
#endif

#define ASSERTS_OFF (!ASSERTS_ON)

#if ASSERTS_ON
#define IF_ASSERTS_ON(...) __VA_ARGS__
#define IF_ASSERTS_OFF(...)
#define NOTHROW
#define NOTHROWC(x)
#else
#define IF_ASSERTS_ON(...)
#define IF_ASSERTS_OFF(...) __VA_ARGS__
#define NOTHROW noexcept
#define NOTHROWC(x) noexcept(x)
#endif
