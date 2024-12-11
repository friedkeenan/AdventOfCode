#include <advent/advent.hpp>

struct WorryIncreaser {
    static constexpr std::string_view Prefix = "  Operation: new = old ";

    struct _old_tag { };

    using Operand   = std::variant<_old_tag, std::size_t>;
    using Operation = std::variant<std::plus<>, std::multiplies<>>;

    Operation _op;
    Operand   _right_operand;

    constexpr WorryIncreaser(std::string_view descriptor) {
        descriptor.remove_prefix(Prefix.length());

        this->_op = [&]() {
            switch (descriptor[0]) {
                case '+': return Operation{std::plus{}};
                case '*': return Operation{std::multiplies{}};

                default: std::unreachable();
            }
        }();

        /* Remove operation and following space. */
        descriptor.remove_prefix(1 + 1);

        this->_right_operand = [&]() {
            if (descriptor == "old") {
                return Operand{_old_tag{}};
            }

            return Operand{advent::to_integral<std::size_t>(descriptor)};
        }();
    }

    constexpr std::size_t operator ()(const std::size_t old) const {
        return std::visit([&](auto op, auto right_operand) {
            if constexpr (std::same_as<decltype(right_operand), _old_tag>) {
                return op(old, old);
            } else {
                return op(old, right_operand);
            }
        }, this->_op, this->_right_operand);
    }
};

static_assert(WorryIncreaser("  Operation: new = old * old")(5) == 25);
static_assert(WorryIncreaser("  Operation: new = old * 100")(5) == 500);
static_assert(WorryIncreaser("  Operation: new = old + old")(5) == 10);
static_assert(WorryIncreaser("  Operation: new = old + 100")(5) == 105);

/* Forward declare. */
struct MonkeyGroup;

struct WorryTest {
    static constexpr std::string_view DivisorPrefix = "  Test: divisible by ";
    static constexpr std::string_view SuccessPrefix = "    If true: throw to monkey ";
    static constexpr std::string_view FailurePrefix = "    If false: throw to monkey ";

    std::size_t _divisor;
    std::size_t _success_index;
    std::size_t _failure_index;

    template<advent::string_viewable_iterator It>
    static constexpr WorryTest ParseFromAndAdvanceIterator(It &it) {
        std::string_view line = *it;
        line.remove_prefix(DivisorPrefix.length());
        const auto divisor = advent::to_integral<std::size_t>(line);

        ++it;
        line = *it;
        line.remove_prefix(SuccessPrefix.length());
        const auto success_index = advent::to_integral<std::size_t>(line);

        ++it;
        line = *it;
        line.remove_prefix(FailurePrefix.length());
        const auto failure_index = advent::to_integral<std::size_t>(line);

        /* Advance past the failure line. */
        ++it;

        return WorryTest{divisor, success_index, failure_index};
    }

    /* We avoid directly naming 'MonkeyGroup' since it is still incomplete at this point. */
    constexpr auto &test(std::same_as<MonkeyGroup> auto &group, const std::size_t worry) const {
        if (worry % this->_divisor == 0) {
            return group[this->_success_index];
        }

        return group[this->_failure_index];
    }
};

struct Monkey {
    static constexpr std::string_view ItemsPrefix = "  Starting items:";

    std::vector<std::size_t> _item_worries;
    WorryIncreaser           _worry_increaser;
    WorryTest                _tester;

    template<advent::string_viewable_iterator It>
    static constexpr Monkey ParseFromAndAdvanceIterator(It &it) {
        /* Advance past the "Monkey 0:" lines. */
        ++it;

        auto starting_line = *it;
        starting_line.remove_prefix(ItemsPrefix.length());

        std::vector<std::size_t> item_worries;
        advent::split_with_callback(starting_line, ',', [&](auto worry_str) {
            /* Remove leading space. */
            worry_str.remove_prefix(1);

            item_worries.push_back(advent::to_integral<std::size_t>(worry_str));
        });

        ++it;
        const auto worry_increaser = WorryIncreaser(*it);

        ++it;
        const auto tester = WorryTest::ParseFromAndAdvanceIterator(it);

        /* Advance past the separating newline. */
        ++it;

        return Monkey{item_worries, worry_increaser, tester};
    }

    constexpr std::size_t divisor() const {
        return this->_tester._divisor;
    }

    constexpr void add_item(const std::size_t worry) {
        this->_item_worries.push_back(worry);
    }

