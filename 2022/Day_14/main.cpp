#include <advent/advent.hpp>

class Cave {
    public:
        enum class Tile : std::uint8_t {
            Air,
            Rock,
            Sand,
        };

        using Coord = advent::vector_2d<std::int64_t>;

        class Path {
            public:
                static constexpr bool LineIsVertical(const Coord start, const Coord end) {
                    return start.y() != end.y();
                }

                std::vector<Coord> coords;

                constexpr explicit Path(const std::string_view path) {
                    advent::split_with_callback(path, " -> ", [&](std::string_view coord) {
                        const auto comma_pos = coord.find_first_of(',');
                        advent::assume(comma_pos != std::string_view::npos);

                        const auto x = advent::to_integral<std::int64_t>(std::string_view(coord.data(), comma_pos));
                        coord.remove_prefix(comma_pos + 1);

                        const auto y = advent::to_integral<std::int64_t>(coord);

                        /* It doesn't like us using 'emplace_back'. */
                        this->coords.push_back(Coord{x, y});
                    });
                }

                template<typename Consumer>
                requires (std::invocable<const Consumer &, const Coord &, const Coord &>)
                constexpr void for_each_line(const Consumer consumer) const {
                    for (const auto i : std::views::iota(1uz, this->coords.size())) {
                        std::invoke(consumer, this->coords[i - 1], this->coords[i]);
                    }
                }
        };

        static constexpr Coord SandSource = {500, 0};

        Coord _grid_origin;
        std::size_t _grid_width;
        std::vector<Tile> _grid;

        template<std::ranges::input_range Rng>
        requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
        constexpr explicit Cave(const bool has_floor, Rng &&rock_paths) {
            std::vector<Path> paths;

            /* The sand source must be included in the grid. */
            Coord min = SandSource;
            Coord max = SandSource;
            for (const std::string_view description : std::forward<Rng>(rock_paths)) {
                if (description.empty()) {
                    continue;
                }

                const auto &path = paths.emplace_back(description);

                for (const auto &coord : path.coords) {
                    if (coord.x() < min.x()) {
                        min.x() = coord.x();
                    }

                    if (coord.y() < min.y()) {
                        min.y() = coord.y();
                    }

                    if (coord.x() > max.x()) {
                        max.x() = coord.x();
                    }

                    if (coord.y() > max.y()) {
                        max.y() = coord.y();
                    }
                }
            }

            if (has_floor) {
                /* Make sure we can fit the max amount of tiles. Could probably do better here. */

                max.y() += 2;
                max.x() += max.y();
                min.x() -= max.y();
            }

            this->_grid_origin = min;

            const auto dimensions = max - min + Coord{1, 1};
            this->_grid_width = dimensions.x();
            this->_grid.resize(dimensions.x() * dimensions.y(), Tile::Air);

            if (has_floor) {
                /* Build the floor. */
                for (auto coord = max; coord.x() >= min.x(); --coord.x()) {
                    this->_at_local(this->to_local(coord)) = Tile::Rock;
                }
            }

            /* Draw the rock lines. */
            for (const auto &path : paths) {
                path.for_each_line([&](const Coord start, const Coord end) {
                    const auto local_start = this->to_local(start);
                    const auto local_end   = this->to_local(end);

                    if (Path::LineIsVertical(local_start, local_end)) {
                        if (local_start.y() <= local_end.y()) {
                            for (auto coord = local_start; coord.y() <= local_end.y(); ++coord.y()) {
                                this->_at_local(coord) = Tile::Rock;
                            }
                        } else {
                            for (auto coord = local_start; coord.y() >= local_end.y(); --coord.y()) {
                                this->_at_local(coord) = Tile::Rock;
                            }
                        }
                    } else {
                        if (local_start.x() <= local_end.x()) {
                            for (auto coord = local_start; coord.x() <= local_end.x(); ++coord.x()) {
                                this->_at_local(coord) = Tile::Rock;
                            }
                        } else {
                            for (auto coord = local_start; coord.x() >= local_end.x(); --coord.x()) {
                                this->_at_local(coord) = Tile::Rock;
                            }
                        }
                    }
                });
            }
        }

        constexpr std::int64_t to_local_x(const std::int64_t x) const {
            return x - this->_grid_origin.x();
        }

        constexpr std::int64_t to_local_y(const std::int64_t y) const {
            return y - this->_grid_origin.y();
        }

