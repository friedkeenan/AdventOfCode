import std;
import advent;

struct SectionInterval {
    /* Range from [start, end]. */
    std::size_t start;
    std::size_t end;

    constexpr explicit SectionInterval(const std::string_view interval) {
        const auto end_of_start = interval.find_first_of('-');

        this->start = advent::to_integral<std::size_t>(interval.substr(0, end_of_start));
        this->end   = advent::to_integral<std::size_t>(interval.substr(end_of_start + 1));

        [[assume(this->start <= this->end)]];
    }

    constexpr bool contains(const SectionInterval &other) const {
        return (
            other.start >= this->start &&
            other.end   <= this->end
        );
    }

    constexpr bool wholly_overlaps(const SectionInterval &other) const {
        return this->contains(other) || other.contains(*this);
    }

    constexpr bool partially_overlaps(const SectionInterval &other) const {
        /*
            If our start is <= other's end,
            then we may overlap, but if our
            end is also >= other's start,
            then we must overlap.
        */
        return (
            this->start <= other.end   &&
            this->end   >= other.start
        );
    }
};

template<typename Checker>
concept SectionsChecker = requires(const Checker checker, const SectionInterval &sections) {
    { std::invoke(checker, sections, sections) } -> std::convertible_to<bool>;
};

template<SectionsChecker auto Checker, advent::string_viewable_range Rng>
constexpr std::size_t count_section_pairs_with_checker(Rng &&pairs) {
    std::size_t num_passing_pairs = 0;

    for (const std::string_view intervals : std::forward<Rng>(pairs)) {
        if (intervals.empty()) {
            continue;
        }

        const auto comma_pos = intervals.find_first_of(',');
        const auto first_sections = SectionInterval(intervals.substr(0, comma_pos));
        const auto second_sections = SectionInterval(intervals.substr(comma_pos + 1));

        if (std::invoke(Checker, first_sections, second_sections)) {
            ++num_passing_pairs;
        }
    }

    return num_passing_pairs;
}

consteval {
    advent::part_one.is_solved_by(^^count_section_pairs_with_checker, &SectionInterval::wholly_overlaps);
    advent::part_two.is_solved_by(^^count_section_pairs_with_checker, &SectionInterval::partially_overlaps);
}

constexpr inline std::string_view example_data = (
    "2-4,6-8\n"
    "2-3,4-5\n"
    "5-7,7-9\n"
    "2-8,3-7\n"
    "6-6,4-6\n"
    "2-6,4-8\n"
);

static_assert(advent::part_one() == 2);
static_assert(advent::part_two() == 4);

int main(int argc, char **argv) {
    return advent::solve_puzzles(argc, argv);
}
