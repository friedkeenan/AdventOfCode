import std;
import advent;

template<bool Behind>
constexpr std::int64_t next_distance(const std::vector<std::int64_t> &upper_elems) {
    static constexpr auto Distance = [](auto &a, auto &b) {
        return b - a;
    };

    /*
        For some reason you can't do a pairwise transform
        on a pairwise transform, so we collect into a vector.
    */
    const auto distances = [&]() {
        std::vector<std::int64_t> distances;
        distances.reserve(upper_elems.size() - 1);

        for (const auto distance : upper_elems | std::views::pairwise_transform(Distance)) {
            distances.push_back(distance);
        }

        return distances;
    }();

    [[assume(distances.size() > 0)]];

    if (std::ranges::all_of(distances, [&](auto &distance) {
        return distance == distances.front();
    })) {
        return distances.front();
    }

    if constexpr (Behind) {
        return distances.front() - next_distance<Behind>(distances);
    } else {
        return distances.back() + next_distance<Behind>(distances);
    }
}

template<bool Behind>
constexpr std::int64_t find_next_element(const std::string_view pattern_description) {
    const auto pattern = [&]() {
        std::vector<std::int64_t> pattern;

        advent::split_for_each(pattern_description, ' ', [&](const std::string_view num) {
            pattern.push_back(advent::to_integral<std::int64_t>(num));
        });

        return pattern;
    }();

    if constexpr (Behind) {
        return pattern.front() - next_distance<Behind>(pattern);
    } else {
        return pattern.back() + next_distance<Behind>(pattern);
    }
}

static_assert(find_next_element<false>("0 3 6 9 12 15")     == 18);
static_assert(find_next_element<false>("1 3 6 10 15 21")    == 28);
static_assert(find_next_element<false>("10 13 16 21 30 45") == 68);

static_assert(find_next_element<true>("0 3 6 9 12 15")     == -3);
static_assert(find_next_element<true>("1 3 6 10 15 21")    == 0);
static_assert(find_next_element<true>("10 13 16 21 30 45") == 5);

template<bool Behind, advent::string_viewable_range Rng>
constexpr std::int64_t sum_next_elements(Rng &&rng) {
    std::int64_t sum = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        sum += find_next_element<Behind>(line);
    }

    return sum;
}

template<bool Behind>
constexpr std::int64_t sum_next_elements_from_string_data(const std::string_view data) {
    return sum_next_elements<Behind>(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "0 3 6 9 12 15\n"
    "1 3 6 10 15 21\n"
    "10 13 16 21 30 45\n"
);

static_assert(sum_next_elements_from_string_data<false>(example_data) == 114);
static_assert(sum_next_elements_from_string_data<true>(example_data) == 2);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_next_elements_from_string_data<false>,
        sum_next_elements_from_string_data<true>
    );
}
