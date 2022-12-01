#pragma once

#include <advent/includes.hpp>

namespace advent {

    /* TODO: Remove this when I receive an [[assume]] implementation. */
    constexpr void assume(const bool success) {
        if (!success) {
            std::unreachable();
        }
    }

}
