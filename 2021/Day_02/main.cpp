#include <advent/advent.hpp>

enum class Direction : std::uint8_t {
    Forward,
    Down,
    Up,
};

struct Command {
    using Value = std::int64_t;

    Direction direction;
    Value     value;

    static constexpr Direction ParseDirection(const std::string_view str) {
        if (str == "forward") {
            return Direction::Forward;
        }

        if (str == "down") {
            return Direction::Down;
        }

        if (str == "up") {
            return Direction::Up;
        }

        std::unreachable();
    }

    static constexpr Command Parse(const std::string_view command) {
        const auto space_separator = command.find(' ');

        /* There must be a separator. */
        [[assume(space_separator != std::string_view::npos)]];

        const auto direction = Command::ParseDirection(command.substr(0, space_separator));
        const auto value     = advent::to_integral<Value>(command.substr(space_separator + 1));

        return Command{direction, value};
    }

    constexpr bool operator ==(const Command &) const = default;
};

static_assert(Command::Parse("forward 5") == Command{Direction::Forward, 5});

template<typename Child>
struct SubmarineImpl {
    Command::Value horizontal_distance = 0;
    Command::Value depth               = 0;

    constexpr Child &_child() {
        static_assert(std::derived_from<Child, SubmarineImpl>);

        return static_cast<Child &>(*this);
    }

    constexpr const Child &_child() const {
        static_assert(std::derived_from<Child, SubmarineImpl>);

        return static_cast<const Child &>(*this);
    }

    constexpr void perform_commands(const std::string_view commands_data) {
        auto parsed_commands =
            commands_data |

            advent::views::split_lines |

            std::views::filter([](const std::string_view command) {
                return command.length() > 0;
            }) |

            std::views::transform(Command::Parse);

        for (const auto command : parsed_commands) {
            this->_child().perform_single_command(command);
        }
    }
};

struct FaultySubmarine : SubmarineImpl<FaultySubmarine> {
    constexpr void perform_single_command(const Command command) {
        switch (command.direction) {
            case Direction::Forward: {
                this->horizontal_distance += command.value;
            } break;

            case Direction::Down: {
                this->depth += command.value;
            } break;

            case Direction::Up: {
                this->depth -= command.value;
            } break;

            default: std::unreachable();
        }
    }
};

struct ProperSubmarine : SubmarineImpl<ProperSubmarine> {
    Command::Value aim = 0;

    constexpr void perform_single_command(const Command command) {
        switch (command.direction) {
            case Direction::Forward: {
                this->horizontal_distance += command.value;
                this->depth               += command.value * this->aim;
            } break;

            case Direction::Down: {
                this->aim += command.value;
            } break;

            case Direction::Up: {
                this->aim -= command.value;
            } break;

            default: std::unreachable();
        }
    }
};

template<typename Submarine>
constexpr Command::Value test_submarine(const std::string_view commands_data) {
    auto submarine = Submarine{};

    submarine.perform_commands(commands_data);

    return submarine.horizontal_distance * submarine.depth;
}

constexpr inline std::string_view example_data = (
    "forward 5\n"
    "down 5\n"
    "forward 8\n"
    "up 3\n"
    "down 8\n"
    "forward 2\n"
);

static_assert(test_submarine<FaultySubmarine>(example_data) == 150);
static_assert(test_submarine<ProperSubmarine>(example_data) == 900);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        test_submarine<FaultySubmarine>,
        test_submarine<ProperSubmarine>
    );
}
