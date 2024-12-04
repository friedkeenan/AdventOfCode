#include <advent/advent.hpp>

template<advent::string_viewable_range Rng>
constexpr advent::grid<char> parse_grid(Rng &&rng) {
    return advent::grid<char>::build([&](auto &builder) {
        for (const std::string_view line : std::forward<Rng>(rng)) {
            if (line.empty()) {
                continue;
            }

            builder.push_row(line.size(), [&](const auto row) {
                std::ranges::copy(line, row.begin());
            });
        }
    });
}

constexpr std::size_t count_xmas_branches(const advent::grid<char> &grid, const char *start) {
    [[assume(*start == 'X')]];

    std::size_t total = 0;
    for (const auto [direction, neighbor] : grid.neighbors_of(start)) {
        if (*neighbor != 'M') {
            continue;
        }

        if (!grid.has_neighbor(direction, neighbor)) {
            continue;
        }

        const auto possible_A = grid.neighbor(direction, neighbor);
        if (*possible_A != 'A') {
            continue;
        }

        if (!grid.has_neighbor(direction, possible_A)) {
            continue;
        }

        const auto possible_S = grid.neighbor(direction, possible_A);
        if (*possible_S != 'S') {
            continue;
        }

        total += 1;
    }

    return total;
}

template<advent::string_viewable_range Rng>
constexpr std::size_t count_xmas_occurrences(Rng &&rng) {
    const auto grid = parse_grid(std::forward<Rng>(rng));

    std::size_t num_occurrences = 0;
    for (const auto &elem : grid.elements()) {
        if (elem != 'X') {
            continue;
        }

        num_occurrences += count_xmas_branches(grid, &elem);
    }

    return num_occurrences;
}

constexpr bool is_mas_x(const advent::grid<char> &grid, const char *center) {
    [[assume(*center == 'A')]];

    if (!grid.has_above_left_neighbor(center)) {
        return false;
    }

    if (!grid.has_above_right_neighbor(center)) {
        return false;
    }

    if (!grid.has_below_left_neighbor(center)) {
        return false;
    }

    if (!grid.has_below_right_neighbor(center)) {
        return false;
    }

    /* TODO: Make this less repetitive. */

    if (*grid.above_left_neighbor(center) == 'M') {
        if (*grid.below_right_neighbor(center) != 'S') {
            return false;
        }

        if (*grid.above_right_neighbor(center) == 'M') {
            if (*grid.below_left_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        if (*grid.below_left_neighbor(center) == 'M') {
            if (*grid.above_right_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        return false;
    }

    if (*grid.above_right_neighbor(center) == 'M') {
        if (*grid.below_left_neighbor(center) != 'S') {
            return false;
        }

        if (*grid.above_left_neighbor(center) == 'M') {
            if (*grid.below_right_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        if (*grid.below_right_neighbor(center) == 'M') {
            if (*grid.above_left_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        return false;
    }

    if (*grid.below_left_neighbor(center) == 'M') {
        if (*grid.above_right_neighbor(center) != 'S') {
            return false;
        }

        if (*grid.below_right_neighbor(center) == 'M') {
            if (*grid.above_left_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        if (*grid.above_left_neighbor(center) == 'M') {
            if (*grid.below_right_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        return false;
    }

    if (*grid.below_right_neighbor(center) == 'M') {
        if (*grid.above_left_neighbor(center) != 'S') {
            return false;
        }

        if (*grid.below_left_neighbor(center) == 'M') {
            if (*grid.above_right_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        if (*grid.above_right_neighbor(center) == 'M') {
            if (*grid.below_left_neighbor(center) == 'S') {
                return true;
            }

            return false;
        }

        return false;
    }

    return false;
}

template<advent::string_viewable_range Rng>
constexpr std::size_t count_mas_x_occurrences(Rng &&rng) {
    const auto grid = parse_grid(std::forward<Rng>(rng));

    /* NOTE: We *could* just skip the grid edges, but meh. */

    std::size_t num_occurrences = 0;
    for (const auto &elem : grid.elements()) {
        if (elem != 'A') {
            continue;
        }

        if (is_mas_x(grid, &elem)) {
            num_occurrences += 1;
        }
    }

    return num_occurrences;
}

constexpr std::size_t count_xmas_occurrences_from_string_data(const std::string_view data) {
    return count_xmas_occurrences(data | advent::views::split_lines);
}

constexpr std::size_t count_mas_x_occurrences_from_string_data(const std::string_view data) {
    return count_mas_x_occurrences(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "MMMSXXMASM\n"
    "MSAMXMSMSA\n"
    "AMXSXMAAMM\n"
    "MSAMASMSMX\n"
    "XMASAMXAMM\n"
    "XXAMMXXAMA\n"
    "SMSMSASXSS\n"
    "SAXAMASAAA\n"
    "MAMMMXMMMM\n"
    "MXMXAXMASX\n"
);

static_assert(count_xmas_occurrences_from_string_data(example_data) == 18);
static_assert(count_mas_x_occurrences_from_string_data(example_data) == 9);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_xmas_occurrences_from_string_data,
        count_mas_x_occurrences_from_string_data
    );
}
