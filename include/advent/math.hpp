#pragma once

#include <advent/common.hpp>

namespace advent {

    struct _pow_fn {
        template<std::integral Base, std::integral Exponent>
        [[nodiscard]]
        constexpr Base operator ()(const Base base, const Exponent exponent) const {
            advent::assume(exponent >= 0);

            if (exponent == 0) {
                return 1;
            }

            return base * (*this)(base, exponent - 1);
        }
    };

    constexpr inline auto pow = _pow_fn{};

}
