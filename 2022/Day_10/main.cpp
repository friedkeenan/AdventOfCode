#include <advent/advent.hpp>

struct Display {
    static constexpr std::int64_t Width  = 40;
    static constexpr std::int64_t Height = 6;

    static_assert(Width <= 64);

    static constexpr std::array<std::int64_t, 2> PosToCoords(const std::int64_t pos) {
        return {
            pos % Width,
            pos / Width
        };
    }

    std::array<std::uint64_t, Height> _pixel_storage = {};

    static constexpr std::uint64_t _x_bit(const std::int64_t x) {
        return std::uint64_t{1} << x;
    }

    constexpr void turn_on(const std::int64_t x, const std::int64_t y) {
        this->_pixel_storage[y] |= _x_bit(x);
    }

    static constexpr bool _on_in_row(const std::uint64_t row, const std::int64_t x) {
        return (row & _x_bit(x)) != 0;
    }

    constexpr std::string render() const {
        std::string result;

        /* A character for each pixel, plus newlines for each row. */
        result.reserve(Width * Height + Height);

        for (const auto row : this->_pixel_storage) {
            for (const auto x : std::views::iota(std::int64_t{}, Width)) {
                if (_on_in_row(row, x)) {
                    result += '#';
                } else {
                    result += '.';
                }
            }

            result += '\n';
        }

        return result;
    }
};

static_assert(Display::PosToCoords(1) == std::array<std::int64_t, 2>{1, 0});

consteval bool test_display() {
    Display display;

    display.turn_on(20, 2);

    return (
        display.render() ==

        "........................................\n"
        "........................................\n"
        "....................#...................\n"
        "........................................\n"
        "........................................\n"
        "........................................\n"
    );
}

static_assert(test_display());

/* Forward declare. */
struct CPU;

template<typename T>
concept Instruction = requires(CPU &cpu, const std::string_view line) {
    { T::CycleDuration }      -> std::same_as<const std::int64_t &>;
    { T::Compare(line) }      -> std::convertible_to<bool>;
    { T::Execute(cpu, line) } -> std::same_as<void>;
};

template<Instruction... Instructions>
struct InstructionHandler;

template<typename Head, typename... Tail>
struct InstructionHandler<Head, Tail...> {
    /* We avoid directly naming 'CPU' because it is still incomplete. */
    template<std::invocable<const CPU &> Peeker>
    static constexpr std::int64_t ExecuteWithCyclePeeker(std::same_as<CPU> auto &cpu, const std::string_view line, const std::int64_t desired_cycle, Peeker &&peeker) {
        [[assume(cpu._cycle <= desired_cycle)]];

        if (Head::Compare(line)) {
            if (cpu._cycle + Head::CycleDuration > desired_cycle) {
                /*
                    If executing this instruction would advance the cycle
                    to our desired cycle or beyond, peek before the execution.
                */
                std::invoke(std::forward<Peeker>(peeker), std::as_const(cpu));
            }

            Head::Execute(cpu, line);

            return Head::CycleDuration;
        }

        return InstructionHandler<Tail...>::ExecuteWithCyclePeeker(cpu, line, desired_cycle, std::forward<Peeker>(peeker));
    }

    static constexpr std::int64_t ExecuteWithDisplay(std::same_as<CPU> auto &cpu, Display &display, const std::string_view line) {
        if (Head::Compare(line)) {
            for (const auto pos : std::views::iota(cpu._cycle, cpu._cycle + Head::CycleDuration)) {
                const auto [x, y] = Display::PosToCoords(pos);

                if (cpu.sprite_on_pixel(x)) {
                    display.turn_on(x, y);
                }
            }

            Head::Execute(cpu, line);

            return Head::CycleDuration;
        }

        return InstructionHandler<Tail...>::ExecuteWithDisplay(cpu, display, line);
    }
};

template<>
struct InstructionHandler<> {
    static constexpr std::int64_t ExecuteWithCyclePeeker(CPU &, const std::string_view, const std::int64_t, auto &&) {
        /* Stub implementation. */

        return 0;
    }

