import std;
import advent;

struct PaperRollsMap {
    static constexpr char FilledSpace = '@';
    static constexpr char EmptySpace  = '.';

    /* Derived classes should add a 'grid' member. */

    constexpr bool is_accessible_roll(this const auto &self, const char *elem) {
        static constexpr std::size_t MaxFilledNeighbors = 3;

        if (*elem != FilledSpace) {
            return false;
        }

        std::size_t filled_neighbors = 0;

        for (const auto [_, neighbor] : self.grid.neighbors_of(elem)) {
            if (*neighbor == FilledSpace) {
                filled_neighbors += 1;
            }
        }

        return filled_neighbors <= MaxFilledNeighbors;
    }
};

struct StaticMap : PaperRollsMap {
    advent::string_view_grid grid;

    constexpr explicit StaticMap(std::string_view map) : grid(map) {}

    constexpr std::size_t count_accessible_rolls(this const StaticMap &self) {
        return static_cast<std::size_t>(
            std::ranges::count_if(self.grid.elements(), [&](const auto &elem) {
                return self.is_accessible_roll(&elem);
            })
        );
    }
};

struct IterativeMap : PaperRollsMap {
    advent::grid<char> grid;

    constexpr explicit IterativeMap(const std::string_view map)
    :
        grid([&](auto &builder) {
            advent::split_with_callback(map, '\n', [&](const auto row) {
                if (row.empty()) {
                    return;
                }

                builder.push_row(row);
            });
        })
    {}

    /*
        NOTE: We take 'self' as an rvalue
        so that we don't have to worry about
        preserving its value or anything.
    */
    constexpr std::size_t count_accessible_rolls(this IterativeMap &&self) {
        std::size_t total_accessible = 0;

        while (true) {
            auto future_grid = self.grid;

            std::size_t currently_accessible = 0;
            for (const auto [current_elem, future_elem] : std::views::zip(self.grid.elements(), future_grid.elements())) {
                if (!self.is_accessible_roll(&current_elem)) {
                    continue;
                }

                currently_accessible += 1;

                future_elem = EmptySpace;
            }

            if (currently_accessible <= 0) {
                return total_accessible;
            }

            total_accessible += currently_accessible;

            self.grid = std::move(future_grid);
        }
    }
};

template<std::derived_from<PaperRollsMap> Map>
constexpr std::size_t count_accessible_rolls(const std::string_view data) {
    return Map(data).count_accessible_rolls();
}

constexpr inline std::string_view example_data = (
    "..@@.@@@@.\n"
    "@@@.@.@.@@\n"
    "@@@@@.@.@@\n"
    "@.@@@@..@.\n"
    "@@.@@@@.@@\n"
    ".@@@@@@@.@\n"
    ".@.@.@.@@@\n"
    "@.@@@.@@@@\n"
    ".@@@@@@@@.\n"
    "@.@.@@@.@.\n"
);

static_assert(count_accessible_rolls<StaticMap>(example_data)    == 13);
static_assert(count_accessible_rolls<IterativeMap>(example_data) == 43);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_accessible_rolls<StaticMap>,
        count_accessible_rolls<IterativeMap>
    );
}
