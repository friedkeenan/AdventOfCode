#pragma once

#include <advent/common.hpp>
#include <advent/math.hpp>
#include <advent/concepts.hpp>

namespace advent {

    template<std::integral ToConvert, ToConvert Base>
    struct _to_integral_fn {
        template<std::ranges::input_range R>
        requires (
            /*
                We stop char arrays from using this overload so
                that they can be converted to a 'std::string_view'
                by another overload.
            */
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
                        ++it;

                        return ToConvert{-1};
                    }

                    return ToConvert{1};
                }
            }();

            const auto base = [&]() -> ToConvert {
                /* If the base is unspecified. */
                if constexpr (Base == 0) {
                    if (*it == '0') {
                        ++it;

                        if (*it == 'b') {
                            ++it;

                            return 2;
                        }

                        if (*it == 'o') {
                            ++it;

                            return 8;
                        }

                        if (*it == 'x') {
                            ++it;

                            return 16;
                        }

                        /* Since the first digit was 0, we don't need to worry about it. */
                        return 10;
                    }

                    return 10;
                } else {
                    return Base;
                }
            }();

            /* If our range is only an input range and is not sized, just stuff the range into a vector. */
            auto rng = [&]() {
                /* Use a subrange as getting the sign and base may have incremented 'it'. */
                auto subrange = std::ranges::subrange(it, std::ranges::end(str));

                using Subrange = decltype(subrange);
                if constexpr (!std::ranges::sized_range<Subrange> && !std::ranges::forward_range<Subrange>) {
                    auto common_subrange = subrange | std::views::common;
                    return std::vector(std::ranges::begin(common_subrange), std::ranges::end(common_subrange));
                } else {
                    return subrange;
                }
            }();

            auto digit_place = [&]() {
                /* If we have a sized range, use the size. Else use the distance from the start to end of the range. */
                if constexpr (std::ranges::sized_range<decltype(rng)>) {
                    return std::ranges::size(rng) - 1;
                } else {
                    return std::ranges::distance(std::ranges::begin(rng), std::ranges::end(rng)) - 1;
                }
            }();

            ToConvert converted = 0;
            for (const auto digit : std::move(rng)) {
                if (base <= 10) {
                    [[assume(digit >= '0' && digit < (static_cast<char>('0' + base)))]];
                } else {
                    [[assume(
                        (digit >= '0' && digit <= '9') ||
                        (digit >= 'a' && digit <= 'f') ||
                        (digit >= 'A' && digit <= 'F')
                    )]];
                }

                if (digit >= '0' && digit <= '9') {
                    converted += static_cast<ToConvert>(advent::pow(base, digit_place) * (digit - '0' + 0));
                } else if (digit >= 'a' && digit <= 'f') {
                    converted += static_cast<ToConvert>(advent::pow(base, digit_place) * (digit - 'a' + 0xa));
                } else if (digit >= 'A' && digit <= 'F') {
                    converted += static_cast<ToConvert>(advent::pow(base, digit_place) * (digit - 'A' + 0xA));
                }

                --digit_place;
            }

            return sign * converted;
        }

        template<std::size_t N>
        [[nodiscard]]
        constexpr ToConvert operator ()(const char (&str)[N]) const {
            return (*this)(std::string_view(str));
        }
    };

    template<std::integral ToConvert, ToConvert Base = 0>
    constexpr inline _to_integral_fn<ToConvert, Base> to_integral{};

    static_assert(advent::to_integral<std::uint16_t>("200") == 200);
    static_assert(advent::to_integral<std::int16_t>("-200") == -200);
    static_assert(advent::to_integral<std::uint16_t>("0x200") == 0x200);
    static_assert(advent::to_integral<std::int16_t>("-0x200") == -0x200);

    static_assert(advent::to_integral<std::uint8_t, 2>("110")  == 6);
    static_assert(advent::to_integral<std::int8_t,  2>("-110") == -6);

}
