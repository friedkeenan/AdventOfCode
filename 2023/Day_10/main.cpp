#include <advent/advent.hpp>

enum class Direction : std::uint8_t {
    North,
    South,
    East,
    West,
};

template<Direction D>
concept ValidDirection = (
    D == Direction::North ||
    D == Direction::South ||
    D == Direction::East  ||
    D == Direction::West
);

enum class Pipe : char {
    NorthSouth = '|',
    EastWest   = '-',
    NorthEast  = 'L',
    NorthWest  = 'J',
    SouthWest  = '7',
    SouthEast  = 'F',

    Ground = '.',
    Start  = 'S',
};

struct Pipes {
    /* TODO: At this point we should probably just make a 2D vector helper. */
    std::vector<Pipe> grid;
    std::size_t       width = 0;

    std::size_t start_pos = 0;

    template<std::ranges::input_range Rng>
    requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
    constexpr explicit Pipes(Rng &&rng) {
        for (const std::string_view line : std::forward<Rng>(rng)) {
            if (line.empty()) {
                continue;
            }

            if (this->width <= 0) {
                this->width = line.size();
            }

            [[assume(line.size() == this->width)]];

            for (const char &c : line) {
                const auto pipe = Pipe{c};

                if (pipe == Pipe::Start) {
                    this->start_pos = this->grid.size();
                }

                this->grid.push_back(pipe);
            }
        }

        this->grid[this->start_pos] = this->real_start_pipe();
    }

    static constexpr bool CanMoveNorth(const Pipe pipe) {
        switch (pipe) {
            case Pipe::NorthSouth:
            case Pipe::NorthEast:
            case Pipe::NorthWest:
                return true;

            default:
                return false;
        }
    }

    static constexpr bool CanMoveSouth(const Pipe pipe) {
        switch (pipe) {
            case Pipe::NorthSouth:
            case Pipe::SouthEast:
            case Pipe::SouthWest:
                return true;

            default:
                return false;
        }
    }

    static constexpr bool CanMoveEast(const Pipe pipe) {
        switch (pipe) {
            case Pipe::EastWest:
            case Pipe::NorthEast:
            case Pipe::SouthEast:
                return true;

            default:
                return false;
        }
    }

    static constexpr bool CanMoveWest(const Pipe pipe) {
        switch (pipe) {
            case Pipe::EastWest:
            case Pipe::NorthWest:
            case Pipe::SouthWest:
                return true;

            default:
                return false;
        }
    }

    constexpr const Pipe &start() const {
        return this->grid[this->start_pos];
    }

    constexpr bool start_connects_north() const {
        return this->pos_can_go_north(this->start_pos) && CanMoveSouth(this->north_of(this->start()));
    }

    constexpr bool start_connects_south() const {
        return this->pos_can_go_south(this->start_pos) && CanMoveNorth(this->south_of(this->start()));
    }

    constexpr bool start_connects_east() const {
        return this->pos_can_go_east(this->start_pos) && CanMoveWest(this->east_of(this->start()));
    }

    constexpr bool start_connects_west() const {
        return this->pos_can_go_west(this->start_pos) && CanMoveEast(this->west_of(this->start()));
    }

    constexpr Pipe real_start_pipe() const {
        if (this->start_connects_north()) {
            if (this->start_connects_south()) {
                return Pipe::NorthSouth;
            }

            if (this->start_connects_east()) {
                return Pipe::NorthEast;
            }

            if (this->start_connects_west()) {
                return Pipe::NorthWest;
            }
        }

        if (this->start_connects_south()) {
            /* NOTE: We don't have to check a north connection. */

            if (this->start_connects_east()) {
                return Pipe::SouthEast;
            }

            if (this->start_connects_west()) {
                return Pipe::SouthWest;
            }
        }

        [[assume(this->start_connects_east() && this->start_connects_west())]];

        return Pipe::EastWest;
    }

    constexpr std::size_t pos_for(const Pipe &pipe) const {
        [[assume(&pipe >= this->grid.data() && &pipe < this->grid.data() + this->grid.size())]];

        return &pipe - this->grid.data();
    }

    constexpr bool pos_can_go_north(const std::size_t pos) const {
        return pos >= this->width;
    }

    constexpr bool pos_can_go_south(const std::size_t pos) const {
        return pos < this->grid.size() - this->width;
    }

    constexpr bool pos_can_go_east(const std::size_t pos) const {
        return (pos + 1) % this->width > 0;
    }

    constexpr bool pos_can_go_west(const std::size_t pos) const {
        return pos % this->width > 0;
    }

    constexpr const Pipe &north_of(const Pipe &pipe) const {
        const auto pos = this->pos_for(pipe);

        [[assume(this->pos_can_go_north(pos))]];

        return this->grid[pos - this->width];
    }

    constexpr const Pipe &south_of(const Pipe &pipe) const {
        const auto pos = this->pos_for(pipe);

        [[assume(this->pos_can_go_south(pos))]];

        return this->grid[pos + this->width];
    }

    constexpr const Pipe &east_of(const Pipe &pipe) const {
        const auto pos = this->pos_for(pipe);

        [[assume(this->pos_can_go_east(pos))]];

        return this->grid[pos + 1];
    }

