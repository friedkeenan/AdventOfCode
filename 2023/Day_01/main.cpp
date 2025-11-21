import std;
import advent;

template<typename Matcher>
concept DigitMatcher = advent::invocable_r<Matcher, std::optional<std::size_t>, const std::string_view &>;

constexpr std::size_t find_first_digit(std::string_view line, DigitMatcher auto &matcher) {
    while (true) {
        [[assume(!line.empty())]];

        const auto maybe_digit = std::invoke(matcher, std::as_const(line));
        if (maybe_digit.has_value()) {
            return *maybe_digit;
        }

        line.remove_prefix(1);
    }
}

constexpr std::size_t find_last_digit(std::string_view line, DigitMatcher auto &matcher) {
    while (true) {
        [[assume(!line.empty())]];

        const auto maybe_digit = std::invoke(matcher, std::as_const(line));
        if (maybe_digit.has_value()) {
            return *maybe_digit;
        }

        line.remove_suffix(1);
    }
}

template<advent::string_viewable_range Rng>
constexpr std::size_t find_sum_of_calibration_values(Rng &&rng, DigitMatcher auto first_matcher, DigitMatcher auto last_matcher) {
    std::size_t sum = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        const auto tens_digit = find_first_digit(line, first_matcher);
        const auto ones_digit = find_last_digit(line, last_matcher);

        /* NOTE: We already converted from characters to integrals. */
        const auto calibration_value = (
            10 * tens_digit +
            1  * ones_digit
        );

        sum += calibration_value;
    }

    return sum;
}

constexpr std::size_t find_sum_of_limited_calibration_values_from_string_data(const std::string_view data) {
    return find_sum_of_calibration_values(
        data | advent::views::split_lines,

        [](const std::string_view line) -> std::optional<std::size_t> {
            if (advent::is_digit(line.front(), 10)) {
                return line.front() - '0';
            }

            return std::nullopt;
        },

        [](const std::string_view line) -> std::optional<std::size_t> {
            if (advent::is_digit(line.back(), 10)) {
                return line.back() - '0';
            }

            return std::nullopt;
        }
    );
}

constexpr std::size_t find_sum_of_expanded_calibration_values_from_string_data(const std::string_view data) {
    static constexpr std::array<std::string_view, 9> textual_digits = {
        "one",
        "two",
        "three",
        "four",
        "five",
        "six",
        "seven",
        "eight",
        "nine",
    };

    return find_sum_of_calibration_values(
        data | advent::views::split_lines,

        [](const std::string_view line) -> std::optional<std::size_t> {
            if (advent::is_digit(line.front(), 10)) {
                return line.front() - '0';
            }

            for (const auto [i, textual_digit] : textual_digits | std::views::enumerate) {
                if (line.starts_with(textual_digit)) {
                    return i + 1;
                }
            }

            return std::nullopt;
        },

        [](const std::string_view line) -> std::optional<std::size_t> {
            if (advent::is_digit(line.back(), 10)) {
                return line.back() - '0';
            }

            for (const auto [i, textual_digit] : textual_digits | std::views::enumerate) {
                if (line.ends_with(textual_digit)) {
                    return i + 1;
                }
            }

            return std::nullopt;
        }
    );
}

constexpr inline std::string_view limited_example_data = (
    "1abc2\n"
    "pqr3stu8vwx\n"
    "a1b2c3d4e5f\n"
    "treb7uchet\n"
);

static_assert(find_sum_of_limited_calibration_values_from_string_data(limited_example_data) == 142);

constexpr inline std::string_view expanded_example_data = (
    "two1nine\n"
    "eightwothree\n"
    "abcone2threexyz\n"
    "xtwone3four\n"
    "4nineeightseven2\n"
    "zoneight234\n"
    "7pqrstsixteen\n"
);

static_assert(find_sum_of_expanded_calibration_values_from_string_data(expanded_example_data) == 281);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        find_sum_of_limited_calibration_values_from_string_data,
        find_sum_of_expanded_calibration_values_from_string_data
    );
}
