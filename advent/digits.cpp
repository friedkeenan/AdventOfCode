#include <advent/defines.hpp>

export module advent:digits;

import std;

import :concepts;
import :math;

namespace advent {

    export constexpr bool is_digit(char c, const std::integral auto base) {
        [[assume(base <= 10 || base == 16)]];

        if (base <= 10) {
            return c >= '0' && c < (static_cast<char>('0' + base));
        }

        return (
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F')
        );
    }

    export constexpr char digit_char(const std::integral auto digit, const std::integral auto base) {
        [[assume(base <= 10 || base == 16)]];

        [[assume(digit >= 0 && std::cmp_less(digit, base))]];

        if (digit < 10) {
            return static_cast<char>('0' + digit);
        }

        /* Base 16. */
        return static_cast<char>('A' + (digit - 10));
    }

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
        static constexpr ToConvert operator ()(R &&str) {
            auto it = std::ranges::begin(str);
            const auto sign = [&]() {
                if constexpr (std::unsigned_integral<ToConvert>) {
                    [[assume(*it != '-')]];

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
                    return std::vector(std::from_range, std::move(subrange));
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
                [[assume(advent::is_digit(digit, base))]];

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
        static constexpr ToConvert operator ()(const char (&str)[N]) {
            return _to_integral_fn::operator ()(std::string_view(str));
        }

        static constexpr ToConvert operator ()(const char digit) {
            [[assume(advent::is_digit(digit, 10))]];

            return static_cast<ToConvert>(digit - '0');
        }
    };

    export template<std::integral ToConvert, ToConvert Base = 0>
    constexpr inline _to_integral_fn<ToConvert, Base> to_integral{};

    static_assert(advent::to_integral<std::uint16_t>("200") == 200);
    static_assert(advent::to_integral<std::int16_t>("-200") == -200);
    static_assert(advent::to_integral<std::uint16_t>("0x200") == 0x200);
    static_assert(advent::to_integral<std::int16_t>("-0x200") == -0x200);

    static_assert(advent::to_integral<std::uint8_t, 2>("110")  == 6);
    static_assert(advent::to_integral<std::int8_t,  2>("-110") == -6);

    export template<std::integral Num, std::integral Base>
    constexpr Num count_digits(Num num, const Base base) {
        Num num_digits = 0;

        while (num > 0) {
            ++num_digits;

            num /= base;
        }

        return num_digits;
    }

    export template<std::integral Num, std::integral Base>
    constexpr auto count_digits_and_raise_base(Num num, const Base base) {
        struct result_info {
            Num  num_digits  = 0;
            Base raised_base = 1;
        };

        result_info result;

        while (num > 0) {
            ++result.num_digits;

            result.raised_base *= base;

            num /= base;
        }

        return result;
    }

    export template<std::integral Num, std::integral Base>
    struct reverse_digits_view : std::ranges::view_interface<reverse_digits_view<Num, Base>> {
        struct iterator {
            using iterator_concept = std::forward_iterator_tag;
            using value_type       = Num;
            using reference        = value_type;

            /* NOTE: We just nab the difference type from 'std::ranges::iota_view'. */
            using difference_type = std::ranges::range_difference_t<std::ranges::iota_view<Num>>;

            Num  _num;
            Base _base;

            constexpr reference operator *(this const iterator &self) {
                return self._num % self._base;
            }

            constexpr iterator &operator ++(this iterator &self) {
                self._num /= self._base;

                return self;
            }

            constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

            friend constexpr bool operator ==(const iterator &lhs, std::default_sentinel_t) {
                return lhs._num <= 0;
            }

            constexpr bool operator ==(const iterator &) const = default;
        };

        Num  _num;
        Base _base;

        constexpr reverse_digits_view(const Num num, const Base base) : _num(num), _base(base) {
            [[assume(num >= 0)]];
            [[assume(base > 1)]];
        }

        constexpr iterator begin(this const reverse_digits_view &self) {
            return iterator{self._num, self._base};
        }

        constexpr std::default_sentinel_t end(this const reverse_digits_view &) {
            return std::default_sentinel;
        }
    };

    static_assert(std::ranges::forward_range<advent::reverse_digits_view<std::size_t, std::size_t>>);

    export template<std::integral Num, std::integral Base>
    struct digits_view : std::ranges::view_interface<digits_view<Num, Base>> {
        /*
            NOTE: This is less efficient than 'reverse_digits_view'.

            It'd be nice if we could customize 'std::views::reverse'.
        */

        struct iterator {
            /* NOTE: This could be random access if our 'advent::pow' were constant time. */

            using iterator_concept = std::bidirectional_iterator_tag;
            using value_type       = Num;
            using reference        = value_type;

            /* NOTE: We just nab the difference type from 'std::ranges::iota_view'. */
            using difference_type = std::ranges::range_difference_t<std::ranges::iota_view<Base>>;

            Base _base;
            Base _raised_base;

            Num _original;

            constexpr reference operator *(this const iterator &self) {
                return self._original % self._raised_base;
            }

            constexpr iterator &operator ++(this iterator &self) {
                self._raised_base /= self._base;

                return self;
            }

            constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

            constexpr iterator &operator --(this iterator &self) {
                self._raised_base *= self._base;

                return self;
            }

            constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, --)

            friend constexpr bool operator ==(const iterator &lhs, std::default_sentinel_t) {
                return lhs._raised_base <= 1;
            }

            constexpr bool operator ==(const iterator &) const = default;
        };

        Base _base;
        Base _raised_base;
        Num  _num;

        constexpr digits_view(Num num, const Base base) : _num(num), _base(base) {
            [[assume(num >= 0)]];
            [[assume(base > 1)]];

            this->_raised_base = 1;
            while (num > 0) {
                this->_raised_base *= base;

                num /= base;
            }
        }

        constexpr iterator begin(this const digits_view &self) {
            return iterator{self._base, self._raised_base, self._num};
        }

        constexpr std::default_sentinel_t end(this const digits_view &) {
            return std::default_sentinel;
        }
    };

    static_assert(std::ranges::bidirectional_range<advent::digits_view<std::size_t, std::size_t>>);

    namespace views {

        namespace impl {

            struct reverse_digits_of_fn {
                static constexpr auto operator ()(const std::integral auto num, const std::integral auto base) {
                    return advent::reverse_digits_view(num, base);
                }
            };

            struct digits_of_fn {
                static constexpr auto operator ()(const std::integral auto num, const std::integral auto base) {
                    return advent::digits_view(num, base);
                }
            };

        }

        export constexpr inline auto reverse_digits_of = impl::reverse_digits_of_fn{};

        export constexpr inline auto digits_of = impl::digits_of_fn{};

    }

}

namespace std::ranges {

    template<typename Num, typename Base>
    constexpr inline bool enable_borrowed_range<advent::reverse_digits_view<Num, Base>> = true;

    template<typename Num, typename Base>
    constexpr inline bool enable_borrowed_range<advent::digits_view<Num, Base>> = true;

}
