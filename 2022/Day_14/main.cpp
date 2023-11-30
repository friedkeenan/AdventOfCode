#include <advent/advent.hpp>

struct Cave {
    enum class Tile : std::uint8_t {
        Air,
        Rock,
        Sand,
    };

    using Position = advent::vector_2d<std::int64_t>;

    struct Path {
        static constexpr bool LineIsVertical(const Position start, const Position end) {
            return start.y() != end.y();
        }

        std::vector<Position> positions;

        constexpr explicit Path(const std::string_view path) {
            advent::split_with_callback(path, " -> ", [&](std::string_view pos) {
                const auto comma_pos = pos.find_first_of(',');
                [[assume(comma_pos != std::string_view::npos)]];

                const auto x = advent::to_integral<std::int64_t>(std::string_view(pos.data(), comma_pos));
                pos.remove_prefix(comma_pos + 1);

                const auto y = advent::to_integral<std::int64_t>(pos);

                /* It doesn't like us using 'emplace_back'. */
                this->positions.push_back(Position{x, y});
            });
        }

        template<typename Consumer>
        requires (std::invocable<const Consumer &, const Position &, const Position &>)
        constexpr void for_each_line(const Consumer consumer) const {
            for (const auto i : std::views::iota(1uz, this->positions.size())) {
                std::invoke(consumer, this->positions[i - 1], this->positions[i]);
            }
        }
    };

    static constexpr Position SandSource = {500, 0};

    Position _grid_origin;
    std::size_t _grid_width;
    std::vector<Tile> _grid;

    template<std::ranges::input_range Rng>
    requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
    constexpr explicit Cave(const bool has_floor, Rng &&rock_paths) {
        std::vector<Path> paths;

        /* The sand source must be included in the grid. */
        Position min = SandSource;
        Position max = SandSource;
        for (const std::string_view description : std::forward<Rng>(rock_paths)) {
            if (description.empty()) {
                continue;
            }

            const auto &path = paths.emplace_back(description);

            for (const auto &pos : path.positions) {
                if (pos.x() < min.x()) {
                    min.x() = pos.x();
                }

                if (pos.y() < min.y()) {
                    min.y() = pos.y();
                }

                if (pos.x() > max.x()) {
                    max.x() = pos.x();
                }

                if (pos.y() > max.y()) {
                    max.y() = pos.y();
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

        const auto dimensions = max - min + Position{1, 1};
        this->_grid_width = dimensions.x();
        this->_grid.resize(dimensions.x() * dimensions.y(), Tile::Air);

        if (has_floor) {
            /* Build the floor. */
            for (auto pos = max; pos.x() >= min.x(); --pos.x()) {
                this->_at_local(this->to_local(pos)) = Tile::Rock;
            }
        }

        /* Draw the rock lines. */
        for (const auto &path : paths) {
            path.for_each_line([&](const Position start, const Position end) {
                const auto local_start = this->to_local(start);
                const auto local_end   = this->to_local(end);

                if (Path::LineIsVertical(local_start, local_end)) {
                    if (local_start.y() <= local_end.y()) {
                        for (auto pos = local_start; pos.y() <= local_end.y(); ++pos.y()) {
                            this->_at_local(pos) = Tile::Rock;
                        }
                    } else {
                        for (auto pos = local_start; pos.y() >= local_end.y(); --pos.y()) {
                            this->_at_local(pos) = Tile::Rock;
                        }
                    }
                } else {
                    if (local_start.x() <= local_end.x()) {
                        for (auto pos = local_start; pos.x() <= local_end.x(); ++pos.x()) {
                            this->_at_local(pos) = Tile::Rock;
                        }
                    } else {
                        for (auto pos = local_start; pos.x() >= local_end.x(); --pos.x()) {
                            this->_at_local(pos) = Tile::Rock;
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

    constexpr Position to_local(const Position absolute) const {
        return absolute - this->_grid_origin;
    }

    constexpr Position local_sand_source() const {
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

    constexpr Tile &_at_local(const Position pos) {
        return this->_at_local(pos.x(), pos.y());
    }

    constexpr const Tile &_at_local(const Position pos) const {
        return this->_at_local(pos.x(), pos.y());
    }

    constexpr bool local_outside_grid(const Position pos) const {
        return (
            (pos.x() < 0 || std::cmp_greater_equal(pos.x(), this->grid_width()))  ||
            (pos.y() < 0 || std::cmp_greater_equal(pos.y(), this->grid_height()))
        );
    }

    constexpr bool drop_sand() {
        auto sand = this->local_sand_source();

        while (true) {
            const auto under = sand + Position{0, 1};
            if (this->local_outside_grid(under)) {
                return false;
            }

            if (this->_at_local(under) == Tile::Air) {
                ++sand.y();

                continue;
            }

            /*
                NOTE: With our logic, sand can clip between diagonal rocks.
                It doesn't seem to be an issue.
            */

            const auto under_left = under - Position{1, 0};
            if (this->local_outside_grid(under_left)) {
                return false;
            }

            if (this->_at_local(under_left) == Tile::Air) {
                ++sand.y();
                --sand.x();

                continue;
            }

            const auto under_right = under + Position{1, 0};
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
    return num_resting_sand_units_no_floor(data | advent::views::split_lines);
}

constexpr std::size_t max_sand_units_with_floor_from_string_data(const std::string_view data) {
    return max_sand_units_with_floor(data | advent::views::split_lines);
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