    static constexpr std::int64_t ExecuteWithDisplay(CPU &, Display &, const std::string_view) {
        /* Stub implementation. */

        return 0;
    }
};

struct NoOp {
    static constexpr std::int64_t CycleDuration = 1;

    static constexpr bool Compare(const std::string_view line) {
        /*
            We could just check equality but that might be
            worse because then it needs to check the size.
            This is in line with 'AddX' anyhow.
        */
        return line.starts_with("noop");
    }

    static constexpr void Execute(CPU &, const std::string_view) {
        /* NoOp does nothing. */
    }
};

static_assert(Instruction<NoOp>);

struct AddX {
    static constexpr std::int64_t CycleDuration = 2;

    static constexpr bool Compare(const std::string_view line) {
        return line.starts_with("addx");
    }

    /* This is a template because 'CPU' is still incomplete. */
    static constexpr void Execute(std::same_as<CPU> auto &cpu, std::string_view line) {
        const auto space_pos = line.find_first_of(' ');
        [[assume(space_pos != std::string_view::npos)]];

        line.remove_prefix(space_pos + 1);

        const auto amount = advent::to_integral<std::int64_t>(line);
        cpu.X += amount;
    }
};

static_assert(Instruction<AddX>);

struct CPU {
    using Handler = InstructionHandler<NoOp, AddX>;

    std::int64_t _cycle = 0;
    std::int64_t X      = 1;

    constexpr bool sprite_on_pixel(const std::int64_t x) const {
        if (this->X == x) {
            return true;
        }

        if (this->X != 0 && this->X - 1 == x) {
            return true;
        }

        if ((this->X + 1) != Display::Width && this->X + 1 == x) {
            return true;
        }

        return false;
    }

    template<std::invocable<const CPU &> Peeker>
    constexpr void execute_with_cycle_peeker(const std::string_view line, const std::int64_t desired_cycle, Peeker &&peeker) {
        this->_cycle += Handler::ExecuteWithCyclePeeker(*this, line, desired_cycle, std::forward<Peeker>(peeker));
    }

    constexpr void execute_with_display(Display &display, const std::string_view line) {
        this->_cycle += Handler::ExecuteWithDisplay(*this, display, line);
    }
};

