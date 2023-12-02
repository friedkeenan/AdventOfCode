#include <advent/advent.hpp>

struct Game {
    /*
        NOTE: We don't need to track each set separately
        for either part (we could just track the maximum
        of each color for both parts), but I find this
        abstraction pleasing and more readable, even if
        it's not as "clever".
    */
    struct CubeSet {
        std::size_t red   = 0;
        std::size_t green = 0;
        std::size_t blue  = 0;

        constexpr bool impossible() const {
            static constexpr std::size_t MaxPossibleRed   = 12;
            static constexpr std::size_t MaxPossibleGreen = 13;
            static constexpr std::size_t MaxPossibleBlue  = 14;

            return (
                this->red   > MaxPossibleRed   ||
                this->green > MaxPossibleGreen ||
                this->blue  > MaxPossibleBlue
            );
        }

        constexpr std::size_t power() const {
            return this->red * this->green * this->blue;
        }
    };

    std::size_t id;
    std::vector<CubeSet> cube_sets;

    constexpr explicit Game(std::string_view description) {
        const auto id_start = description.find_first_of(' ');
        [[assume(id_start != std::string_view::npos)]];

        const auto id_end = description.find_first_of(':', id_start);
        [[assume(id_end != std::string_view::npos)]];

        this->id = advent::to_integral<std::size_t>(
            /* Move forward past the space and shrink the size to not include the colon. */
            description.substr(id_start + 1, id_end - id_start - 1)
        );

        /* Move past the space trailing after the colon as well. */
        description.remove_prefix(id_end + 1 + 1);

        advent::split_with_callback(description, "; ", [&](const std::string_view set_description) {
            auto &cube_set = this->cube_sets.emplace_back();

            advent::split_with_callback(set_description, ", ", [&](std::string_view cube_description) {
                const auto count_end = cube_description.find_first_of(' ');
                [[assume(count_end != std::string_view::npos)]];

                const auto count = advent::to_integral<std::size_t>(cube_description.substr(0, count_end));

                cube_description.remove_prefix(count_end + 1);

                if (cube_description.starts_with("red")) {
                    cube_set.red = count;
                } else if (cube_description.starts_with("green")) {
                    cube_set.green = count;
                } else {
                    [[assume(cube_description.starts_with("blue"))]];

                    cube_set.blue = count;
                }
            });
        });
    }

    constexpr bool possible() const {
        for (const auto &cube_set : this->cube_sets) {
            if (cube_set.impossible()) {
                return false;
            }
        }

        return true;
    }

    constexpr CubeSet minimum_needed_set() const {
        auto current_maximum = CubeSet{};

        for (const auto &cube_set : this->cube_sets) {
            if (cube_set.red > current_maximum.red) {
                current_maximum.red = cube_set.red;
            }

            if (cube_set.green > current_maximum.green) {
                current_maximum.green = cube_set.green;
            }

            if (cube_set.blue > current_maximum.blue) {
                current_maximum.blue = cube_set.blue;
            }
        }

        return current_maximum;
    }
};

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t sum_possible_game_ids(Rng &&rng) {
    std::size_t sum = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        const auto game = Game(line);

        if (game.possible()) {
            sum += game.id;
        }
    }

    return sum;
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t sum_minimum_needed_set_powers(Rng &&rng) {
    std::size_t sum = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        const auto game = Game(line);

        const auto minimum_set = game.minimum_needed_set();

        sum += minimum_set.power();
    }

    return sum;
}

constexpr std::size_t sum_possible_game_ids_from_string_data(const std::string_view data) {
    return sum_possible_game_ids(data | advent::views::split_lines);
}

constexpr std::size_t sum_minimum_needed_set_powers_from_string_data(const std::string_view data) {
    return sum_minimum_needed_set_powers(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green\n"
    "Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue\n"
    "Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red\n"
    "Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red\n"
    "Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green\n"
);

static_assert(sum_possible_game_ids_from_string_data(example_data) == 8);
static_assert(sum_minimum_needed_set_powers_from_string_data(example_data) == 2286);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_possible_game_ids_from_string_data,
        sum_minimum_needed_set_powers_from_string_data
    );
}
