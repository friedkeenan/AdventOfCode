#include <advent/advent.hpp>

constexpr std::size_t count_xmas_branches(const advent::string_view_grid &grid, const char *start) {
    [[assume(*start == 'X')]];

    std::size_t total = 0;
    for (const auto [position, neighbor] : grid.neighbors_of(start)) {
        if (*neighbor != 'M') {
            continue;
        }

        if (!grid.has_neighbor(position, neighbor)) {
            continue;
        }

        const auto possible_A = grid.neighbor(position, neighbor);
        if (*possible_A != 'A') {
            continue;
        }

        if (!grid.has_neighbor(position, possible_A)) {
            continue;
        }

        const auto possible_S = grid.neighbor(position, possible_A);
        if (*possible_S != 'S') {
            continue;
        }

        total += 1;
    }

    return total;
}

constexpr std::size_t count_xmas_occurrences(const std::string_view data) {
    const auto grid = advent::string_view_grid(data);

    std::size_t num_occurrences = 0;
    for (const auto &elem : grid.elements()) {
        if (elem != 'X') {
            continue;
        }

        num_occurrences += count_xmas_branches(grid, &elem);
    }

    return num_occurrences;
}

constexpr bool is_mas_x(const advent::string_view_grid &grid, const char *center) {
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

    for (const auto position : advent::neighbor_positions<advent::diagonal_neighbor>) {
        if (*grid.neighbor(position, center) == 'M') {
            if (*grid.neighbor(advent::opposite_neighbor(position), center) != 'S') {
                return false;
            }

            if (*grid.neighbor(advent::row_opposite_neighbor(position), center) == 'M') {
                return *grid.neighbor(advent::column_opposite_neighbor(position), center) == 'S';
            }

            if (*grid.neighbor(advent::column_opposite_neighbor(position), center) == 'M') {
                return *grid.neighbor(advent::row_opposite_neighbor(position), center) == 'S';
            }
        }
    }

    return false;
}

constexpr std::size_t count_mas_x_occurrences(const std::string_view data) {
    const auto grid = advent::string_view_grid(data);

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

static_assert(count_xmas_occurrences(example_data) == 18);
static_assert(count_mas_x_occurrences(example_data) == 9);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_xmas_occurrences,
        count_mas_x_occurrences
    );
}
