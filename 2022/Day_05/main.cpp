#include <advent/advent.hpp>

class Crates {
    public:
        class InstructionInfo {
            public:
                std::size_t quantity;
                std::size_t from;
                std::size_t to;

                constexpr explicit InstructionInfo(std::string_view instruction) {
                    const auto parse_next = [&](const std::string_view label) {
                        instruction.remove_prefix(label.length() + 1);

                        const auto end_of_num = instruction.find_first_of(' ');
                        if (end_of_num == std::string_view::npos) {
                            return advent::to_integral<std::size_t>(instruction);
                        }

                        const auto num = advent::to_integral<std::size_t>(instruction.substr(0, end_of_num));
                        instruction.remove_prefix(end_of_num + 1);

                        return num;
                    };

                    this->quantity = parse_next("move");
                    this->from     = parse_next("from");
                    this->to       = parse_next("to");
                }
        };

        class Stack {
            public:
                using CrateIterator = std::vector<char>::const_iterator;

                std::vector<char> crates;

                constexpr char top() const {
                    return this->crates.back();
                }

                constexpr void add_to_top(const char crate) {
                    this->crates.push_back(crate);
                }

                constexpr void add_to_top(CrateIterator first, CrateIterator last) {
                    this->crates.insert(this->crates.end(), first, last);
                }

                constexpr void add_to_bottom(const char crate) {
                    this->crates.insert(this->crates.begin(), crate);
                }

                constexpr void move_top_to(Stack &other) {
                    other.add_to_top(this->top());

                    this->crates.pop_back();
                }

                constexpr void move_top_several_to(const std::size_t quantity, Stack &other) {
                    const auto first_it = this->crates.end() - quantity;

                    other.add_to_top(first_it, this->crates.end());
                    this->crates.erase(first_it, this->crates.end());
                }
        };

        static constexpr std::size_t NumStacksFromLength(const std::size_t length) {
            /* Each stack has 4 characters except for the last, which has 3. */

            return (length + 1) / 4;
        }

        template<std::input_iterator It>
        requires (std::convertible_to<std::iter_reference_t<It>, std::string_view>)
        static constexpr Crates ParseFromAndAdvanceIterator(It &it) {
            /* This variable is reused. */
            std::string_view row = *it;

            auto stacks = std::vector<Stack>(NumStacksFromLength(row.length()));

            while (true) {
                /* If we reached the stack labels. */
                if (row[1] == '1') {
                    ++it;

                    return Crates{stacks};
                }

                /* The crate letters are one after multiples of 4. */
                for (std::size_t i = 1; i < row.length(); i += 4) {
                    const auto crate = row[i];
                    if (crate == ' ') {
                        continue;
                    }

                    /* There *may* be more efficient ways of doing this but meh. */
                    stacks[i / 4].add_to_bottom(crate);
                }

                ++it;
                row = *it;
            }
        }

        std::vector<Stack> stacks;

        constexpr void move_individual_crates(const InstructionInfo &info) {
            for ([[maybe_unused]] const auto i : std::views::iota(0uz, info.quantity)) {
                this->stacks[info.from - 1].move_top_to(this->stacks[info.to - 1]);
            }
        }

        constexpr void move_several_crates(const InstructionInfo &info) {
            this->stacks[info.from - 1].move_top_several_to(info.quantity, this->stacks[info.to - 1]);
        }

        constexpr std::string top_crates() const {
            auto crates = std::string(this->stacks.size(), ' ');

            for (const auto i : std::views::iota(0uz, this->stacks.size())) {
                /* There must definitely be a crate in each stack. */
                crates[i] = this->stacks[i].top();
            }

            return crates;
        }
};

template<typename Executer>
concept InstructionExecuter = std::invocable<const Executer &, Crates &, const Crates::InstructionInfo &>;

template<InstructionExecuter Executer, std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::string top_crates_after_moving(const Executer executer, Rng &&crate_info) {
    auto it     = std::ranges::begin(crate_info);
    auto crates = Crates::ParseFromAndAdvanceIterator(it);

    /* Now we loop over the instructions. */
    for (; it != std::ranges::end(crate_info); ++it) {
        const std::string_view instruction = *it;

        if (instruction.empty()) {
            continue;
        }

        const auto info = Crates::InstructionInfo(instruction);
        std::invoke(executer, crates, info);
    }

    return crates.top_crates();
}

constexpr std::string top_crates_after_moving_individuals_from_string_data(const std::string_view data) {
    return top_crates_after_moving(&Crates::move_individual_crates, advent::views::split_lines(data));
}

constexpr std::string top_crates_after_moving_several_from_string_data(const std::string_view data) {
    return top_crates_after_moving(&Crates::move_several_crates, advent::views::split_lines(data));
}

constexpr inline std::string_view example_data = (
    "    [D]    \n"
    "[N] [C]    \n"
    "[Z] [M] [P]\n"
    " 1   2   3 \n"
    "\n"
    "move 1 from 2 to 1\n"
    "move 3 from 1 to 3\n"
    "move 2 from 2 to 1\n"
    "move 1 from 1 to 2\n"
);

static_assert(top_crates_after_moving_individuals_from_string_data(example_data) == "CMZ");
static_assert(top_crates_after_moving_several_from_string_data(example_data) == "MCD");

int main(int argc, char **argv) {
    const auto data = advent::puzzle_data(argc, argv);
    if (!data.has_value()) {
        fmt::print("Unable to read puzzle data!\n");

        return 1;
    }

    const auto part_one_solution = top_crates_after_moving_individuals_from_string_data(*data);
    const auto part_two_solution = top_crates_after_moving_several_from_string_data(*data);

    fmt::print("Part one solution: {}\n", part_one_solution);
    fmt::print("Part two solution: {}\n", part_two_solution);
}
