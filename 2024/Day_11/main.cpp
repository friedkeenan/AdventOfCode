import std;
import advent;

struct StoneIterationResult {
    std::size_t original_stone;
    std::size_t iterations;
    std::size_t num_stones;
};

constexpr std::size_t tick_stone_and_count(std::vector<StoneIterationResult> &results, const std::size_t stone, const std::size_t iterations) {
    if (iterations == 0) {
        return 1;
    }

    const auto it = std::ranges::find_if(results, [&](auto result) {
        return result.original_stone == stone && result.iterations == iterations;
    });

    if (it != results.end()) {
        return it->num_stones;
    }

    const auto mark_and_return = [&](const std::size_t num_stones) {
        results.emplace_back(stone, iterations, num_stones);

        return num_stones;
    };

    if (stone <= 0) {
        return mark_and_return(tick_stone_and_count(results, 1, iterations - 1));
    }

    auto [num_digits, raised_base] = advent::count_digits_and_raise_base(stone, 10uz);

    if (num_digits % 2 == 0) {
        for (auto _ : std::views::iota(0uz, num_digits / 2)) {
            raised_base /= 10;
        }

        return mark_and_return(
            tick_stone_and_count(results, stone / raised_base, iterations - 1) +
            tick_stone_and_count(results, stone % raised_base, iterations - 1)
        );
    }

    return mark_and_return(tick_stone_and_count(results, 2024uz * stone, iterations - 1));
}

template<std::size_t NumIterations>
constexpr std::size_t count_stones_after_iterations(std::string_view data) {
    [[assume(!data.empty())]];
    [[assume(data.back() == '\n')]];

    data.remove_suffix(1);

    std::size_t num_stones = 0;

    /* NOTE: This would probably be faster with a hash map, but we need constexpr. */
    std::vector<StoneIterationResult> results;
    advent::split_with_callback(data, ' ', [&](const auto stone_repr) {
        /* NOTE: Stones do not affect each other so we can tick them individually. */

        num_stones += tick_stone_and_count(results, advent::to_integral<std::size_t>(stone_repr), NumIterations);
    });

    return num_stones;
}

constexpr inline std::string_view example_data = (
    "125 17\n"
);

static_assert(count_stones_after_iterations<25>(example_data) == 55312);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_stones_after_iterations<25>,
        count_stones_after_iterations<75>
    );
}
