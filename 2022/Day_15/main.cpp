#include <advent/advent.hpp>

using Coord    = std::int64_t;
using Position = advent::vector_2d<Coord>;

class CoordRange {
    public:
        Coord start;
        Coord end;

        constexpr Coord distance() const {
            return end - start + 1;
        }

        constexpr CoordRange clamp(const Coord start, const Coord end) const {
            return CoordRange{
                std::max(start, this->start),
                std::min(end,   this->end)
            };
        }

        constexpr bool combinable(const CoordRange other) const {
            return (
                (this->start - 1) <= other.end   &&
                (this->end   + 1) >= other.start
            );
        }

        constexpr CoordRange combine(const CoordRange other) const {
            return CoordRange{
                std::min(this->start, other.start),
                std::max(this->end,   other.end)
            };
        }
};

static_assert(CoordRange{4, 5}.combinable(CoordRange{5, 6}));

class SensorRegion {
    public:
        static constexpr std::string_view SensorPositionPrefix = "Sensor at x=";
        static constexpr std::string_view YCoordPrefix         = ", y=";
        static constexpr std::string_view BeaconPositionPrefix = ": closest beacon is at x=";

        Position sensor;
        Position beacon;

        constexpr SensorRegion(std::string_view description) {
            description.remove_prefix(SensorPositionPrefix.length());

            const auto parse_pos = [&]() {
                const auto comma_pos = description.find_first_of(',');
                advent::assume(comma_pos != std::string_view::npos);

                const auto x = advent::to_integral<Coord>(std::string_view(description.data(), comma_pos));
                description.remove_prefix(comma_pos + YCoordPrefix.length());

                const auto colon_pos = description.find_first_of(':');
                if (colon_pos == std::string_view::npos) {
                    return Position{x, advent::to_integral<Coord>(description)};
                }

                const auto y = advent::to_integral<Coord>(std::string_view(description.data(), colon_pos));
                description.remove_prefix(colon_pos + BeaconPositionPrefix.length());

                return Position{x, y};
            };

            this->sensor = parse_pos();
            this->beacon = parse_pos();
        }

        constexpr Coord radius() const {
            return this->beacon.manhattan_distance(this->sensor);
        }

        constexpr std::optional<CoordRange> row_slice(const Coord row) const {
            const auto radius = this->radius();

            if (advent::abs(row - this->sensor.y()) > radius) {
                /* If the row is not within our region, do nothing. */

                return std::nullopt;
            }

            const auto half_width = radius - advent::abs(row - this->sensor.y());

            return CoordRange{
                this->sensor.x() - half_width,
                this->sensor.x() + half_width
            };
        }
};

template<Coord Row, std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t num_non_beacons_in_row(Rng &&sensors) {
    std::vector<CoordRange> ranges;

    std::vector<Coord> beacons_in_row;

    for (const std::string_view description : std::forward<Rng>(sensors)) {
        if (description.empty()) {
            continue;
        }

        const auto region = SensorRegion(description);

        /* We do not have an implementation of 'std::ranges::contains'. */
        if (region.beacon.y() == Row && std::ranges::find(beacons_in_row, region.beacon.x()) == beacons_in_row.end()) {
            beacons_in_row.push_back(region.beacon.x());
        }

        const auto maybe_slice = region.row_slice(Row);
        if (!maybe_slice.has_value()) {
            continue;
        }

        ranges.push_back(*maybe_slice);
    }

    /* combine and count range distances. */
    std::size_t num_spaces_in_slice = 0;
    while (!ranges.empty()) {
        auto range = ranges.back();
        ranges.pop_back();

        bool combined = false;
        for (auto &other : ranges) {
            if (range.combinable(other)) {
                other = range.combine(other);

                combined = true;
                break;
            }
        }

        if (!combined) {
            num_spaces_in_slice += range.distance();
        }
    }

    return num_spaces_in_slice - beacons_in_row.size();
}

constexpr std::size_t tuning_frequency(const Position pos) {
    return 4'000'000 * static_cast<std::size_t>(pos.x()) + pos.y();
}

