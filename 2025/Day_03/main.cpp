import std;
import advent;

struct Bank {
    std::string_view batteries;

    template<std::size_t NumBatteries>
    constexpr std::size_t find_max_joltage(this const Bank self) {
        [[assume(self.batteries.size() >= NumBatteries)]];

        if constexpr (NumBatteries == 0) {
            return 0;
        } else if constexpr (NumBatteries == 1) {
            return advent::digit_from_char(*std::ranges::max_element(self.batteries));
        } else {
            /*
                The leftmost digit matters most, so search for
                the maximum that still leaves room for later digits.
            */

            std::size_t max_joltage = 0;

            auto search_start = self.batteries.begin();
            for (const auto place : std::views::iota(0uz, NumBatteries) | std::views::reverse) {
                search_start = std::ranges::max_element(search_start, self.batteries.end() - place);

                max_joltage *= 10uz;
                max_joltage += advent::digit_from_char(*search_start);

                /* Move one forward so we don't find this battery again. */
                ++search_start;
            }

            return max_joltage;
        }
    }
};

template<std::size_t NumBatteries, advent::string_viewable_range Rng>
constexpr std::size_t sum_bank_joltages(Rng &&rng) {
    std::size_t sum = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        const auto bank = Bank{line};

        sum += bank.find_max_joltage<NumBatteries>();
    }

    return sum;
}

template<std::size_t NumBatteries>
constexpr std::size_t sum_bank_joltages_from_string_data(const std::string_view data) {
    return sum_bank_joltages<NumBatteries>(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "987654321111111\n"
    "811111111111119\n"
    "234234234234278\n"
    "818181911112111\n"
);

static_assert(sum_bank_joltages_from_string_data<2>(example_data)  == 357);
static_assert(sum_bank_joltages_from_string_data<12>(example_data) == 3121910778619);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_bank_joltages_from_string_data<2>,
        sum_bank_joltages_from_string_data<12>
    );
}
