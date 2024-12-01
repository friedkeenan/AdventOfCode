#include <advent/advent.hpp>

struct Map {
    struct SourceRange {
        std::size_t start;
        std::size_t size;

        constexpr std::size_t end() const {
            return this->start + this->size;
        }
    };

    struct KeyRange {
        std::size_t destination_start;
        std::size_t source_start;
        std::size_t size;

        constexpr std::size_t source_end() const {
            return this->source_start + this->size;
        }

        constexpr bool contains(const std::size_t source) const {
            return source >= this->source_start && source < this->source_end();
        }

        constexpr std::size_t convert(const std::size_t source) const {
            [[assume(source >= this->source_start)]];

            return this->destination_start + (source - this->source_start);
        }
    };

    std::vector<KeyRange> ranges;

    template<advent::string_viewable_iterator It>
    static constexpr Map ParseAndAdvanceIterator(It &it) {
        /* We ignore the map's category line. */
        ++it;

        const auto parse_ranges = [&]() {
            std::vector<KeyRange> ranges;

            while (true) {
                const std::string_view range_description = *it;

                /* NOTE: We advance past the ending empty line as well. */
                ++it;

                if (range_description.empty()) {
                    return ranges;
                }

                const auto first_end = range_description.find_first_of(' ');
                [[assume(first_end != std::string_view::npos)]];

                const auto second_end = range_description.find_first_of(' ', first_end + 1);
                [[assume(second_end != std::string_view::npos)]];

                const auto destination_start = advent::to_integral<std::size_t>(range_description.substr(0, first_end));
                const auto source_start      = advent::to_integral<std::size_t>(range_description.substr(first_end + 1, second_end - first_end - 1));
                const auto size              = advent::to_integral<std::size_t>(range_description.substr(second_end + 1));

                ranges.emplace_back(destination_start, source_start, size);
            }
        };

        return Map{parse_ranges()};
    }

    constexpr std::size_t convert(const std::size_t source) const {
        for (const auto &range : this->ranges) {
            if (range.contains(source)) {
                return range.convert(source);
            }
        }

        return source;
    }
};

constexpr inline std::string_view SeedsPrefix = "seeds: ";

template<advent::string_viewable_range Rng>
constexpr std::size_t minimum_location_of_seeds(Rng &&rng) {
    auto it = std::ranges::begin(rng);

    const std::string_view seeds_description = *it;
    [[assume(seeds_description.starts_with(SeedsPrefix))]];

    std::vector<std::size_t> sources;
    advent::split_with_callback(seeds_description.substr(SeedsPrefix.size()), ' ', [&](const std::string_view seed) {
        sources.push_back(advent::to_integral<std::size_t>(seed));
    });

    /* Move past the first line and the following empty line. */
    ++it;
    ++it;

    while (it != std::ranges::end(rng)) {
        const auto map = Map::ParseAndAdvanceIterator(it);

        for (auto &source : sources) {
            source = map.convert(source);
        }
    }

    return std::ranges::min(sources);
}

template<advent::string_viewable_range Rng>
constexpr std::size_t minimum_location_of_seed_ranges(Rng &&rng) {
    auto it = std::ranges::begin(rng);

    std::string_view seeds_description = *it;
    [[assume(seeds_description.starts_with(SeedsPrefix))]];

    seeds_description.remove_prefix(SeedsPrefix.size());

    std::vector<Map::SourceRange> source_ranges;
    while (true) {
        const auto first_end = seeds_description.find_first_of(' ');
        [[assume(first_end != std::string_view::npos)]];

        const auto second_end = seeds_description.find_first_of(' ', first_end + 1);

        const auto start = advent::to_integral<std::size_t>(seeds_description.substr(0, first_end));
        const auto size  = advent::to_integral<std::size_t>(seeds_description.substr(first_end + 1, second_end - first_end - 1));

        source_ranges.emplace_back(start, size);

        if (second_end == std::string_view::npos) {
            break;
        }

        seeds_description.remove_prefix(second_end + 1);
    }

    /* Move past the first line and the following empty line. */
    ++it;
    ++it;

    const auto maps = [&]() {
        std::vector<Map> maps;

        while (it != std::ranges::end(rng)) {
            maps.push_back(Map::ParseAndAdvanceIterator(it));
        }

        return maps;
    }();

    /*
        NOTE: There is a better way to do this.
        I do not have the space in my brain today for it.
        Forgive me.
    */

    auto location_minimum = std::numeric_limits<std::size_t>::max();
    for (const auto &source_range : source_ranges) {
        for (auto source : std::views::iota(source_range.start, source_range.end())) {
            for (const auto &map : maps) {
                source = map.convert(source);
            }

            if (source < location_minimum) {
                location_minimum = source;
            }
        }
    }

    return location_minimum;
}

constexpr std::size_t minimum_location_of_seeds_from_string_data(const std::string_view data) {
    return minimum_location_of_seeds(data | advent::views::split_lines);
}

constexpr std::size_t minimum_location_of_seed_ranges_from_string_data(const std::string_view data) {
    return minimum_location_of_seed_ranges(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "seeds: 79 14 55 13\n"
    "\n"
    "seed-to-soil map:\n"
    "50 98 2\n"
    "52 50 48\n"
    "\n"
    "soil-to-fertilizer map:\n"
    "0 15 37\n"
    "37 52 2\n"
    "39 0 15\n"
    "\n"
    "fertilizer-to-water map:\n"
    "49 53 8\n"
    "0 11 42\n"
    "42 0 7\n"
    "57 7 4\n"
    "\n"
    "water-to-light map:\n"
    "88 18 7\n"
    "18 25 70\n"
    "\n"
    "light-to-temperature map:\n"
    "45 77 23\n"
    "81 45 19\n"
    "68 64 13\n"
    "\n"
    "temperature-to-humidity map:\n"
    "0 69 1\n"
    "1 0 69\n"
    "\n"
    "humidity-to-location map:\n"
    "60 56 37\n"
    "56 93 4\n"
);

static_assert(minimum_location_of_seeds_from_string_data(example_data) == 35);
static_assert(minimum_location_of_seed_ranges_from_string_data(example_data) == 46);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        minimum_location_of_seeds_from_string_data,
        minimum_location_of_seed_ranges_from_string_data
    );
}
