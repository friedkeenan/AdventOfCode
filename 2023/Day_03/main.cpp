#include <advent/advent.hpp>

struct Engine {
    std::vector<char> _schematic;
    std::size_t       _width = 0;

    template<std::ranges::input_range Rng>
    requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
    constexpr explicit Engine(Rng &&rng) {
        for (const std::string_view line : std::forward<Rng>(rng)) {
            if (line.empty()) {
                continue;
            }

            if (this->_width <= 0) {
                this->_width = line.size();
            }

            [[assume(line.size() == this->_width)]];

            std::ranges::copy(line, std::back_inserter(this->_schematic));
        }
    }

    constexpr std::size_t _distance_from_left(const std::size_t pos) const {
        return pos % this->_width;
    }

    constexpr bool _is_at_top(const std::size_t pos) const {
        return pos < this->_width;
    }

    constexpr bool _is_at_bottom(const std::size_t pos) const {
        return pos >= (this->_schematic.size() - this->_width);
    }

    constexpr bool _is_at_left(const std::size_t pos) const {
        return this->_distance_from_left(pos) == 0;
    }

    constexpr bool _is_at_right(const std::size_t pos) const {
        return this->_distance_from_left(pos + 1) == 0;
    }

    constexpr const char &_above(const std::size_t pos) const {
        [[assume(!this->_is_at_top(pos))]];

        return this->_schematic[pos - this->_width];
    }

    constexpr const char &_below(const std::size_t pos) const {
        [[assume(!this->_is_at_bottom(pos))]];

        return this->_schematic[pos + this->_width];
    }

    constexpr const char &_left_of(const std::size_t pos) const {
        [[assume(!this->_is_at_left(pos))]];

        return this->_schematic[pos - 1];
    }

    constexpr const char &_right_of(const std::size_t pos) const {
        [[assume(!this->_is_at_right(pos))]];

        return this->_schematic[pos + 1];
    }

    constexpr const char &_above_left_of(const std::size_t pos) const {
        [[assume(!this->_is_at_top(pos) && !this->_is_at_left(pos))]];

        return this->_schematic[pos - this->_width - 1];
    }

    constexpr const char &_below_left_of(const std::size_t pos) const {
        [[assume(!this->_is_at_bottom(pos) && !this->_is_at_left(pos))]];

        return this->_schematic[pos + this->_width - 1];
    }

    constexpr const char &_above_right_of(const std::size_t pos) const {
        [[assume(!this->_is_at_top(pos) && !this->_is_at_right(pos))]];

        return this->_schematic[pos - this->_width + 1];
    }

    constexpr const char &_below_right_of(const std::size_t pos) const {
        [[assume(!this->_is_at_bottom(pos) && !this->_is_at_right(pos))]];

        return this->_schematic[pos + this->_width + 1];
    }

    constexpr const char *_start_of_line(const std::size_t pos) const {
        [[assume(pos < this->_schematic.size())]];

        return this->_schematic.data() + (pos - this->_distance_from_left(pos));
    }

    constexpr std::string_view _line(const std::size_t pos) const {
        [[assume(pos < this->_schematic.size())]];

        return std::string_view(this->_start_of_line(pos), this->_width);
    }

    constexpr std::string_view _above_line(const std::size_t pos) const {
        [[assume(!this->_is_at_top(pos))]];

        const auto top_pos = pos - this->_width;

        return std::string_view(this->_start_of_line(top_pos), this->_width);
    }

    constexpr std::string_view _below_line(const std::size_t pos) const {
        [[assume(!this->_is_at_bottom(pos))]];

        const auto below_pos = pos + this->_width;

        return std::string_view(this->_start_of_line(below_pos), this->_width);
    }

    static constexpr bool IsSymbol(const char c) {
        static constexpr char BlankCharacter = '.';

        return c != BlankCharacter && !advent::is_digit(c, 10);
    }

