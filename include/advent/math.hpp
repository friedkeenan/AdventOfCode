#pragma once

#include <advent/common.hpp>
#include <advent/concepts.hpp>

namespace advent {

    struct _pow_fn {
        template<std::integral Base, std::integral Exponent>
        [[nodiscard]]
        constexpr Base operator ()(Base base, Exponent exponent) const {
            /* This is a binary pow implementation. */

            [[assume(exponent >= 0)]];

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

    struct _gcd_fn {
        template<std::integral A, std::integral B>
        [[nodiscard]]
        constexpr auto operator ()(A a, B b) const {
            using Common = std::common_type_t<A, B>;

            while (true) {
                if (a == 0) {
                    return Common{b};
                }

                b %= a;

                if (b == 0) {
                    return Common{a};
                }

                a %= b;
            }
        }
    };

    constexpr inline auto gcd = _gcd_fn{};

    static_assert(advent::gcd(12, 6) == 6);
    static_assert(advent::gcd(12, 9) == 3);

    struct _lcm_fn {
        template<std::integral A, std::integral B>
        [[nodiscard]]
        constexpr auto operator ()(A a, B b) const {
            const auto gcd = advent::gcd(a, b);

            using Common = decltype(gcd);

            if (gcd == 0) {
                return Common{};
            }

            return Common{(a / gcd) * b};
        }
    };

    constexpr inline auto lcm = _lcm_fn{};

    static_assert(advent::lcm(12, 6) == 12);
    static_assert(advent::lcm(12, 9) == 36);

    template<std::size_t NumMaxes, std::ranges::input_range Rng>
    requires (advent::arithmetic<std::ranges::range_value_t<Rng>>)
    constexpr auto find_maxes(Rng &&rng) {
        using Elem = std::ranges::range_value_t<Rng>;

        std::array<Elem, NumMaxes> maxes = {};
        if constexpr (NumMaxes == 0) {
            return maxes;
        } else {
            const auto find_min_of_maxes = [&]() -> Elem & {
                Elem *current_min = &maxes[0];

                for (const auto i : std::views::iota(1uz, NumMaxes)) {
                    if (maxes[i] < *current_min) {
                        current_min = &maxes[i];
                    }
                }

                return *current_min;
            };

            for (const auto elem : std::forward<Rng>(rng)) {
                auto &min_of_maxes = find_min_of_maxes();

                if (elem > min_of_maxes) {
                    min_of_maxes = elem;
                }
            }

            return maxes;
        }
    }

}