        constexpr Coord to_local(const Coord absolute) const {
            return absolute - this->_grid_origin;
        }

        constexpr Coord local_sand_source() const {
            return this->to_local(SandSource);
        }

        constexpr std::size_t grid_width() const {
            return this->_grid_width;
        }

        constexpr std::size_t grid_height() const {
            return this->_grid.size() / this->grid_width();
        }

        constexpr Tile &_at_local(const std::int64_t x, const std::int64_t y) {
            return this->_grid[y * this->grid_width() + x];
        }

        constexpr const Tile &_at_local(const std::int64_t x, const std::int64_t y) const {
            return this->_grid[y * this->grid_width() + x];
        }

        constexpr Tile &_at_local(const Coord coord) {
            return this->_at_local(coord.x(), coord.y());
        }

        constexpr const Tile &_at_local(const Coord coord) const {
            return this->_at_local(coord.x(), coord.y());
        }

        constexpr bool local_outside_grid(const Coord coord) const {
            return (
                (coord.x() < 0 || std::cmp_greater_equal(coord.x(), this->grid_width()))  ||
                (coord.y() < 0 || std::cmp_greater_equal(coord.y(), this->grid_height()))
            );
        }

        constexpr bool drop_sand() {
            auto sand = this->local_sand_source();

            while (true) {
                const auto under = sand + Coord{0, 1};
                if (this->local_outside_grid(under)) {
                    return false;
                }

                if (this->_at_local(under) == Tile::Air) {
                    ++sand.y();

                    continue;
                }

                /* TODO: Are our checks sufficient for diagonal blockage? */

                const auto under_left = under - Coord{1, 0};
                if (this->local_outside_grid(under_left)) {
                    return false;
                }

                if (this->_at_local(under_left) == Tile::Air) {
                    ++sand.y();
                    --sand.x();

                    continue;
                }

                const auto under_right = under + Coord{1, 0};
                if (this->local_outside_grid(under_right)) {
                    return false;
                }

                if (this->_at_local(under_right) == Tile::Air) {
                    ++sand.y();
                    ++sand.x();

                    continue;
                }

                this->_at_local(sand) = Tile::Sand;
                if (sand == this->local_sand_source()) {
                    return false;
                }

                return true;
            }
        }

        constexpr void print() const {
            for (const auto i : std::views::iota(0uz, this->_grid.size())) {
                if (i % this->grid_width() == 0) {
                    advent::print("\n");
                }

                const auto tile = this->_grid[i];
                switch (tile) {
                    case Tile::Air: {
                        advent::print(".");
                    } break;

                    case Tile::Rock: {
                        advent::print("#");
                    } break;

                    case Tile::Sand: {
                        advent::print("O");
                    } break;

                    default: std::unreachable();
                }
            }

            advent::print("\n");
        }
};

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t num_resting_sand_units_no_floor(Rng &&rock_paths) {
    auto cave = Cave(false, std::forward<Rng>(rock_paths));

    std::size_t num_sand_units = 0;
    while (true) {
        const auto at_rest = cave.drop_sand();
        if (!at_rest) {
            return num_sand_units;
        }

        ++num_sand_units;
    }
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t max_sand_units_with_floor(Rng &&rock_paths) {
    auto cave = Cave(true, std::forward<Rng>(rock_paths));

    std::size_t num_sand_units = 0;
    while (true) {
        const auto source_free = cave.drop_sand();
        if (!source_free) {
            return num_sand_units + 1;
        }

        ++num_sand_units;
    }
}

constexpr std::size_t num_resting_sand_units_no_floor_from_string_data(const std::string_view data) {
    return num_resting_sand_units_no_floor(advent::views::split_lines(data));
}

constexpr std::size_t max_sand_units_with_floor_from_string_data(const std::string_view data) {
    return max_sand_units_with_floor(advent::views::split_lines(data));
}

constexpr inline std::string_view example_data = (
    "498,4 -> 498,6 -> 496,6\n"
    "503,4 -> 502,4 -> 502,9 -> 494,9\n"
);

static_assert(num_resting_sand_units_no_floor_from_string_data(example_data) == 24);
static_assert(max_sand_units_with_floor_from_string_data(example_data) == 93);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        num_resting_sand_units_no_floor_from_string_data,
        max_sand_units_with_floor_from_string_data
    );
}
