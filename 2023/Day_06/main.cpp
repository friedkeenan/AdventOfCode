#include <advent/advent.hpp>

struct RaceInfo {
    std::size_t duration;
    std::size_t record_distance;

    constexpr std::size_t num_ways_to_break_record() const {
        /*
            NOTE: Using integers makes this calculation a tad temperamental.

            This configuration seems to work for this problem.

            In essence we calculate the quasi-roots of the relevant
            quadratic and get the distance between them.

            I call them quasi-roots because they're not the actual
            roots, but rather close enough approximations that
            end up being exactly what we need.
        */

        const auto needed_distance = this->record_distance + 1;

        const auto discriminant = this->duration * this->duration - 4 * needed_distance;

        const auto processed_discriminant = advent::floor_sqrt(discriminant);

        const auto high = (this->duration + processed_discriminant)      / 2;
        const auto low  = (this->duration - processed_discriminant - 1)  / 2;

        return high - low;
    }
};

static_assert(RaceInfo{7,    9}.num_ways_to_break_record() == 4);
static_assert(RaceInfo{15,  40}.num_ways_to_break_record() == 8);
static_assert(RaceInfo{30, 200}.num_ways_to_break_record() == 9);

static_assert(RaceInfo{71530, 940200}.num_ways_to_break_record() == 71503);

/* NOTE: The example data and the real data have a different number of trailing spaces. */
constexpr inline std::string_view TimePrefix     = "Time:     ";
constexpr inline std::string_view DistancePrefix = "Distance: ";

template<advent::string_viewable_range Rng>
constexpr std::vector<RaceInfo> parse_separated_races(Rng &&rng) {
    std::vector<RaceInfo> races;

    auto it = std::ranges::begin(rng);

    std::string_view times_description = *it;

    [[assume(times_description.starts_with(TimePrefix))]];
    times_description.remove_prefix(TimePrefix.size());

    advent::split_with_callback(times_description, ' ', [&](const std::string_view time) {
        if (time.empty()) {
            return;
        }

        /* We will fill in the record distance later. */
        races.emplace_back(advent::to_integral<std::size_t>(time), 0);
    });

    ++it;

    std::string_view distances_description = *it;

    [[assume(distances_description.starts_with(DistancePrefix))]];
    distances_description.remove_prefix(DistancePrefix.size());

    std::size_t i = 0;
    advent::split_with_callback(distances_description, ' ', [&](const std::string_view distance) {
        if (distance.empty()) {
            return;
        }

        races[i].record_distance = advent::to_integral<std::size_t>(distance);

        ++i;
    });

    return races;
}

template<advent::string_viewable_range Rng>
constexpr std::size_t number_of_ways_to_beat_separated_races(Rng &&rng) {
    const auto races = parse_separated_races(std::forward<Rng>(rng));

    std::size_t num_ways = 1;

    for (const auto &race : races) {
        num_ways *= race.num_ways_to_break_record();
    }

    return num_ways;
}

template<advent::string_viewable_range Rng>
constexpr RaceInfo parse_single_race(Rng &&rng) {
    static constexpr auto is_digit_filter = std::views::filter([](char c) {
        return advent::is_digit(c, 10);
    });

    auto it = std::ranges::begin(rng);

    std::string_view times_description = *it;

    [[assume(times_description.starts_with(TimePrefix))]];
    times_description.remove_prefix(TimePrefix.size());

    const auto time = advent::to_integral<std::size_t>(times_description | is_digit_filter);

    ++it;

    std::string_view distances_description = *it;

    [[assume(distances_description.starts_with(DistancePrefix))]];
    distances_description.remove_prefix(DistancePrefix.size());

    const auto record_distance = advent::to_integral<std::size_t>(distances_description | is_digit_filter);

    return RaceInfo{time, record_distance};
}

template<advent::string_viewable_range Rng>
constexpr std::size_t number_of_ways_to_beat_single_race(Rng &&rng) {
    const auto race = parse_single_race(std::forward<Rng>(rng));

    return race.num_ways_to_break_record();
}

constexpr std::size_t number_of_ways_to_beat_separated_races_from_string_data(const std::string_view data) {
    return number_of_ways_to_beat_separated_races(data | advent::views::split_lines);
}

constexpr std::size_t number_of_ways_to_beat_single_race_from_string_data(const std::string_view data) {
    return number_of_ways_to_beat_single_race(data | advent::views::split_lines);
}

constexpr std::string_view example_data = (
    "Time:      7  15   30\n"
    "Distance:  9  40  200\n"
);

static_assert(number_of_ways_to_beat_separated_races_from_string_data(example_data) == 288);
static_assert(number_of_ways_to_beat_single_race_from_string_data(example_data) == 71503);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        number_of_ways_to_beat_separated_races_from_string_data,
        number_of_ways_to_beat_single_race_from_string_data
    );
}
