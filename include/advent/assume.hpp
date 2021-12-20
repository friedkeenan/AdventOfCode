#pragma once

namespace advent {

    /* TODO: Remove this when 'std::unreachable' is accepted and implemented. */
    [[noreturn]]
    void unreachable() {
        __builtin_unreachable();
    }

    /* TODO: Remove this when an 'assume' facility is accepted and implemented. */
    struct _assume_fn {
        constexpr void operator ()(const bool success) const {
            if (!success) {
                advent::unreachable();
            }
        }
    };

    constexpr inline _assume_fn assume{};

}
