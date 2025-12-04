export module advent:print;

import std;

namespace advent {

    /*
        These functions only print outside of constant evaluation.

        Mostly useful for debugging.
    */

    export template<typename... Args>
    constexpr void print(std::format_string<Args...> fmt, Args &&... args) {
        if !consteval {
            std::print(std::move(fmt), std::forward<Args>(args)...);
        }
    }

    export template<typename... Args>
    constexpr void println(std::format_string<Args...> fmt, Args &&... args) {
        if !consteval {
            std::println(std::move(fmt), std::forward<Args>(args)...);
        }
    }

    export constexpr void println() {
        if !consteval {
            /* TODO: Just call the proper overload when we receive an implementation of it. */
            std::print("\n");
        }
    }

}
