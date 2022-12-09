#pragma once

#include <advent/common.hpp>

namespace advent {

    template<typename... Args>
    constexpr void print(fmt::format_string<Args...> fmt, Args &&... args) {
        /*
            This function only prints outside of constant evaluation.

            Mostly useful for debugging.
        */

        if !consteval {
            fmt::print(std::move(fmt), std::forward<Args>(args)...);
        }
    }

}
