#pragma once

namespace advent {

    struct _discard_fn {
        template<typename... Args>
        constexpr void operator ()(Args &&... args) const {
            (static_cast<void>(args), ...);
        }
    };

    constexpr inline _discard_fn discard{};

}
