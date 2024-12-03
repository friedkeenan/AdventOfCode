#include <advent/advent.hpp>

struct Multiplication {
    static constexpr std::string_view Prefix    = "mul(";
    static constexpr char             Suffix    = ')';
    static constexpr char             Separator = ',';

    static constexpr bool ValidArgumentCharacter(const char c) {
        return advent::is_digit(c, 10);
    }

    std::size_t left;
    std::size_t right;

    constexpr std::size_t result(this const Multiplication self) {
        return self.left * self.right;
    }
};

struct InstructionFinder {
    static constexpr std::string_view ToggleOff = "don't()";
    static constexpr std::string_view ToggleOn  = "do()";

    std::string_view memory;

    constexpr bool _is_exhausted(this const InstructionFinder self) {
        return self.memory.empty();
    }

    constexpr void _exhaust_all(this InstructionFinder &self) {
        self.memory = std::string_view();
    }

    constexpr void _exhaust(this InstructionFinder &self, const std::size_t num_characters) {
        self.memory = self.memory.substr(num_characters);
    }

    template<bool WithToggling>
    constexpr std::size_t _find_next_prefix(this InstructionFinder &self) {
        if constexpr (!WithToggling) {
            return self.memory.find(Multiplication::Prefix);
        } else {
            /*
                NOTE: We always leave the memory in such a way
                that being toggled on is its initial state.
            */

            while (true) {
                const auto next_prefix_pos = self.memory.find(Multiplication::Prefix);

                const auto toggle_off_pos = self.memory.rfind(ToggleOff, next_prefix_pos);
                if (toggle_off_pos == std::string_view::npos) {
                    /* No toggle off, proceed normally. */
                    return next_prefix_pos;
                }

                const auto toggle_on_pos = self.memory.find(ToggleOn, toggle_off_pos + ToggleOff.size());
                if (toggle_on_pos == std::string_view::npos) {
                    /* No future toggle on, any further instructions are bunk. */
                    return std::string_view::npos;
                }

                if (toggle_on_pos < next_prefix_pos) {
                    /* Toggle on after the toggle off and before the instruction, proceed normally. */
                    return next_prefix_pos;
                }

                /* Move past the toggle on, renew search. */
                self._exhaust(toggle_on_pos + ToggleOn.size());
            }
        }
    }

    constexpr std::optional<std::size_t> _parse_argument(this InstructionFinder &self, const char terminator) {
        for (const auto [i, character] : self.memory | std::views::enumerate) {
            if (character == terminator) {
                if (i <= 0) {
                    /* Move past the terminator. */
                    self._exhaust(i + 1);

                    return std::nullopt;
                }

                const auto value = advent::to_integral<std::size_t>(self.memory.substr(0, i));

                /* Move past the terminator. */
                self._exhaust(i + 1);

                return value;
            }

            if (!Multiplication::ValidArgumentCharacter(character)) {
                /* Move to the invalid character, which could be the start of an instruction. */
                self._exhaust(i);

                return std::nullopt;
            }
        }

        self._exhaust_all();

        return std::nullopt;
    }

    template<bool WithToggling>
    constexpr std::optional<Multiplication> _find_next(this InstructionFinder &self) {
        const auto next_prefix_pos = self._find_next_prefix<WithToggling>();
        if (next_prefix_pos == std::string_view::npos) {
            self._exhaust_all();

            return std::nullopt;
        }

        self.memory = self.memory.substr(next_prefix_pos + Multiplication::Prefix.size());

        const auto left = self._parse_argument(Multiplication::Separator);
        if (!left.has_value()) {
            return std::nullopt;
        }

        const auto right = self._parse_argument(Multiplication::Suffix);
        if (!right.has_value()) {
            return std::nullopt;
        }

        return Multiplication{left.value(), right.value()};
    }

    template<bool WithToggling>
    constexpr std::optional<Multiplication> find_next(this InstructionFinder &self) {
        while (!self._is_exhausted()) {
            const auto instruction = self._find_next<WithToggling>();

            if (instruction.has_value()) {
                return instruction.value();
            }
        }

        return std::nullopt;
    }
};

template<bool WithToggling>
constexpr std::size_t sum_multiplications(const std::string_view memory) {
    auto finder = InstructionFinder{memory};

    std::size_t sum = 0;
    while (true) {
        const auto instruction = finder.find_next<WithToggling>();
        if (!instruction.has_value()) {
            return sum;
        }

        sum += instruction->result();
    }
}

constexpr inline std::string_view example_data_without_toggling = (
    "xmul(2,4)%&mul[3,7]!@^do_not_mul(5,5)+mul(32,64]then(mul(11,8)mul(8,5))\n"
);

static_assert(sum_multiplications<false>(example_data_without_toggling) == 161);
static_assert(sum_multiplications<true>(example_data_without_toggling)  == 161);

constexpr inline std::string_view example_data_with_toggling = (
    "xmul(2,4)&mul[3,7]!^don't()_mul(5,5)+mul(32,64](mul(11,8)undo()?mul(8,5))\n"
);

static_assert(sum_multiplications<false>(example_data_with_toggling) == 161);
static_assert(sum_multiplications<true>(example_data_with_toggling)  == 48);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_multiplications<false>,
        sum_multiplications<true>
    );
}