    constexpr std::size_t num_items() const {
        return this->_item_worries.size();
    }

    /* We avoid directly naming 'MonkeyGroup' since it is still incomplete at this point. */
    constexpr void throw_items(std::same_as<MonkeyGroup> auto &group, const bool decrease_worry_for_boredom) {
        for (const auto item_worry : this->_item_worries) {
            const auto new_worry = [&]() {
                if (decrease_worry_for_boredom) {
                    /* Increase worry and become bored in the same line. */
                    return this->_worry_increaser(item_worry) / 3;
                } else {
                    return this->_worry_increaser(item_worry);
                }
            }();

            auto &new_monkey = this->_tester.test(group, new_worry);

            if (decrease_worry_for_boredom) {
                /* It'd be nice to use the same logic for this, but I don't want to add that. */
                new_monkey.add_item(new_worry);
            } else {
                new_monkey.add_item(new_worry % group.lowest_common_divisor);
            }
        }

        this->_item_worries.clear();
    }
};

struct MonkeyGroup {
    std::vector<Monkey> _monkeys;
    std::size_t lowest_common_divisor;

    template<advent::string_viewable_range Rng>
    constexpr MonkeyGroup(Rng &&monkeys) {
        auto it = std::ranges::begin(monkeys);

        this->lowest_common_divisor = 1;
        while (it != std::ranges::end(monkeys)) {
            auto monkey = Monkey::ParseFromAndAdvanceIterator(it);

            this->lowest_common_divisor = advent::lcm(this->lowest_common_divisor, monkey.divisor());

            this->_monkeys.push_back(std::move(monkey));
        }
    }

    constexpr std::size_t num_monkeys() const {
        return this->_monkeys.size();
    }

    constexpr Monkey &operator [](const std::size_t index) {
        return this->_monkeys[index];
    }

    constexpr const Monkey &operator [](const std::size_t index) const {
        return this->_monkeys[index];
    }
};

template<std::size_t NumRounds, advent::string_viewable_range Rng>
constexpr std::size_t find_monkey_business(const bool decrease_worry_for_boredom, Rng &&monkeys) {
    auto group = MonkeyGroup(std::forward<Rng>(monkeys));

    auto num_inspected_items = std::vector<std::size_t>(group.num_monkeys(), 0);
    for (auto _ : std::views::iota(0uz, NumRounds)) {
        for (const auto i : std::views::iota(0uz, group.num_monkeys())) {
            auto &monkey = group[i];

            num_inspected_items[i] += monkey.num_items();

            monkey.throw_items(group, decrease_worry_for_boredom);
        }
    }

    const auto maxes = advent::find_maxes<2>(num_inspected_items);

    return maxes[0] * maxes[1];
}

constexpr std::size_t find_monkey_business_with_worry_decrease_from_string_data(const std::string_view data) {
    return find_monkey_business<20>(true, data | advent::views::split_lines);
}

constexpr std::size_t find_monkey_business_without_worry_decrease_from_string_data(const std::string_view data) {
    return find_monkey_business<10'000>(false, data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "Monkey 0:\n"
    "  Starting items: 79, 98\n"
    "  Operation: new = old * 19\n"
    "  Test: divisible by 23\n"
    "    If true: throw to monkey 2\n"
    "    If false: throw to monkey 3\n"
    "\n"
    "Monkey 1:\n"
    "  Starting items: 54, 65, 75, 74\n"
    "  Operation: new = old + 6\n"
    "  Test: divisible by 19\n"
    "    If true: throw to monkey 2\n"
    "    If false: throw to monkey 0\n"
    "\n"
    "Monkey 2:\n"
    "  Starting items: 79, 60, 97\n"
    "  Operation: new = old * old\n"
    "  Test: divisible by 13\n"
    "    If true: throw to monkey 1\n"
    "    If false: throw to monkey 3\n"
    "\n"
    "Monkey 3:\n"
    "  Starting items: 74\n"
    "  Operation: new = old + 3\n"
    "  Test: divisible by 17\n"
    "    If true: throw to monkey 0\n"
    "    If false: throw to monkey 1\n"
);

static_assert(find_monkey_business_with_worry_decrease_from_string_data(example_data) == 10605);
static_assert(find_monkey_business_without_worry_decrease_from_string_data(example_data) == 2713310158);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        find_monkey_business_with_worry_decrease_from_string_data,
        find_monkey_business_without_worry_decrease_from_string_data
    );
}