template<std::int64_t CycleStart, std::int64_t CycleStep, std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::int64_t sum_signal_strengths(Rng &&asm_lines) {
    CPU cpu;

    std::int64_t signal_strength_sum = 0;

    /* 'CycleStart' is 1-indexed. */
    auto desired_cycle = CycleStart - 1;
    for (const std::string_view line : std::forward<Rng>(asm_lines)) {
        if (line.empty()) {
            continue;
        }

        cpu.execute_with_cycle_peeker(line, desired_cycle, [&](const CPU &cpu) {
            signal_strength_sum += cpu.X * (desired_cycle + 1);

            desired_cycle += CycleStep;
        });
    }

    return signal_strength_sum;
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::string render_display(Rng &&asm_lines) {
    /*
        NOTE: We return the rendered display, but part two
        wants capital letters shown in the display. If I knew
        what each letter looked like in the display, we could
        parse them out, and that does feel *more* in the spirit
        of Advent of Code, to go from an input to something you
        can just paste in, but I don't know what each letter
        looks like, and I think ultimately it's okay that we have
        to make the user interpret the display.
    */

    CPU cpu;
    Display display;

    for (const std::string_view line : std::forward<Rng>(asm_lines)) {
        if (line.empty()) {
            continue;
        }

        cpu.execute_with_display(display, line);
    }

    return display.render();
}

template<std::int64_t CycleStart, std::int64_t CycleStep>
constexpr std::int64_t sum_signal_strengths_from_string_data(const std::string_view data) {
    return sum_signal_strengths<CycleStart, CycleStep>(data | advent::views::split_lines);
}

constexpr std::string render_display_from_string_data(const std::string_view data) {
    return render_display(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "addx 15\n"
    "addx -11\n"
    "addx 6\n"
    "addx -3\n"
    "addx 5\n"
    "addx -1\n"
    "addx -8\n"
    "addx 13\n"
    "addx 4\n"
    "noop\n"
    "addx -1\n"
    "addx 5\n"
    "addx -1\n"
    "addx 5\n"
    "addx -1\n"
    "addx 5\n"
    "addx -1\n"
    "addx 5\n"
    "addx -1\n"
    "addx -35\n"
    "addx 1\n"
    "addx 24\n"
    "addx -19\n"
    "addx 1\n"
    "addx 16\n"
    "addx -11\n"
    "noop\n"
    "noop\n"
    "addx 21\n"
    "addx -15\n"
    "noop\n"
    "noop\n"
    "addx -3\n"
    "addx 9\n"
    "addx 1\n"
    "addx -3\n"
    "addx 8\n"
    "addx 1\n"
    "addx 5\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "addx -36\n"
    "noop\n"
    "addx 1\n"
    "addx 7\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "addx 2\n"
    "addx 6\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "addx 1\n"
    "noop\n"
    "noop\n"
    "addx 7\n"
    "addx 1\n"
    "noop\n"
    "addx -13\n"
    "addx 13\n"
    "addx 7\n"
    "noop\n"
    "addx 1\n"
    "addx -33\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "addx 2\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "addx 8\n"
    "noop\n"
    "addx -1\n"
    "addx 2\n"
    "addx 1\n"
    "noop\n"
    "addx 17\n"
    "addx -9\n"
    "addx 1\n"
    "addx 1\n"
    "addx -3\n"
    "addx 11\n"
    "noop\n"
    "noop\n"
    "addx 1\n"
    "noop\n"
    "addx 1\n"
    "noop\n"
    "noop\n"
    "addx -13\n"
    "addx -19\n"
    "addx 1\n"
    "addx 3\n"
    "addx 26\n"
    "addx -30\n"
    "addx 12\n"
    "addx -1\n"
    "addx 3\n"
    "addx 1\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "addx -9\n"
    "addx 18\n"
    "addx 1\n"
    "addx 2\n"
    "noop\n"
    "noop\n"
    "addx 9\n"
    "noop\n"
    "noop\n"
    "noop\n"
    "addx -1\n"
    "addx 2\n"
    "addx -37\n"
    "addx 1\n"
    "addx 3\n"
    "noop\n"
    "addx 15\n"
    "addx -21\n"
    "addx 22\n"
    "addx -6\n"
    "addx 1\n"
    "noop\n"
    "addx 2\n"
    "addx 1\n"
    "noop\n"
    "addx -10\n"
    "noop\n"
    "noop\n"
    "addx 20\n"
    "addx 1\n"
    "addx 2\n"
    "addx 2\n"
    "addx -6\n"
    "addx -11\n"
    "noop\n"
    "noop\n"
    "noop\n"
);

static_assert(sum_signal_strengths_from_string_data<20, 40>(example_data) == 13140);

static_assert(
    render_display_from_string_data(example_data) ==

    "##..##..##..##..##..##..##..##..##..##..\n"
    "###...###...###...###...###...###...###.\n"
    "####....####....####....####....####....\n"
    "#####.....#####.....#####.....#####.....\n"
    "######......######......######......####\n"
    "#######.......#######.......#######.....\n"
);

int main(int argc, char **argv) {
    /* NOTE: Part two doesn't warrant use of 'advent::solve_puzzles' because of its non-typical result. */

    const auto data = advent::puzzle_data(argc, argv);
    if (!data.has_value()) {
        advent::print("Unable to read puzzle data!\n");

        return 1;
    }

    const auto part_one_solution = sum_signal_strengths_from_string_data<20, 40>(*data);
    const auto part_two_solution = render_display_from_string_data(*data);

    fmt::print("Part one solution: {}\n", part_one_solution);
    fmt::print("Part two solution:\n{}", part_two_solution);
}
