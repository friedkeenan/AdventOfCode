import std;
import advent;

struct Bank {
    std::string_view batteries;

    template<std::size_t NumBatteries>
    requires (NumBatteries > 1)
    constexpr auto joltage_contributions(this const Bank self) {
        struct JoltageContribution {
            std::array<std::size_t, NumBatteries> potential_contributions = {};
        };

        std::vector<JoltageContribution> contributions;
        contributions.reserve(self.batteries.size());

        for (const auto it : self.batteries | advent::views::iterators) {
            const auto battery_value = advent::digit_from_char(*it);

            const auto distance_from_end   = self.batteries.end() - it;
            const auto distance_from_begin = it - self.batteries.begin();

            const auto min_place = std::max(0z, advent::ssize_t{NumBatteries} - distance_from_begin - 1);
            const auto max_place = std::min(advent::ssize_t{NumBatteries} - 1, distance_from_end - 1);

            auto &contribution = contributions.emplace_back();
            for (const auto place : std::views::iota(min_place, max_place + 1)) {
                contribution.potential_contributions[place] = battery_value * advent::pow(10uz, place);
            }
        }

        return contributions;
    }

    template<std::size_t NumBatteries>
    requires (NumBatteries > 1)
    constexpr std::size_t find_max_joltage_impl(this const Bank self) {
        /* NOTE: This implementation works for N=2 as well, but it is slower. */

        const auto contributions = self.joltage_contributions<NumBatteries>();

        std::size_t max_joltage = 0;

        auto search_start = contributions.begin();
        for (const auto place : std::views::iota(0uz, NumBatteries) | std::views::reverse) {
            search_start = std::ranges::max_element(search_start, contributions.end(), {}, [&](const auto &contribution) {
                return contribution.potential_contributions[place];
            });

            max_joltage += search_start->potential_contributions[place];

            /* Move one forward so we don't find this contribution again. */
            ++search_start;
        }

        return max_joltage;
    }

    template<std::size_t NumBatteries>
    constexpr std::size_t find_max_joltage(this const Bank self) {
        if constexpr (NumBatteries == 0) {
            return 0;
        } else if constexpr (NumBatteries == 1) {
            return advent::digit_from_char(*std::ranges::max_element(self.batteries));
        } else if constexpr (NumBatteries == 2) {
            const auto max_battery_iter = std::ranges::max_element(self.batteries);

            const auto max_battery_value = advent::digit_from_char(*max_battery_iter);

            std::size_t max_joltage = 0;
            for (const auto it : self.batteries | advent::views::iterators) {
                if (it == max_battery_iter) {
                    continue;
                }

                const auto battery_value = advent::digit_from_char(*it);

                const auto joltage = [&]() {
                    /*
                        NOTE: Apparently using a dedicated function
                        to combine the battery values makes this slower
                        by 2 microseconds, even if I add 'always_inline'.
                    */

                    if (it < max_battery_iter) {
                        return battery_value * 10uz + max_battery_value;
                    }

                    return max_battery_value * 10uz + battery_value;
                }();

                if (joltage > max_joltage) {
                    max_joltage = joltage;
                }
            }

            return max_joltage;
        } else {
            return self.find_max_joltage_impl<NumBatteries>();
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
