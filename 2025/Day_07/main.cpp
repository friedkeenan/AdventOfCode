import std;
import advent;

struct ClassicalTachyonManifold {
    enum class Tile : char {
        Start    = 'S',
        Splitter = '^',
        Empty    = '.',
        Beam     = '|',
    };

    advent::grid<Tile> grid;

    template<advent::string_viewable_range Rng>
    constexpr explicit ClassicalTachyonManifold(Rng &&lines)
    :
        grid([&](auto &builder) {
            for (const std::string_view line : std::forward<Rng>(lines)) {
                if (line.empty()) {
                    continue;
                }

                builder.push_row(line.size(), [&](const auto row) {
                    for (auto [character, tile] : std::views::zip(line, row)) {
                        tile = Tile{character};
                    }
                });
            }
        })
    {}

    constexpr Tile *start_location(this ClassicalTachyonManifold &self) {
        for (auto &tile : self.grid.elements()) {
            if (tile == Tile::Start) {
                return &tile;
            }
        }

        std::unreachable();
    }

    template<advent::neighbor_enum auto Direction>
    constexpr std::size_t count_splits_from_direction(this ClassicalTachyonManifold &self, Tile *location) {
        [[assume(*location == Tile::Splitter)]];

        if (self.grid.has_neighbor(Direction, location)) {
            const auto neighbor = self.grid.neighbor(Direction, location);
            if (*neighbor != Tile::Empty) {
                return 0uz;
            }

            return self.count_splits_from(neighbor);
        }

        return 0uz;
    }

    constexpr std::size_t count_splits_from(this ClassicalTachyonManifold &self, Tile *location) {
        [[assume(*location != Tile::Splitter)]];
        [[assume(*location != Tile::Beam)]];

        while (self.grid.has_below_neighbor(location)) {
            location = self.grid.below_neighbor(location);

            if (*location == Tile::Splitter) {
                const auto left_count  = self.count_splits_from_direction<advent::adjacent_neighbor::left>(location);
                const auto right_count = self.count_splits_from_direction<advent::adjacent_neighbor::right>(location);

                return 1 + left_count + right_count;
            }

            if (*location != Tile::Empty) {
                return 0uz;
            }

            [[assume(*location == Tile::Empty)]];

            *location = Tile::Beam;
        }

        return 0;
    }

    constexpr std::size_t count_splits(this ClassicalTachyonManifold &&self) {
        const auto start = self.start_location();

        return self.count_splits_from(start);
    }
};

struct QuantumTachyonManifold {
    static constexpr char SplitterCharacter = '^';
    static constexpr char StartCharacter    = 'S';

    static constexpr std::size_t UnmarkedTile = 0;
    static constexpr std::size_t SplitterTile = -1;
    static constexpr std::size_t StartTile    = -2;

    advent::grid<std::size_t> grid;

    template<advent::string_viewable_range Rng>
    constexpr explicit QuantumTachyonManifold(Rng &&lines)
    :
        grid([&](auto &builder) {
            for (const std::string_view line : std::forward<Rng>(lines)) {
                if (line.empty()) {
                    continue;
                }

                builder.push_row(line.size(), [&](const auto row) {
                    for (auto [character, tile] : std::views::zip(line, row)) {
                        if (character == SplitterCharacter) {
                            tile = SplitterTile;

                            continue;
                        }

                        if (character == StartCharacter) {
                            tile = StartTile;

                            continue;
                        }

                        tile = UnmarkedTile;
                    }
                });
            }
        })
    {}

    constexpr std::size_t *start_location(this QuantumTachyonManifold &self) {
        for (auto &tile : self.grid.elements()) {
            if (tile == StartTile) {
                return &tile;
            }
        }

        std::unreachable();
    }

    template<advent::neighbor_enum auto Direction>
    constexpr std::size_t count_timelines_from_direction(this QuantumTachyonManifold &self, std::size_t *location) {
        [[assume(*location == SplitterTile)]];

        if (self.grid.has_neighbor(Direction, location)) {
            const auto neighbor = self.grid.neighbor(Direction, location);

            if (*neighbor == UnmarkedTile) {
                /* Cache the count to use when another timeline splits here. */
                *neighbor = self.count_timelines_from(neighbor);
            }

            return *neighbor;
        }

        return 0uz;
    }

    constexpr std::size_t count_timelines_from(this QuantumTachyonManifold &self, std::size_t *location) {
        [[assume(*location == UnmarkedTile || *location == StartTile)]];

        while (self.grid.has_below_neighbor(location)) {
            location = self.grid.below_neighbor(location);

            if (*location == SplitterTile) {
                const auto left_count  = self.count_timelines_from_direction<advent::adjacent_neighbor::left>(location);
                const auto right_count = self.count_timelines_from_direction<advent::adjacent_neighbor::right>(location);

                return left_count + right_count;
            }
        }

        return 1;
    }

    constexpr std::size_t count_timelines(this QuantumTachyonManifold &&self) {
        const auto start = self.start_location();

        return self.count_timelines_from(start);
    }
};

template<advent::string_viewable_range Rng>
constexpr std::size_t count_tachyon_splits(Rng &&rng) {
    return ClassicalTachyonManifold(std::forward<Rng>(rng)).count_splits();
}

template<advent::string_viewable_range Rng>
constexpr std::size_t count_tachyon_timelines(Rng &&rng) {
    return QuantumTachyonManifold(std::forward<Rng>(rng)).count_timelines();
}

constexpr std::size_t count_tachyon_splits_from_string_data(const std::string_view data) {
    return count_tachyon_splits(data | advent::views::split_lines);
}

constexpr std::size_t count_tachyon_timelines_from_string_data(const std::string_view data) {
    return count_tachyon_timelines(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    ".......S.......\n"
    "...............\n"
    ".......^.......\n"
    "...............\n"
    "......^.^......\n"
    "...............\n"
    ".....^.^.^.....\n"
    "...............\n"
    "....^.^...^....\n"
    "...............\n"
    "...^.^...^.^...\n"
    "...............\n"
    "..^...^.....^..\n"
    "...............\n"
    ".^.^.^.^.^...^.\n"
    "...............\n"
);

static_assert(count_tachyon_splits_from_string_data(example_data)    == 21);
static_assert(count_tachyon_timelines_from_string_data(example_data) == 40);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_tachyon_splits_from_string_data,
        count_tachyon_timelines_from_string_data
    );
}
