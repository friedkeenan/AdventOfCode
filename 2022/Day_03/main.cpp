#include <advent/advent.hpp>

class Rucksack {
    public:
        static constexpr std::size_t ItemTypePriority(const char item_type) {
            if (item_type >= 'a' && item_type <= 'z') {
                return (item_type - 'a') + 1;
            }

            return (item_type - 'A') + 1 + 26;
        }

        std::string_view items;

        constexpr std::string_view first_compartment() const {
            return this->items.substr(0, this->items.length() / 2);
        }

        constexpr std::string_view second_compartment() const {
            return this->items.substr(this->items.length() / 2);
        }

        constexpr char illicit_item_type() const {
            const auto first_compartment  = this->first_compartment();
            const auto second_compartment = this->second_compartment();

            for (const char item_type : first_compartment) {
                if (second_compartment.contains(item_type)) {
                    return item_type;
                }
            }

            /* There must be an illicit item type. */
            std::unreachable();
        }

        constexpr bool contains(const char item) const {
            return this->items.contains(item);
        }


        /* Add range facilities. */
        constexpr const char *data() const {
            return this->items.data();
        }

        constexpr std::string_view::const_iterator begin() const {
            return this->items.begin();
        }

        constexpr std::string_view::const_iterator end() const {
            return this->items.end();
        }

        constexpr std::size_t size() const {
            return this->items.size();
        }
};

static_assert(std::ranges::contiguous_range<Rucksack>);

class ElfGroup {
    public:
        std::array<Rucksack, 3> rucksacks;

        constexpr char derive_badge_item() const {
            for (const char item : this->rucksacks[0]) {
                if (this->rucksacks[1].contains(item) && this->rucksacks[2].contains(item)) {
                    return item;
                }
            }

            /* There must be a badge item. */
            std::unreachable();
        }
};

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t illicit_item_type_priority_sum(Rng &&rucksacks) {
    std::size_t priority_sum = 0;

    for (const std::string_view encoding : std::forward<Rng>(rucksacks)) {
        if (encoding.empty()) {
            continue;
        }

        const auto rucksack = Rucksack{encoding};
        const auto illicit  = rucksack.illicit_item_type();

        priority_sum += Rucksack::ItemTypePriority(illicit);
    }

    return priority_sum;
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t badge_item_type_priority_sum(Rng &&rucksacks) {
    std::size_t priority_sum = 0;

    /* NOTE: We could do this differently if I had an implementation of 'std::views::chunk'. */

    std::array<Rucksack, 3> group_rucksacks;
    std::uint8_t num_in_group = 0;
    for (const std::string_view encoding : std::forward<Rng>(rucksacks)) {
        if (encoding.empty()) {
            continue;
        }

        group_rucksacks[num_in_group] = Rucksack{encoding};

        if (num_in_group == 2) {
            num_in_group = 0;

            const auto group = ElfGroup{group_rucksacks};
            const auto badge = group.derive_badge_item();

            priority_sum += Rucksack::ItemTypePriority(badge);
        } else {
            ++num_in_group;
        }
    }

    return priority_sum;
}

constexpr std::size_t illicit_item_type_priority_sum_from_string_data(const std::string_view data) {
    return illicit_item_type_priority_sum(advent::views::split_lines(data));
}

constexpr std::size_t badge_item_type_priority_sum_from_string_data(const std::string_view data) {
    return badge_item_type_priority_sum(advent::views::split_lines(data));
}

constexpr inline std::string_view example_data = (
    "vJrwpWtwJgWrhcsFMMfFFhFp\n"
    "jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL\n"
    "PmmdzqPrVvPwwTWBwg\n"
    "wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn\n"
    "ttgJtRGJQctTZtZT\n"
    "CrZsJsPPZsGzwwsLwLmpwMDw\n"
);

static_assert(illicit_item_type_priority_sum_from_string_data(example_data) == 157);
static_assert(badge_item_type_priority_sum_from_string_data(example_data) == 70);

int main(int argc, char **argv) {
    const auto data = advent::puzzle_data(argc, argv);
    if (!data.has_value()) {
        fmt::print("Unable to read puzzle data!\n");

        return 1;
    }

    const auto part_one_solution = illicit_item_type_priority_sum_from_string_data(*data);
    const auto part_two_solution = badge_item_type_priority_sum_from_string_data(*data);

    fmt::print("Part one solution: {}\n", part_one_solution);
    fmt::print("Part two solution: {}\n", part_two_solution);
}