    constexpr const Pipe &west_of(const Pipe &pipe) const {
        const auto pos = this->pos_for(pipe);

        [[assume(this->pos_can_go_west(pos))]];

        return this->grid[pos - 1];
    }

    template<Direction MoveDirection>
    requires (ValidDirection<MoveDirection>)
    constexpr std::pair<const Pipe &, Direction> move_through(const Pipe &pipe) const {
        if constexpr (MoveDirection == Direction::North) {
            return {this->north_of(pipe), Direction::South};
        } else if constexpr(MoveDirection == Direction::South) {
            return {this->south_of(pipe), Direction::North};
        } else if constexpr (MoveDirection == Direction::East) {
            return {this->east_of(pipe), Direction::West};
        } else {
            return {this->west_of(pipe), Direction::East};
        }
    }

    constexpr std::pair<const Pipe &, Direction> follow(const Pipe &pipe, const Direction enter) const {
        switch (pipe) {
            case Pipe::NorthSouth: {
                switch (enter) {
                    case Direction::North: return this->move_through<Direction::South>(pipe);
                    case Direction::South: return this->move_through<Direction::North>(pipe);

                    default: std::unreachable();
                }
            } break;

            case Pipe::EastWest: {
                switch (enter) {
                    case Direction::East: return this->move_through<Direction::West>(pipe);
                    case Direction::West: return this->move_through<Direction::East>(pipe);

                    default: std::unreachable();
                }
            } break;

            case Pipe::NorthEast: {
                switch (enter) {
                    case Direction::East:  return this->move_through<Direction::North>(pipe);
                    case Direction::North: return this->move_through<Direction::East>(pipe);

                    default: std::unreachable();
                }
            } break;

            case Pipe::NorthWest: {
                switch (enter) {
                    case Direction::North: return this->move_through<Direction::West>(pipe);
                    case Direction::West:  return this->move_through<Direction::North>(pipe);

                    default: std::unreachable();
                }
            } break;

            case Pipe::SouthWest: {
                switch (enter) {
                    case Direction::South: return this->move_through<Direction::West>(pipe);
                    case Direction::West:  return this->move_through<Direction::South>(pipe);

                    default: std::unreachable();
                }
            } break;

            case Pipe::SouthEast: {
                switch (enter) {
                    case Direction::South: return this->move_through<Direction::East>(pipe);
                    case Direction::East:  return this->move_through<Direction::South>(pipe);

                    default: std::unreachable();
                }
            } break;

            default:
                std::unreachable();
        }
    }

    constexpr auto start_info() const {
        struct StartInfo {
            const Pipe &start;

            Direction first_direction;
            Direction second_direction;
        };

        const auto &start = this->start();

        switch (start) {
            case Pipe::NorthSouth: return StartInfo{start, Direction::North, Direction::South};
            case Pipe::EastWest:   return StartInfo{start, Direction::East,  Direction::West};
            case Pipe::NorthEast:  return StartInfo{start, Direction::North, Direction::East};
            case Pipe::NorthWest:  return StartInfo{start, Direction::North, Direction::West};
            case Pipe::SouthWest:  return StartInfo{start, Direction::South, Direction::West};
            case Pipe::SouthEast:  return StartInfo{start, Direction::South, Direction::East};

            default: std::unreachable();
        }
    }
};

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t steps_to_get_furthest_from_start(Rng &&rng) {
    const auto pipes = Pipes(std::forward<Rng>(rng));

    const auto [start, first_direction, second_direction] = pipes.start_info();

    auto [first_pipe,  first_enter]  = pipes.follow(start, first_direction);
    auto [second_pipe, second_enter] = pipes.follow(start, second_direction);

    auto first_pipe_ptr  = &first_pipe;
    auto second_pipe_ptr = &second_pipe;

    std::size_t steps = 1;
    while (true) {
        if (first_pipe_ptr == second_pipe_ptr) {
            return steps;
        }

        const auto [next_first_pipe,  next_first_enter]  = pipes.follow(*first_pipe_ptr, first_enter);
        const auto [next_second_pipe, next_second_enter] = pipes.follow(*second_pipe_ptr, second_enter);

        first_pipe_ptr  = &next_first_pipe;
        second_pipe_ptr = &next_second_pipe;

        first_enter  = next_first_enter;
        second_enter = next_second_enter;

        ++steps;
    }
}

constexpr std::size_t steps_to_get_furthest_from_start_from_string_data(const std::string_view data) {
    return steps_to_get_furthest_from_start(data | advent::views::split_lines);
}

constexpr inline std::string_view simple_example_data = (
    ".....\n"
    ".S-7.\n"
    ".|.|.\n"
    ".L-J.\n"
    ".....\n"
);

static_assert(steps_to_get_furthest_from_start_from_string_data(simple_example_data) == 4);

constexpr inline std::string_view complex_example_data = (
    "..F7.\n"
    ".FJ|.\n"
    "SJ.L7\n"
    "|F--J\n"
    "LJ...\n"
);

static_assert(steps_to_get_furthest_from_start_from_string_data(complex_example_data) == 8);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        steps_to_get_furthest_from_start_from_string_data
    );
}
