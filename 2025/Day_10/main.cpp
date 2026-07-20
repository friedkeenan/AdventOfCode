import std;
import advent;

struct Combinator {
    /*
        Combination finder adapted from https://stackoverflow.com/a/28698654.

        There's more efficient ways to do this, but they get much more complicated.
    */

    std::vector<bool> _used_indices;

    constexpr void reset(this Combinator &self, std::size_t num_active, std::size_t num_indices) {
        [[assume(num_active <= num_indices)]];

        self._used_indices.clear();
        self._used_indices.reserve(num_indices);

        /* NOTE: Last permutation. */
        self._used_indices.resize(num_active,  true);
        self._used_indices.resize(num_indices, false);
    }

    constexpr const std::vector<bool> &used_indices(this const Combinator &self) {
        return self._used_indices;
    }

    constexpr bool increment(this Combinator &self) {
        return std::ranges::prev_permutation(self._used_indices).found;
    }
};

struct StartingMachine {
    using Bitset = std::bitset<64>;

    Bitset              needed_lights;
    std::vector<Bitset> buttons;

    constexpr explicit StartingMachine(std::string_view description) {
        [[assume(description.front() == '[')]];

        description.remove_prefix(1);

        std::size_t pos = 0;
        while (true) {
            if (description.front() == ']') {
                break;
            }

            if (description.front() == '#') {
                this->needed_lights.set(pos);
            }

            pos += 1;
            description.remove_prefix(1);
        }

        while (true) {
            const auto start_pos = description.find_first_of('(');
            if (start_pos == std::string_view::npos) {
                break;
            }

            description.remove_prefix(start_pos + 1);

            const auto end_pos = description.find_first_of(')');
            [[assume(end_pos != std::string_view::npos)]];

            auto &button = this->buttons.emplace_back();
            advent::split_for_each(description.substr(0, end_pos), ',', [&](const auto index_str) {
                const auto index = advent::to_integral<std::size_t>(index_str);

                button.set(index);
            });

            description.remove_prefix(end_pos + 1);
        }
    }

    constexpr Bitset accumulate_buttons(this const StartingMachine &self, const Combinator &combinator) {
        Bitset result;

        for (const auto [is_used, button] : std::views::zip(combinator.used_indices(), self.buttons)) {
            if (!is_used) {
                continue;
            }

            result ^= button;
        }

        return result;
    }

    constexpr std::size_t min_needed_buttons(this const StartingMachine &self) {
        /*
            NOTE: XOR is commutative, and (a ^ a) == 0.

            This means that a button being pressed more than once is useless,
            and that a button being in a different position is useless.
        */

        if (self.needed_lights.none()) {
            return 0;
        }

        if (self.buttons.empty()) {
            return 0;
        }

        if (self.buttons.size() == 1) {
            return 1;
        }

        if (std::ranges::any_of(self.buttons, [&](const auto button) {
            return button == self.needed_lights;
        })) {
            return 1;
        }

        if (self.buttons.size() == 2) {
            return 2;
        }

        auto combinator = Combinator();

        for (const auto num_buttons : std::views::iota(2uz, self.buttons.size())) {
            combinator.reset(num_buttons, self.buttons.size());

            while (true) {
                const auto result = self.accumulate_buttons(combinator);

                if (result == self.needed_lights) {
                    return num_buttons;
                }

                if (!combinator.increment()) {
                    break;
                }
            }
        }

        return self.buttons.size();
    }
};

template<typename Machine, advent::string_viewable_range Rng>
constexpr std::size_t sum_minimum_button_presses(Rng &&rng) {
    std::size_t sum = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        const auto min = Machine(line).min_needed_buttons();

        sum += min;
    }

    return sum;
}

template<typename Machine>
constexpr std::size_t sum_minimum_button_presses_from_string_data(const std::string_view data) {
    return sum_minimum_button_presses<Machine>(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "[.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}\n"
    "[...#.] (0,2,3,4) (2,3) (0,4) (0,1,2) (1,2,3,4) {7,5,12,7,2}\n"
    "[.###.#] (0,1,2,3,4) (0,3,4) (0,1,2,4,5) (1,2) {10,11,11,5,10,5}\n"
);

static_assert(sum_minimum_button_presses_from_string_data<StartingMachine>(example_data) == 7);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_minimum_button_presses_from_string_data<StartingMachine>
    );
}