    template<typename Callback>
    requires (std::invocable<Callback &, std::size_t>)
    constexpr void for_each_part_number(Callback callback) const {
        /*
            NOTE: I hate this.

            It seems that we could instead look for
            symbols instead of digits. I would be
            concerned that the same number could be
            attached to two different symbols but
            from a glance, it seems like that never
            happens.

            We could also maybe use 'find_first_of(Digits)'
            or something like that instead of looping over
            every character, but we would then have to
            switch up how we look for symbols and that
            seems so not worth it.
        */

        bool started_number = false;
        bool is_part_number = false;

        std::size_t number_start = 0;

        for (const auto pos : std::views::iota(0uz, this->_schematic.size())) {
            if (this->_is_at_left(pos) && started_number) {
                if (is_part_number) {
                    const auto number_repr = std::string_view(this->_schematic.data() + number_start, pos - number_start);

                    std::invoke(callback, advent::to_integral<std::size_t>(number_repr));
                }

                started_number = false;
                is_part_number = false;
            }

            const auto &elem = this->_schematic[pos];

            if (advent::is_digit(elem, 10)) {
                if (!started_number) {
                    started_number = true;

                    number_start = pos;

                    if (!this->_is_at_left(pos)) {
                        if (IsSymbol(this->_left_of(pos))) {
                            is_part_number = true;
                        } else if (!this->_is_at_top(pos) && IsSymbol(this->_above_left_of(pos))) {
                            is_part_number = true;
                        } else if (!this->_is_at_bottom(pos) && IsSymbol(this->_below_left_of(pos))) {
                            is_part_number = true;
                        }
                    }
                }

                if (!is_part_number) {
                    if (!this->_is_at_top(pos) && IsSymbol(this->_above(pos))) {
                        is_part_number = true;
                    } else if (!this->_is_at_bottom(pos) && IsSymbol(this->_below(pos))) {
                        is_part_number = true;
                    }
                }

                continue;
            }

            if (!started_number) {
                continue;
            }

            /* We've reached the character after a number. */

            if (!is_part_number) {
                if (IsSymbol(elem)) {
                    is_part_number = true;
                } else if (!this->_is_at_top(pos) && IsSymbol(this->_above(pos))) {
                    is_part_number = true;
                } else if (!this->_is_at_bottom(pos) && IsSymbol(this->_below(pos))) {
                    is_part_number = true;
                }
            }

            if (is_part_number) {
                const auto number_repr = std::string_view(this->_schematic.data() + number_start, pos - number_start);

                std::invoke(callback, advent::to_integral<std::size_t>(number_repr));
            }

            started_number = false;
            is_part_number = false;
        }

        /*
            We could have a number that is at the very end
            of the schematic, which our loop would not handle.
        */

        if (!started_number || !is_part_number) {
            return;
        }

        const auto number_repr = std::string_view(this->_schematic.data() + number_start, this->_schematic.size() - number_start);

        std::invoke(callback, advent::to_integral<std::size_t>(number_repr));
    }

