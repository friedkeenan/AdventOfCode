#include <advent/advent.hpp>

struct AntennaMap {
    static constexpr char NullFrequency = '.';

    advent::string_view_grid frequencies;

    constexpr explicit AntennaMap(const std::string_view description) : frequencies(description) {}

    template<typename AntinodesMarker>
    requires (
        std::invocable<
            AntinodesMarker &,

            advent::grid<bool> &,
            advent::vector_2d<std::size_t>,
            advent::vector_2d<std::size_t>
        >
    )
    constexpr std::size_t _count_antinodes(this const AntennaMap &self, AntinodesMarker marker) {
        auto antinodes = advent::grid<bool>::from_dimensions(self.frequencies.width(), self.frequencies.height());

        const auto elements = self.frequencies.elements();
        for (const auto first_it : std::views::iota(elements.begin(), elements.end())) {
            const auto &first_frequency = *first_it;

            if (first_frequency == NullFrequency) {
                continue;
            }

            /* Any previous antennas would have already processed us. */
            for (const auto &second_frequency : std::ranges::subrange(first_it + 1, elements.end())) {
                /* NOTE: This will cover if 'second_frequency' is a 'NullFrequency'. */
                if (second_frequency != first_frequency) {
                    continue;
                }

                std::invoke(
                    marker,

                    antinodes,
                    self.frequencies.coords_of(&first_frequency),
                    self.frequencies.coords_of(&second_frequency)
                );
            }
        }

        return std::ranges::count_if(antinodes.elements(), std::identity{});
    }

    constexpr std::size_t count_nonharmonic_antinodes(this const AntennaMap &self) {
        return self._count_antinodes([](auto &antinodes, const auto first_coords, const auto second_coords) {
            const auto offset = second_coords - first_coords;

            const auto first_antinode = first_coords - offset;
            if (antinodes.contains_coords(first_antinode)) {
                antinodes[first_antinode] = true;
            }

            const auto second_antinode = second_coords + offset;
            if (antinodes.contains_coords(second_antinode)) {
                antinodes[second_antinode] = true;
            }
        });
    }

    constexpr std::size_t count_harmonic_antinodes(this const AntennaMap &self) {
        return self._count_antinodes([](auto &antinodes, const auto first_coords, const auto second_coords) {
            const auto offset = second_coords - first_coords;

            for (auto coords = first_coords; antinodes.contains_coords(coords); coords -= offset) {
                antinodes[coords] = true;
            }

            for (auto coords = second_coords; antinodes.contains_coords(coords); coords += offset) {
                antinodes[coords] = true;
            }
        });
    }
};

constexpr std::size_t count_nonharmonic_antinodes(const std::string_view data) {
    return AntennaMap(data).count_nonharmonic_antinodes();
}

constexpr std::size_t count_harmonic_antinodes(const std::string_view data) {
    return AntennaMap(data).count_harmonic_antinodes();
}

constexpr inline std::string_view example_data = (
    "............\n"
    "........0...\n"
    ".....0......\n"
    ".......0....\n"
    "....0.......\n"
    "......A.....\n"
    "............\n"
    "............\n"
    "........A...\n"
    ".........A..\n"
    "............\n"
    "............\n"
);

static_assert(count_nonharmonic_antinodes(example_data) == 14);
static_assert(count_harmonic_antinodes(example_data) == 34);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_nonharmonic_antinodes,
        count_harmonic_antinodes
    );
}
