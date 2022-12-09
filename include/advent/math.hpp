#pragma once

#include <advent/common.hpp>

namespace advent {

    struct _pow_fn {
        template<std::integral Base, std::integral Exponent>
        [[nodiscard]]
        constexpr Base operator ()(Base base, Exponent exponent) const {
            /* This is a binary pow implementation. */

            advent::assume(exponent >= 0);

            if (exponent == 0) {
                return 1;
            }

            Base result = 1;
            while (true) {
                if (exponent % 2 == 1) {
                    /* If the exponent is odd, we multiply by the base an extra time. */
                    result *= base;
                }

                exponent /= 2;
                if (exponent <= 0) {
                    return result;
                }

                /* Halve the exponent, so square the base to be used for the next odd exponent. */
                base *= base;
            }
        }
    };

    constexpr inline auto pow = _pow_fn{};

    struct _abs_fn {
        template<advent::arithmetic T>
        [[nodiscard]]
        constexpr auto operator ()(const T num) const noexcept {
            if constexpr (advent::unsigned_type<T>) {
                return num;
            } else {
                if (num < 0) {
                    return -num;
                }

                return num;
            }
        }
    };

    constexpr inline auto abs = _abs_fn{};

}