    template<typename Callback>
    requires (std::invocable<Callback &, std::size_t>)
    constexpr void for_each_gear_ratio(Callback callback) const {
        /* NOTE: I also hate this a lot. */

        static constexpr std::string_view Digits = "0123456789";
        static constexpr char             Gear   = '*';

        struct NumberRepresentations {
            std::string_view first;
            std::string_view second;

            constexpr bool full() const {
                return !this->second.empty();
            }

            constexpr void push(const std::string_view repr) {
                [[assume(!repr.empty())]];
                [[assume(!this->full())]];

                if (this->first.empty()) {
                    this->first = repr;
                } else {
                    this->second = repr;
                }
            }

            constexpr std::size_t gear_ratio() const {
                [[assume(this->full())]];

                const auto first_number  = advent::to_integral<std::size_t>(this->first);
                const auto second_number = advent::to_integral<std::size_t>(this->second);

                return first_number * second_number;
            }
        };

        auto gear_it = this->_schematic.begin();

        while (true) {
            gear_it = std::ranges::find(gear_it, this->_schematic.end(), Gear);
            if (gear_it == this->_schematic.end()) {
                return;
            }

            const std::size_t gear_pos = gear_it - this->_schematic.begin();

            ++gear_it;

            auto representations = NumberRepresentations{};

            const auto handle_left_of_line = [&](const std::string_view line) {
                const auto number_region = line.substr(0, this->_distance_from_left(gear_pos));

                auto number_start = number_region.find_last_not_of(Digits);
                if (number_start == std::string_view::npos) {
                    number_start = 0;
                } else {
                    ++number_start;
                }

                representations.push(number_region.substr(number_start));
            };

            const auto handle_right_of_line = [&](const std::string_view line) {
                const auto number_region = line.substr(this->_distance_from_left(gear_pos) + 1);

                const auto number_end = number_region.find_first_not_of(Digits);

                representations.push(number_region.substr(0, number_end));
            };

            if (!this->_is_at_left(gear_pos) && advent::is_digit(this->_left_of(gear_pos), 10)) {
                handle_left_of_line(this->_line(gear_pos));
            }

            if (!this->_is_at_right(gear_pos) && advent::is_digit(this->_right_of(gear_pos), 10)) {
                handle_right_of_line(this->_line(gear_pos));
            }

            const auto handle_full_line = [&](const std::string_view number_region) {
                auto number_start = number_region.find_last_not_of(Digits, this->_distance_from_left(gear_pos));
                if (number_start == std::string_view::npos) {
                    number_start = 0;
                } else {
                    ++number_start;
                }

                auto number_end = number_region.find_first_not_of(Digits, this->_distance_from_left(gear_pos));
                if (number_end == std::string_view::npos) {
                    number_end = this->_width;
                }

                representations.push(number_region.substr(number_start, number_end - number_start));
            };

            if (!this->_is_at_top(gear_pos)) {
                /*
                    NOTE: If there was a digit directly above
                    us, then there cannot be a distinct number
                    to the top left or the top right of us.
                */
                if (advent::is_digit(this->_above(gear_pos), 10)) {
                    if (representations.full()) {
                        continue;
                    }

                    handle_full_line(this->_above_line(gear_pos));
                } else {
                    if (!this->_is_at_left(gear_pos) && advent::is_digit(this->_above_left_of(gear_pos), 10)) {
                        if (representations.full()) {
                            continue;
                        }

                        handle_left_of_line(this->_above_line(gear_pos));
                    }
                    if (!this->_is_at_right(gear_pos) && advent::is_digit(this->_above_right_of(gear_pos), 10)) {
                        if (representations.full()) {
                            continue;
                        }

                        handle_right_of_line(this->_above_line(gear_pos));
                    }
                }
            }

            if (!this->_is_at_bottom(gear_pos)) {
                /*
                    NOTE: If there was a digit directly below
                    us, then there cannot be a distinct number
                    to the bottom left or the bottom right of us.
                */
                if (advent::is_digit(this->_below(gear_pos), 10)) {
                    if (representations.full()) {
                        continue;
                    }

                    handle_full_line(this->_below_line(gear_pos));
                } else {
                    if (!this->_is_at_left(gear_pos) && advent::is_digit(this->_below_left_of(gear_pos), 10)) {
                        if (representations.full()) {
                            continue;
                        }

                        handle_left_of_line(this->_below_line(gear_pos));
                    }
                    if (!this->_is_at_right(gear_pos) && advent::is_digit(this->_below_right_of(gear_pos), 10)) {
                        if (representations.full()) {
                            continue;
                        }

                        handle_right_of_line(this->_below_line(gear_pos));
                    }
                }
            }

            if (representations.full()) {
                std::invoke(callback, representations.gear_ratio());
            }
        }
    }
};

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t sum_engine_part_numbers(Rng &&rng) {
    const auto engine = Engine(std::forward<Rng>(rng));

    std::size_t sum = 0;

    engine.for_each_part_number([&](const std::size_t part_number) {
        sum += part_number;
    });

    return sum;
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t sum_gear_ratios(Rng &&rng) {
    const auto engine = Engine(std::forward<Rng>(rng));

    std::size_t sum = 0;

    engine.for_each_gear_ratio([&](const std::size_t gear_ratio) {
        sum += gear_ratio;
    });

    return sum;
}

constexpr std::size_t sum_engine_part_numbers_from_string_data(const std::string_view data) {
    return sum_engine_part_numbers(data | advent::views::split_lines);
}

constexpr std::size_t sum_gear_ratios_from_string_data(const std::string_view data) {
    return sum_gear_ratios(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "467..114..\n"
    "...*......\n"
    "..35..633.\n"
    "......#...\n"
    "617*......\n"
    ".....+.58.\n"
    "..592.....\n"
    "......755.\n"
    "...$.*....\n"
    ".664.598..\n"
);

static_assert(sum_engine_part_numbers_from_string_data(example_data) == 4361);
static_assert(sum_gear_ratios_from_string_data(example_data) == 467835);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_engine_part_numbers_from_string_data,
        sum_gear_ratios_from_string_data
    );
}
