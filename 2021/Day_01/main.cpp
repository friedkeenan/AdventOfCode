#include <advent/advent.hpp>

template<std::ranges::input_range R>
requires (advent::arithmetic<std::ranges::range_value_t<R>>)
constexpr std::size_t count_depth_increases(R &&rng) {
    std::size_t depth_increases = 0;

    for (const auto &[prev_depth, depth] : std::forward<R>(rng) | std::views::pairwise) {
        if (depth > prev_depth) {
            ++depth_increases;
        }
    }

    return depth_increases;
}

template<std::ranges::input_range R>
requires (advent::arithmetic<std::ranges::range_value_t<R>>)
constexpr std::size_t count_chunked_depth_increases(R &&rng) {
    std::size_t depth_increases = 0;

    advent::addition_result<std::ranges::range_reference_t<R>> prev_sum = 0;
    for (const auto [i, depths] : std::forward<R>(rng) | std::views::adjacent<3> | std::views::enumerate) {
        const auto new_sum = std::apply([](auto &... depths) {
            return (depths + ...);
        }, depths);

        /* NOTE: We can't increment 'depth_increases' on our first iteration. */
        if (new_sum > prev_sum && i > 0) {
            ++depth_increases;
        }

        prev_sum = new_sum;
    }

    return depth_increases;
}

constexpr auto parse_string_data(const std::string_view str) {
    return advent::views::split_lines(str) | std::views::transform(advent::to_integral<std::uint16_t>);
}

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
    return advent::solve_puzzles(
        argc, argv,

        count_depth_increases_from_string_data,
        count_chunked_depth_increases_from_string_data
    );
}
