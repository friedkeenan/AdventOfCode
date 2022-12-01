#include <advent/advent.hpp>

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t find_max_calories(Rng &&calorie_list) {
    std::size_t current_max_calories = 0;
    std::size_t current_inventory_calories = 0;

    for (const std::string_view item_calories : std::forward<Rng>(calorie_list)) {
        /* An empty string indicates the end of an inventory. */
        if (item_calories.empty()) {
            if (current_inventory_calories > current_max_calories) {
                current_max_calories = current_inventory_calories;
            }

            current_inventory_calories = 0;

            continue;
        }

        current_inventory_calories += advent::to_integral<std::size_t>(item_calories);
    }

    /*
        The last inventory does not end with an empty string,
        so we check again for a potential new maximum.
    */
    if (current_inventory_calories > current_max_calories) {
        current_max_calories = current_inventory_calories;
    }

    return current_max_calories;
}

/*
    NOTE: If we need this sort of thing elsewhere, we potentially
    may want to make it more generic and put it in the 'advent'
    library.
*/
template<typename Elem, std::size_t Size>
requires (Size > 0)
constexpr Elem &find_min_element(std::array<Elem, Size> &elements) {
    Elem *current_min = &elements[0];

    for (auto i = 1uz; i < Size; ++i) {
        if (elements[i] < *current_min) {
            current_min = &elements[i];
        }
    }

    return *current_min;
}

template<std::size_t NumMaximums, std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t find_sum_of_max_calories(Rng &&calorie_list) {
    std::array<std::size_t, NumMaximums> current_max_calories = {};

    std::size_t current_inventory_calories = 0;

    /* We store this as a lambda because we need to use it in two different places. */
    const auto potential_new_maximum_work = [&]() {
        /* We only need to replace the minimum of the current max calories. */

        auto &min_of_maxes = find_min_element(current_max_calories);
        if (current_inventory_calories > min_of_maxes) {
            min_of_maxes = current_inventory_calories;
        }
    };

    for (const std::string_view item_calories : std::forward<Rng>(calorie_list)) {
        /* An empty string indicates the end of an inventory. */
        if (item_calories.empty()) {
            potential_new_maximum_work();

            current_inventory_calories = 0;

            continue;
        }

        current_inventory_calories += advent::to_integral<std::size_t>(item_calories);
    }

    /*
        The last inventory does not end in a blank line,
        so we need to check if we have a new top contender.
    */
    potential_new_maximum_work();

    return std::accumulate(current_max_calories.begin(), current_max_calories.end(), 0uz);
}

constexpr std::size_t find_max_calories_from_string_data(const std::string_view data) {
    return find_max_calories(advent::views::split_lines(data));
}

template<std::size_t NumMaximums = 3>
constexpr std::size_t find_sum_of_max_calories_from_string_data(const std::string_view data) {
    return find_sum_of_max_calories<NumMaximums>(advent::views::split_lines(data));
}

constexpr inline std::string_view example_data = (
    /* Total 6000. */
    "1000\n"
    "2000\n"
    "3000\n"
    "\n"

    /* Total 4000. */
    "4000\n"
    "\n"

    /* Total 11000. */
    "5000\n"
    "6000\n"
    "\n"

    /* Total 24000. */
    "7000\n"
    "8000\n"
    "9000\n"
    "\n"

    /* Total 10000. */
    "10000"
);

static_assert(find_max_calories_from_string_data(example_data) == 24000);
static_assert(find_sum_of_max_calories_from_string_data(example_data) == 45000);

int main(int argc, char **argv) {
    const auto data = advent::puzzle_data(argc, argv);
    if (!data.has_value()) {
        fmt::print("Unable to read puzzle data!\n");

        return 1;
    }

    const auto part_one_solution = find_max_calories_from_string_data(*data);
    const auto part_two_solution = find_sum_of_max_calories_from_string_data(*data);

    fmt::print("Part one solution: {}\n", part_one_solution);
    fmt::print("Part two solution: {}\n", part_two_solution);
}
