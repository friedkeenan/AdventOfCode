#include <advent/advent.hpp>

enum class Direction : char {
    Right = 'R',
    Left  = 'L',
    Down  = 'D',
    Up    = 'U',
};

class Motion {
    public:
        Direction   direction;
        std::size_t amount;

        static constexpr Motion Parse(std::string_view description) {
            const auto direction = Direction{description[0]};

            /* Remove direction and space. */
            description.remove_prefix(1 + 1);

            const auto amount = advent::to_integral<std::size_t>(description);

            return Motion{direction, amount};
        }

        constexpr bool operator ==(const Motion &) const noexcept = default;
};

static_assert(Motion::Parse("R 45") == Motion{Direction::Right, 45});

template<std::size_t Length>
requires (Length >= 1)
class Rope {
    public:
        using Position = advent::vector_2d<advent::ssize_t>;

        std::array<Position, Length> _knots = {};

        constexpr Position back() const {
            return this->_knots.back();
        }

        constexpr void _move_head(const Direction direction) {
            switch (direction) {
                case Direction::Right: {
                    this->_knots[0].x() += 1;
                } return;

                case Direction::Left: {
                    this->_knots[0].x() -= 1;
                } return;

                case Direction::Down: {
                    this->_knots[0].y() -= 1;
                } return;

                case Direction::Up: {
                    this->_knots[0].y() += 1;
                } return;

                default: std::unreachable();
            }
        }

        static constexpr void _move_tail(Position &tail, const Position leader) {
            const auto x_dist = advent::abs(tail.x() - leader.x());
            const auto y_dist = advent::abs(tail.y() - leader.y());

            if (x_dist < 2 && y_dist < 2) {
                return;
            }

            const auto axis = [&]() {
                if (x_dist > y_dist) {
                    return 0;
                }

                return 1;
            }();

            const auto increment = [&]() {
                if (leader[axis] > tail[axis]) {
                    return 1;
                }

                return -1;
            }();

            tail[axis] += increment;

            const auto opposite_axis = (axis == 0 ? 1 : 0);

            if (tail[opposite_axis] == leader[opposite_axis]) {
                return;
            }


            if (tail[opposite_axis] < leader[opposite_axis]) {
                tail[opposite_axis] += 1;
            } else {
                tail[opposite_axis] -= 1;
            }
        }

        constexpr void move(const Direction direction) {
            this->_move_head(direction);

            for (const auto i : std::views::iota(1uz, Length)) {
                _move_tail(this->_knots[i], this->_knots[i - 1]);
            }
        }
};

template<std::size_t Length, std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t count_unique_end_positions(Rng &&motion_descriptions) {
    Rope<Length> rope;

    /*
        There is probably a more efficient way to do this than a
        vector of positions which we manually check for duplicates.
    */
    auto end_positions = std::vector{rope.back()};
    for (const std::string_view description : std::forward<Rng>(motion_descriptions)) {
        if (description.empty()) {
            continue;
        }

        const auto motion = Motion::Parse(description);

        for ([[maybe_unused]] const auto i : std::views::iota(0uz, motion.amount)) {
            rope.move(motion.direction);
            const auto end_pos = rope.back();

            /* We do not have an implementation of 'std::ranges::contains'. */
            if (std::ranges::find(end_positions, end_pos) == end_positions.end()) {
                end_positions.push_back(end_pos);
            }
        }
    }

    return end_positions.size();
}

template<std::size_t Length>
constexpr std::size_t count_unique_end_positions_from_string_data(const std::string_view data) {
    return count_unique_end_positions<Length>(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "R 4\n"
    "U 4\n"
    "L 3\n"
    "D 1\n"
    "R 4\n"
    "D 1\n"
    "L 5\n"
    "R 2\n"
);

static_assert(count_unique_end_positions_from_string_data<2>(example_data) == 13);
static_assert(count_unique_end_positions_from_string_data<10>(example_data) == 1);

constexpr inline std::string_view spiral_example_data = (
    "R 5\n"
    "U 8\n"
    "L 8\n"
    "D 3\n"
    "R 17\n"
    "D 10\n"
    "L 25\n"
    "U 20\n"
);

static_assert(count_unique_end_positions_from_string_data<10>(spiral_example_data) == 36);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_unique_end_positions_from_string_data<2>,
        count_unique_end_positions_from_string_data<10>
    );
}