template<Coord Max, std::ranges::input_range Rng>
requires (Max > 0 && std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t find_tuning_frequency(Rng &&sensors) {
    /* Dear god forgive me for this code. */

    /* We don't have an implementation of 'std::ranges::to'. */
    const auto regions = [&]() {
        std::vector<SensorRegion> regions;

        for (const std::string_view description : std::forward<Rng>(sensors)) {
            if (description.empty()) {
                continue;
            }

            regions.emplace_back(description);
        }

        return regions;
    }();

    for (const auto row : std::views::iota(Coord{0}, Max + 1)) {
        CoordRange main;

        auto it = regions.begin();

        /* We do 'while (true)' because there must be a row with a valid slice at some point. */
        while (true) {
            const auto maybe_slice = it->row_slice(row);
            if (!maybe_slice.has_value()) {
                ++it;

                continue;
            }

            main = *maybe_slice;

            if (main.end < 0 || main.start > Max) {
                ++it;

                continue;
            }

            main = main.clamp(0, Max);

            break;
        }

        ++it;

        std::vector<CoordRange> others;

        while (it != regions.end()) {
            const auto maybe_slice = it->row_slice(row);
            if (!maybe_slice.has_value()) {
                ++it;

                continue;
            }

            auto slice = *maybe_slice;
            if (slice.end < 0 || slice.start > Max) {
                ++it;

                continue;
            }

            slice = slice.clamp(0, Max);

            if (main.combinable(slice)) {
                main = main.combine(slice);

                ++it;

                continue;
            }

            others.push_back(slice);

            ++it;
        }

        if (!others.empty()) {
            /* Try to combine all the ranges with 'main'. */
            while (true) {
                bool combined = false;

                auto i = others.size() - 1;
                while (true) {
                    if (main.combinable(others[i])) {

                        main = main.combine(others[i]);
                        others.erase(others.begin() + i);

                        combined = true;
                    }

                    if (i == 0) {
                        break;
                    }

                    --i;
                }

                if (!combined || others.empty()) {
                    break;
                }
            }
        }

        if (!others.empty()) {
            /* Combine the leftovers into one range. */

            auto separated = others.front();
            for (const auto i : std::views::iota(1uz, others.size())) {
                advent::assume(separated.combinable(others[i]));

                separated = separated.combine(others[i]);
            }

            if (separated.start > main.end) {
                return tuning_frequency(Position{main.end + 1, row});
            }

            return tuning_frequency(Position{main.start - 1, row});
        }

        if (main.distance() < Max) {
            /* We have one distinct range but it doesn't fill the whole row. */

            if (main.start == 0) {
                return tuning_frequency(Position{Max, row});
            }

            return tuning_frequency(Position{0, row});
        }
    }

    std::unreachable();
}

template<Coord Row>
constexpr std::size_t num_non_beacons_in_row_from_string_data(const std::string_view data) {
    return num_non_beacons_in_row<Row>(advent::views::split_lines(data));
}

template<Coord Max>
requires (Max > 0)
constexpr std::size_t find_tuning_frequency_from_string_data(const std::string_view data) {
    return find_tuning_frequency<Max>(advent::views::split_lines(data));
}

constexpr inline std::string_view example_data = (
    "Sensor at x=2, y=18: closest beacon is at x=-2, y=15\n"
    "Sensor at x=9, y=16: closest beacon is at x=10, y=16\n"
    "Sensor at x=13, y=2: closest beacon is at x=15, y=3\n"
    "Sensor at x=12, y=14: closest beacon is at x=10, y=16\n"
    "Sensor at x=10, y=20: closest beacon is at x=10, y=16\n"
    "Sensor at x=14, y=17: closest beacon is at x=10, y=16\n"
    "Sensor at x=8, y=7: closest beacon is at x=2, y=10\n"
    "Sensor at x=2, y=0: closest beacon is at x=2, y=10\n"
    "Sensor at x=0, y=11: closest beacon is at x=2, y=10\n"
    "Sensor at x=20, y=14: closest beacon is at x=25, y=17\n"
    "Sensor at x=17, y=20: closest beacon is at x=21, y=22\n"
    "Sensor at x=16, y=7: closest beacon is at x=15, y=3\n"
    "Sensor at x=14, y=3: closest beacon is at x=15, y=3\n"
    "Sensor at x=20, y=1: closest beacon is at x=15, y=3\n"
);

static_assert(num_non_beacons_in_row_from_string_data<10>(example_data) == 26);

/* Add this extra assert for a test case where the slices don't all overlap. */
static_assert(num_non_beacons_in_row_from_string_data<11>(example_data) == 28);

static_assert(find_tuning_frequency_from_string_data<20>(example_data) == 56000011);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        num_non_beacons_in_row_from_string_data<2'000'000>,
        find_tuning_frequency_from_string_data<4'000'000>
    );
}
