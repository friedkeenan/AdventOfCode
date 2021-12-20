#include <advent/advent.hpp>

template<std::ranges::input_range R>
requires (advent::arithmetic<std::ranges::range_value_t<R>>)
constexpr std::size_t count_depth_increases(R &&rng) {
    auto it = std::ranges::begin(rng);

    /* TODO: Use 'std::views::pairwise' when implemented. */
    auto prev_depth = *it;
    it++;

    std::size_t depth_increases = 0;
    for (const auto depth : std::ranges::subrange(it, std::ranges::end(rng))) {
        if (depth > prev_depth) {
            depth_increases++;
        }

        prev_depth = depth;
    }

    return depth_increases;
}

template<std::ranges::input_range R>
requires (advent::arithmetic<std::ranges::range_value_t<R>>)
constexpr std::size_t count_chunked_depth_increases(R &&rng) {
    /* TODO: Use 'std::views::adjacent' when implemented. */
    auto it = std::ranges::begin(rng);

    const auto first_depth = *it;
    it++;
    const auto second_depth = *it;
    it++;
    const auto third_depth = *it;
    it++;

    auto prev_depths = std::array{second_depth, third_depth};
    auto prev_sum    = first_depth + second_depth + third_depth;

    std::size_t depth_increases = 0;
    for (const auto depth : std::ranges::subrange(it, std::ranges::end(rng))) {
        const auto new_sum = prev_depths[0] + prev_depths[1] + depth;

        if (new_sum > prev_sum) {
            depth_increases++;
        }

        prev_depths[0] = prev_depths[1];
        prev_depths[1] = depth;

        prev_sum = new_sum;
    }

    return depth_increases;
}

constexpr inline auto parse_string_data = [](const std::string_view str) {
    return advent::views::split_lines(str) | std::views::transform(advent::to_integral<std::uint16_t>);
};

constexpr std::size_t count_depth_increases_from_string_data(const std::string_view data) {
    return count_depth_increases(parse_string_data(data));
}

constexpr std::size_t count_chunked_depth_increases_from_string_data(const std::string_view data) {
    return count_chunked_depth_increases(parse_string_data(data));
}

constexpr inline std::string_view example_data = (
    "199\n"
    "200\n"
    "208\n"
    "210\n"
    "200\n"
    "207\n"
    "240\n"
    "269\n"
    "260\n"
    "263\n"
);

static_assert(count_depth_increases_from_string_data(example_data) == 7);

static_assert(count_chunked_depth_increases_from_string_data(example_data) == 5);

int main(int argc, char **argv) {
    const auto data = advent::puzzle_data(argc, argv);
    if (!data.has_value()) {
        fmt::print("Unable to read puzzle data!\n");

        return 1;
    }

    const auto part_one_solution = count_depth_increases_from_string_data(*data);
    const auto part_two_solution = count_chunked_depth_increases_from_string_data(*data);

    fmt::print("Part one solution: {}\n", part_one_solution);
    fmt::print("Part two solution: {}\n", part_two_solution);
}
