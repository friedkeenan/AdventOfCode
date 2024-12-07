#include <advent/advent.hpp>

template<std::size_t Base = 10>
struct Concatenate {
    static constexpr std::size_t operator ()(std::size_t left, const std::size_t right) {
        /* Shift all the digits in 'left' over to make space to add 'right'. */
        for (auto _ : advent::views::reverse_digits_of(right, Base)) {
            left *= Base;
        }

        return left + right;
    }
};

template<bool IncludeConcatenation>
struct CalibrationRecord {
    std::size_t expected_result;
    std::vector<std::size_t> operands;

    constexpr explicit CalibrationRecord(const std::string_view description) {
        const auto result_end_pos = description.find_first_of(':');
        [[assume(result_end_pos != std::string_view::npos)]];

        this->expected_result = advent::to_integral<std::size_t>(description.substr(0, result_end_pos));

        /* NOTE: We move past the trailing space after the colon. */
        advent::split_with_callback(description.substr(result_end_pos + 1), ' ', [&](const auto operand) {
            this->operands.push_back(advent::to_integral<std::size_t>(operand));
        });

        [[assume(this->operands.size() > 0)]];
    }

    constexpr bool _is_possibly_correct(
        this const CalibrationRecord &self,
        std::size_t running_total,
        const std::size_t operand_index,
        const auto op
    ) {
        if (operand_index >= self.operands.size() - 1) {
            return op(running_total, self.operands.back()) == self.expected_result;
        }

        running_total = op(running_total, self.operands[operand_index]);

        /* NOTE: It appears slower to check this for any other operation. */
        if constexpr (std::same_as<decltype(op), const Concatenate<>>) {
            /* The running total can only increase from here. */
            if (running_total > self.expected_result) {
                return false;
            }
        }

        if (self._is_possibly_correct(running_total, operand_index + 1, std::plus{})) {
            return true;
        }

        const bool mult_result = self._is_possibly_correct(running_total, operand_index + 1, std::multiplies{});

        if constexpr (IncludeConcatenation) {
            if (mult_result) {
                return true;
            }

            return self._is_possibly_correct(running_total, operand_index + 1, Concatenate{});
        } else {
            return mult_result;
        }
    }

    constexpr bool is_possibly_correct(this const CalibrationRecord &self) {
        if (self._is_possibly_correct(0, 0, std::plus{})) {
            return true;
        }

        const auto mult_result = self._is_possibly_correct(0, 0, std::multiplies{});

        if constexpr (IncludeConcatenation) {
            if (mult_result) {
                return true;
            }

            return self._is_possibly_correct(0, 0, Concatenate{});
        } else {
            return mult_result;
        }
    }
};

template<bool IncludeConcatenation, advent::string_viewable_range Rng>
constexpr std::vector<CalibrationRecord<IncludeConcatenation>> parse_records(Rng &&rng) {
    std::vector<CalibrationRecord<IncludeConcatenation>> records;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        records.emplace_back(line);
    }

    return records;
}

template<bool IncludeConcatenation, advent::string_viewable_range Rng>
constexpr std::size_t sum_possibly_correct_calibration_results(Rng &&rng) {
    const auto records = parse_records<IncludeConcatenation>(std::forward<Rng>(rng));

    std::size_t sum = 0;
    for (const auto &record : records) {
        if (record.is_possibly_correct()) {
            sum += record.expected_result;
        }
    }

    return sum;
}

template<bool IncludeConcatenation>
constexpr std::size_t sum_possibly_correct_calibration_results_from_string_data(const std::string_view data) {
    return sum_possibly_correct_calibration_results<IncludeConcatenation>(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "190: 10 19\n"
    "3267: 81 40 27\n"
    "83: 17 5\n"
    "156: 15 6\n"
    "7290: 6 8 6 15\n"
    "161011: 16 10 13\n"
    "192: 17 8 14\n"
    "21037: 9 7 18 13\n"
    "292: 11 6 16 20\n"
);

static_assert(sum_possibly_correct_calibration_results_from_string_data<false>(example_data) == 3749);
static_assert(sum_possibly_correct_calibration_results_from_string_data<true>(example_data)  == 11387);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_possibly_correct_calibration_results_from_string_data<false>,
        sum_possibly_correct_calibration_results_from_string_data<true>
    );
}
