import std;
import advent;

struct Operation {
    using Variant = std::variant<std::plus<>, std::multiplies<>>;

    Variant op;

    constexpr Operation(const char symbol)
    :
        op([&]() -> Variant {
            if (symbol == '+') {
                return std::plus{};
            }

            [[assume(symbol == '*')]];

            return std::multiplies{};
        }())
    {}

    constexpr std::size_t identity_value(this const Operation self) {
        return self.op.visit(advent::overloaded{
            [](const std::plus<>) {
                return 0uz;
            },

            [](const std::multiplies<>) {
                return 1uz;
            }
        });
    }

    constexpr std::size_t operator ()(this const Operation self, const std::size_t left, const std::size_t right) {
        return self.op.visit([&](const auto op) {
            return op(left, right);
        });
    }
};

struct MathProblem {
    advent::grid_view<const char> digits;
    Operation op;

    template<typename DigitsGetter>
    requires (std::invocable<const DigitsGetter &, const advent::grid_view<const char> &>)
    constexpr std::size_t solve(this const MathProblem &self, const DigitsGetter digits_getter) {
        std::size_t result = self.op.identity_value();

        for (const auto digit_chars : std::invoke(digits_getter, self.digits)) {
            std::size_t operand = 0;

            for (const char digit_char : digit_chars) {
                if (digit_char == ' ') {
                    continue;
                }

                operand *= 10uz;
                operand += advent::digit_from_char(digit_char);
            }

            result = self.op(result, operand);
        }

        return result;
    }

    constexpr std::size_t solve_row_numbers(this const MathProblem &self) {
        return self.solve([](const auto &grid) {
            return grid.rows();
        });
    }

    constexpr std::size_t solve_column_numbers(this const MathProblem &self) {
        return self.solve([](const auto &grid) {
            return grid.columns();
        });
    }
};

constexpr std::string_view::const_iterator retrieve_symbols_start(const std::string_view data) {
    [[assume(data.size() > 1)]];
    [[assume(data.back() == '\n')]];

    const auto last_line_start = data.find_last_of('\n', data.size() - 2);
    [[assume(last_line_start != std::string_view::npos)]];
    [[assume(last_line_start != data.size() - 1)]];

    return data.begin() + last_line_start + 1;
}

template<typename Consumer>
requires (std::invocable<Consumer &, MathProblem>)
constexpr void for_each_problem(const std::string_view data, Consumer &&consumer) {
    const auto symbols_start_it = retrieve_symbols_start(data);

    const auto line_length = data.end() - symbols_start_it;

    auto current_symbol_it = symbols_start_it;
    while (true) {
        auto next_symbol_it = std::ranges::find_if(current_symbol_it + 1, data.end(), [](const char c) {
            return c != ' ';
        });

        [[assume(next_symbol_it != data.end())]];

        if (*next_symbol_it == '\n') {
            /*
                We reached the end of the line, now we move over
                one more character to make calculations consistent.
            */
            ++next_symbol_it;
        }

        const auto displacement = current_symbol_it - symbols_start_it;

        std::invoke(consumer, MathProblem{
            advent::grid_view(
                std::span(data.begin() + displacement, current_symbol_it),

                next_symbol_it - current_symbol_it - 1,

                line_length
            ),

            Operation(*current_symbol_it)
        });

        if (next_symbol_it == data.end()) {
            return;
        }

        current_symbol_it = next_symbol_it;
    }
}

template<auto SolveMethod>
constexpr std::size_t sum_solutions(const std::string_view data) {
    std::size_t result = 0;

    for_each_problem(data, [&](const auto &problem) {
        result += std::invoke(SolveMethod, problem);
    });

    return result;
}

constexpr inline std::string_view example_data = (
    "123 328  51 64 \n"
    " 45 64  387 23 \n"
    "  6 98  215 314\n"
    "*   +   *   +  \n"
);

static_assert(sum_solutions<&MathProblem::solve_row_numbers>(example_data)    == 4277556);
static_assert(sum_solutions<&MathProblem::solve_column_numbers>(example_data) == 3263827);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_solutions<&MathProblem::solve_row_numbers>,
        sum_solutions<&MathProblem::solve_column_numbers>
    );
}
