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

struct RowNumberProblems {
    struct Problem {
        Operation   op;
        std::size_t progress;

        constexpr Problem(const char op_symbol)
        :
            op(op_symbol),
            progress(op.identity_value())
        {}

        constexpr void feed_operand(this Problem &self, const std::size_t operand) {
            self.progress = self.op(self.progress, operand);
        }

        constexpr std::size_t result(this const Problem self) {
            return self.progress;
        }
    };

    std::vector<Problem> problems;

    constexpr RowNumberProblems(std::string_view ops_description) {
        while (true) {
            const auto symbol_pos = ops_description.find_first_not_of(' ');
            if (symbol_pos == std::string_view::npos) {
                return;
            }

            this->problems.emplace_back(ops_description[symbol_pos]);

            ops_description.remove_prefix(symbol_pos + 1);
        }
    }

    constexpr void feed_line(this RowNumberProblems &self, std::string_view line) {
        for (auto &problem : self.problems) {
            const auto operand_start = line.find_first_not_of(' ');
            [[assume(operand_start != std::string_view::npos)]];

            const auto operand_end = line.find_first_of(' ', operand_start);

            const auto operand = advent::to_integral<std::size_t>(line.substr(operand_start, operand_end - operand_start));

            problem.feed_operand(operand);

            if (operand_end != std::string_view::npos) {
                line.remove_prefix(operand_end);
            }
        }
    }

    constexpr std::size_t sum_solutions(this const RowNumberProblems &self) {
        std::size_t sum = 0;

        for (const auto &problem : self.problems) {
            sum += problem.result();
        }

        return sum;
    }
};

struct ColumnNumberProblems {
    struct Problem {
        std::vector<std::size_t> progress;

        std::size_t digits_start;
        std::size_t num_digits;

        Operation op;

        constexpr Problem(const char op_symbol, std::size_t digits_start, std::size_t num_digits)
        :
            progress(num_digits, 0uz),
            digits_start(digits_start),
            num_digits(num_digits),
            op(op_symbol)
        {}

        constexpr void accumulate_from_line(this Problem &self, const std::string_view line) {
            const auto digits = std::string_view(line.begin() + self.digits_start, self.num_digits);

            for (const auto [progress, digit_char] : std::views::zip(self.progress, digits)) {
                if (digit_char == ' ') {
                    continue;
                }

                progress *= 10uz;
                progress += advent::digit_from_char(digit_char);
            }
        }

        constexpr std::size_t result(this const Problem &self) {
            /*
                NOTE: There does not appear to be a
                speed improvement when first unwrapping
                'self.op' so that the operation does not
                have a branch each time it's called.
            */
            return std::ranges::fold_left(self.progress, self.op.identity_value(), self.op);
        }
    };

    std::vector<Problem> problems;

    constexpr ColumnNumberProblems(const std::string_view ops_description) {
        std::size_t current_symbol_pos = 0;
        while (true) {
            const char symbol = ops_description[current_symbol_pos];

            const auto next_symbol_pos = ops_description.find_first_not_of(' ', current_symbol_pos + 1);
            if (next_symbol_pos == std::string_view::npos) {
                this->problems.emplace_back(symbol, current_symbol_pos, ops_description.size() - current_symbol_pos);

                return;
            }

            this->problems.emplace_back(symbol, current_symbol_pos, next_symbol_pos - current_symbol_pos - 1);

            current_symbol_pos = next_symbol_pos;
        }
    }

    constexpr void feed_line(this ColumnNumberProblems &self, const std::string_view line) {
        for (auto &problem : self.problems) {
            problem.accumulate_from_line(line);
        }
    }

    constexpr std::size_t sum_solutions(this const ColumnNumberProblems &self) {
        std::size_t sum = 0;

        for (const auto &problem : self.problems) {
            sum += problem.result();
        }

        return sum;
    }
};

template<typename Problems>
constexpr Problems parse_and_remove_symbols(std::string_view &data) {
    [[assume(data.back() == '\n')]];
    data.remove_suffix(1);

    const auto symbols_start = data.find_last_of('\n');
    [[assume(symbols_start != std::string_view::npos)]];

    const auto symbols = data.substr(symbols_start + 1);

    data.remove_suffix(data.size() - symbols_start);

    return Problems(symbols);
}

template<typename Problems>
constexpr std::size_t sum_solutions(std::string_view data) {
    auto problems = parse_and_remove_symbols<Problems>(data);

    /*
        NOTE: Part one is faster by about 20 microseconds
        to split forwards rather than in reverse.
    */
    advent::split_for_each(data, '\n', [&](const auto line) {
        problems.feed_line(line);
    });

    return problems.sum_solutions();
}

constexpr inline std::string_view example_data = (
    "123 328  51 64 \n"
    " 45 64  387 23 \n"
    "  6 98  215 314\n"
    "*   +   *   +  \n"
);

static_assert(sum_solutions<RowNumberProblems>(example_data)    == 4277556);
static_assert(sum_solutions<ColumnNumberProblems>(example_data) == 3263827);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_solutions<RowNumberProblems>,
        sum_solutions<ColumnNumberProblems>
    );
}
