#pragma once

#include <advent/common.hpp>
#include <advent/math.hpp>
#include <advent/basic_vector.hpp>
#include <advent/concepts.hpp>

namespace advent {

    /*
        TODO: Do we want it like this or like a normal function template definition?

        This can automatically be passed around as an invocable.
    */
    template<std::integral ToConvert>
    struct _to_integral_fn {
        template<std::ranges::input_range R>
        requires (
            !advent::array_of<R, char>                        &&
            std::same_as<std::ranges::range_value_t<R>, char>
        )
        [[nodiscard]]
        constexpr ToConvert operator ()(R &&str) const {
            auto it = std::ranges::begin(str);
            const auto sign = [&]() {
                if constexpr (std::unsigned_integral<ToConvert>) {
                    return ToConvert{1};
                } else {
                    if (*it == '-') {
                        it++;

                        return ToConvert{-1};
                    }

                    return ToConvert{1};
                }
            }();

            const auto base = [&]() -> ToConvert {
                if (*it == '0') {
                    it++;

                    if (*it == 'b') {
                        it++;

                        return 2;
                    }

                    if (*it == 'o') {
                        it++;

                        return 8;
                    }

                    if (*it == 'x') {
                        it++;

                        return 16;
                    }

                    /* Since the first digit was 0, we don't need to worry about it. */
                    return 10;
                }

                return 10;
            }();

            /*
                If we have a sized range, just start the place at the maximum and count down.

                Else if we have a bidirectional range, start at 0 and go through the range backwards and count up.

                Else, convert the range to a vector and proceed as a sized range.
            */
            auto rng = [&]() {
                /* Use a subrange as getting the sign and base may have incremented 'it'. */
                auto subrange = std::ranges::subrange(it, std::ranges::end(str));

                using Subrange = decltype(subrange);
                if constexpr (std::ranges::sized_range<Subrange>) {
                    return subrange;
                } else if constexpr (std::ranges::bidirectional_range<Subrange>) {
                    return std::move(subrange) | std::views::reverse;
                } else {
                    return advent::basic_vector(std::move(subrange));
                }
            }();

            auto digit_place = [&]() {
                if constexpr (std::ranges::sized_range<decltype(rng)>) {
                    return std::ranges::size(rng) - 1;
                } else {
                    return std::size_t{0};
                }
            }();

            ToConvert converted = 0;
            for (const auto digit : std::move(rng)) {
                if (base <= 10) {
                    advent::assume(digit >= '0' && digit < ('0' + base));
                } else {
                    advent::assume(
                        (digit >= '0' && digit <= '9') ||
                        (digit >= 'a' && digit <= 'f') ||
                        (digit >= 'A' && digit <= 'F')
                    );
                }

                if (digit >= '0' && digit <= '9') {
                    converted += static_cast<ToConvert>(advent::pow(base, digit_place) * (digit - '0' + 0));
                } else if (digit >= 'a' && digit <= 'f') {
                    converted += static_cast<ToConvert>(advent::pow(base, digit_place) * (digit - 'a' + 0xa));
                } else if (digit >= 'A' && digit <= 'F') {
                    converted += static_cast<ToConvert>(advent::pow(base, digit_place) * (digit - 'A' + 0xA));
                }

                if constexpr (std::ranges::sized_range<decltype(rng)>) {
                    digit_place--;
                } else {
                    digit_place++;
                }
            }

            return sign * converted;
        }

        template<std::size_t N>
        [[nodiscard]]
        constexpr ToConvert operator ()(const char (&str)[N]) const {
            return (*this)(std::string_view(str));
        }
    };

    template<std::integral ToConvert>
    constexpr inline _to_integral_fn<ToConvert> to_integral{};

    static_assert(advent::to_integral<std::uint16_t>("200") == 200);
    static_assert(advent::to_integral<std::int16_t>("-200") == -200);
    static_assert(advent::to_integral<std::uint16_t>("0x200") == 0x200);
    static_assert(advent::to_integral<std::int16_t>("-0x200") == -0x200);

}
