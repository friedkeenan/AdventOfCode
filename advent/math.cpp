export module advent:math;

import std;

import :concepts;

namespace advent {

    struct _pow_fn {
        template<std::integral Base, std::integral Exponent>
        [[nodiscard]]
        static constexpr Base operator ()(Base base, Exponent exponent) noexcept {
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

    export constexpr inline auto pow = _pow_fn{};

    struct _floor_sqrt_fn {
        template<std::integral T>
        [[nodiscard]]
        static constexpr T operator ()(const T num) noexcept {
            [[assume(num >= 0)]];

            /* NOTE: We use binary search to find the floored square root. */

            T low  = 0;
            T high = num;

            while (true) {
                const auto midpoint = std::midpoint(low, high);

                const auto squared = midpoint * midpoint;
                if (squared == num) {
                    return midpoint;
                }

                if (squared < num) {
                    const auto next_squared = (midpoint + 1) * (midpoint + 1);

                    if (next_squared > num) {
                        return midpoint;
                    }

                    low = midpoint + 1;
                } else {
                    high = midpoint - 1;
                }
            }
        }
    };

    export constexpr inline auto floor_sqrt = _floor_sqrt_fn{};

    static_assert(advent::floor_sqrt(0)  == 0);
    static_assert(advent::floor_sqrt(1)  == 1);
    static_assert(advent::floor_sqrt(2)  == 1);
    static_assert(advent::floor_sqrt(25) == 5);
    static_assert(advent::floor_sqrt(30) == 5);

    struct _ceil_sqrt_fn {
        template<std::integral T>
        [[nodiscard]]
        static constexpr T operator ()(const T num) noexcept {
            [[assume(num >= 0)]];

            /* NOTE: We use binary search to find the ceiled square root. */

            T low  = 0;
            T high = num;

            while (true) {
                const auto midpoint = std::midpoint(low, high);

                const auto squared = midpoint * midpoint;
                if (squared == num) {
                    return midpoint;
                }

                if (squared < num) {
                    low = midpoint + 1;
                } else {
                    const auto last_squared = (midpoint - 1) * (midpoint - 1);

                    if (last_squared < num) {
                        return midpoint;
                    }

                    high = midpoint - 1;
                }
            }
        }
    };

    export constexpr inline auto ceil_sqrt = _ceil_sqrt_fn{};

    static_assert(advent::ceil_sqrt(0)  == 0);
    static_assert(advent::ceil_sqrt(1)  == 1);
    static_assert(advent::ceil_sqrt(2)  == 2);
    static_assert(advent::ceil_sqrt(25) == 5);
    static_assert(advent::ceil_sqrt(30) == 6);

    struct _abs_fn {
        template<advent::arithmetic T>
        [[nodiscard]]
        static constexpr auto operator ()(const T num) noexcept {
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

    export constexpr inline auto abs = _abs_fn{};

    static_assert(advent::abs(-1) == 1);
    static_assert(advent::abs(1)  == 1);
    static_assert(advent::abs(0)  == 0);

    static_assert(advent::abs(1u) == 1u);
    static_assert(advent::abs(0u) == 0u);

    struct _gcd_fn {
        template<std::integral A, std::integral B>
        [[nodiscard]]
        static constexpr auto operator ()(A a, B b) noexcept {
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

    export constexpr inline auto gcd = _gcd_fn{};

    static_assert(advent::gcd(12, 6) == 6);
    static_assert(advent::gcd(12, 9) == 3);

    static_assert(advent::gcd(0, 9) == 9);
    static_assert(advent::gcd(0, 0) == 0);

    struct _lcm_fn {
        template<std::integral A, std::integral B>
        [[nodiscard]]
        static constexpr auto operator ()(const A a, const B b) noexcept {
            const auto gcd = advent::gcd(a, b);

            using Common = decltype(gcd);

            if (gcd == 0) {
                return Common{};
            }

            return Common{(a / gcd) * b};
        }
    };

    export constexpr inline auto lcm = _lcm_fn{};

    static_assert(advent::lcm(12, 6) == 12);
    static_assert(advent::lcm(12, 9) == 36);

    static_assert(advent::lcm(0, 9) == 0);
    static_assert(advent::lcm(0, 0) == 0);

    export template<std::size_t NumMaxes, std::ranges::input_range Rng>
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
