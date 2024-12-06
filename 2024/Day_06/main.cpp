#include <advent/advent.hpp>

enum class Tile : char {
    Obstruction = '#',
    Empty       = '.',
    PassedOver  = 'X',
};

enum class GuardAction : std::uint8_t {
    Escape,
    Turn,
    Move,
};

struct GuardInfo {
    advent::vector_2d<std::size_t> position;
    advent::adjacent_neighbor      direction;

    constexpr auto tile(this const GuardInfo &self, auto &map) {
        return &map.map[self.position];
    }

    constexpr bool operator ==(const GuardInfo &) const = default;
};

struct Map {
    /* NOTE: We use an owning grid so that we can modify the tiles. */
    advent::grid<Tile> map;

    GuardInfo guard_info;

    static constexpr advent::adjacent_neighbor CharToDirection(const char character) {
        switch (character) {
            using enum advent::adjacent_neighbor;

            case '^': return above;
            case '>': return right;
            case 'v': return below;
            case '<': return left;

            default: std::unreachable();
        }
    }

    constexpr void _initialize_guard(this Map &self, advent::grid<Tile>::builder &builder, Tile *tile, const char guard_character) {
        self.guard_info = {builder.coords_of(tile), CharToDirection(guard_character)};

        *tile = Tile::PassedOver;
    }

    template<advent::string_viewable_range Rng>
    constexpr explicit Map(Rng &&rng) {
        this->map = advent::grid<Tile>::build([&](auto &builder) {
            for (const std::string_view line : std::forward<Rng>(rng)) {
                if (line.empty()) {
                    continue;
                }

                builder.push_row(line.size(), [&](const auto row) {
                    for (const auto [tile, character] : std::views::zip(row, line)) {
                        switch (character) {
                            case '#':
                            case '.': {
                                tile = Tile{character};
                            } break;

                            default: {
                                /* Guard character. */

                                this->_initialize_guard(builder, &tile, character);
                            } break;
                        }
                    }
                });
            }
        });
    }

    constexpr GuardAction next_guard_action(this const Map &self, const GuardInfo &guard_info) {
        const auto guard = guard_info.tile(self);

        if (!self.map.has_neighbor(guard_info.direction, guard)) {
            return GuardAction::Escape;
        }

        const auto next_tile = self.map.neighbor(guard_info.direction, guard);
        if (*next_tile == Tile::Obstruction) {
            return GuardAction::Turn;
        }

        return GuardAction::Move;
    }

    constexpr void mark_passed_over_tiles(this Map &self) {
        /* NOTE: This doesn't mark the starting position. */

        auto guard_info = self.guard_info;

        while (true) {
            switch (self.next_guard_action(guard_info)) {
                case GuardAction::Escape:
                    return;

                case GuardAction::Turn: {
                    guard_info.direction = advent::next_clockwise_neighbor(guard_info.direction);
                } break;

                case GuardAction::Move: {
                    guard_info.position = self.map.neighbor_of_coords(
                        guard_info.direction,
                        guard_info.position
                    );

                    *guard_info.tile(self) = Tile::PassedOver;
                } break;

                default: std::unreachable();
            }
        }
    }
};

struct MapForEscape : Map {
    using Map::Map;

    constexpr std::size_t count_visited_positions(this MapForEscape &self) {
        /* NOTE: The starting tile is already marked as passed over. */

        self.mark_passed_over_tiles();

        return std::ranges::count(self.map.elements(), Tile::PassedOver);
    }
};

struct MapForLoop : Map {
    using Map::Map;

    constexpr bool _does_new_obstruction_loop(this MapForLoop &self, Tile &tile_to_obstruct) {
        [[assume(self.map.coords_of(&tile_to_obstruct) != self.guard_info.position)]];
        [[assume(tile_to_obstruct == Tile::PassedOver)]];

        tile_to_obstruct = Tile::Obstruction;

        const advent::scope_guard reset_tile = [&]() {
            tile_to_obstruct = Tile::PassedOver;
        };

        /* If we come across the same turn, then we are on a loop. */
        std::vector<GuardInfo> turn_records;

        auto guard_info = self.guard_info;
        while (true) {
            switch (self.next_guard_action(guard_info)) {
                case GuardAction::Escape:
                    return false;

                case GuardAction::Move: {
                    guard_info.position = self.map.neighbor_of_coords(
                        guard_info.direction,
                        guard_info.position
                    );
                } break;

                case GuardAction::Turn: {
                    if (std::ranges::contains(turn_records, guard_info)) {
                        return true;
                    }

                    turn_records.push_back(guard_info);

                    guard_info.direction = advent::next_clockwise_neighbor(guard_info.direction);
                } break;

                default: std::unreachable();
            }
        }
    }

    constexpr std::size_t count_looping_new_obstructions(this MapForLoop &self) {
        /*
            NOTE: We only try to add obstructions to tiles
            that the guard would have otherwise passed over.
        */

        self.mark_passed_over_tiles();

        /*
            We pretend we don't pass over the starting tile so that we
            don't have to later check that a tile isn't the starting tile.
        */
        *self.guard_info.tile(self) = Tile::Empty;

        std::size_t sum = 0;
        for (auto &tile : self.map.elements()) {
            /* NOTE: The starting tile is not marked as passed over. */
            if (tile != Tile::PassedOver) {
                continue;
            }

            if (self._does_new_obstruction_loop(tile)) {
                sum += 1;
            }
        }

        return sum;
    }
};

template<advent::string_viewable_range Rng>
constexpr std::size_t count_visited_positions(Rng &&rng) {
    auto map = MapForEscape(std::forward<Rng>(rng));

    return map.count_visited_positions();
}

template<advent::string_viewable_range Rng>
constexpr std::size_t count_looping_new_obstructions(Rng &&rng) {
    auto map = MapForLoop(std::forward<Rng>(rng));

    return map.count_looping_new_obstructions();
}

constexpr std::size_t count_visited_positions_from_string_data(const std::string_view data) {
    return count_visited_positions(data | advent::views::split_lines);
}

constexpr std::size_t count_looping_new_obstructions_from_string_data(const std::string_view data) {
    return count_looping_new_obstructions(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "....#.....\n"
    ".........#\n"
    "..........\n"
    "..#.......\n"
    ".......#..\n"
    "..........\n"
    ".#..^.....\n"
    "........#.\n"
    "#.........\n"
    "......#...\n"
);

static_assert(count_visited_positions_from_string_data(example_data) == 41);
static_assert(count_looping_new_obstructions_from_string_data(example_data) == 6);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_visited_positions_from_string_data,
        count_looping_new_obstructions_from_string_data
    );
}
