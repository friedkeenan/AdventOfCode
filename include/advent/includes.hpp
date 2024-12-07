#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <compare>
#include <limits>
#include <array>
#include <span>
#include <string_view>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <utility>
#include <functional>
#include <numeric>
#include <iterator>
#include <ranges>
#include <algorithm>
#include <type_traits>
#include <concepts>

#ifdef ADVENT_TIME_SOLUTIONS

#include <chrono>

#endif

/*
    GCC was raising erroneous warnings for overflowing
    string operations in fmtlib, so we disable them here.

    I'm sure nothing bad can come from this.
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"

#include <fmt/format.h>

#ifdef ADVENT_TIME_SOLUTIONS

#include <fmt/chrono.h>

#endif

#pragma GCC diagnostic pop
