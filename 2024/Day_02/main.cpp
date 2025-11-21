import std;
import advent;

struct Report {
    std::vector<advent::ssize_t> levels;

    constexpr explicit Report(const std::string_view line)
    :
        levels(
            std::from_range,

            line |
            advent::views::split_string(' ') |
            std::views::transform(advent::to_integral<advent::ssize_t>)
        )
    {
        [[assume(this->levels.size() >= 2)]];
    }

    template<std::ranges::forward_range Rng>
    requires (std::same_as<std::ranges::range_value_t<Rng>, advent::ssize_t>)
    static constexpr bool _safe_without_dampener(Rng &&levels) {
        auto it = std::ranges::begin(levels);
        const auto end = std::ranges::end(levels);

        const auto first = *it;
        ++it;

        const auto second = *it;

        const bool increasing = (second > first);

        const auto initial_difference = advent::abs(second - first);
        if (initial_difference < 1 || initial_difference > 3) {
            return false;
        }

        for (const auto [prev, next] : std::ranges::subrange(it, end) | std::views::pairwise) {
            if (increasing && next <= prev) {
                return false;
            } else if (!increasing && next >= prev) {
                return false;
            }

            const auto difference = advent::abs(next - prev);
            if (difference < 1 || difference > 3) {
                return false;
            }
        }

        return true;
    }

    constexpr bool safe_without_dampener(this const Report &self) {
        return _safe_without_dampener(self.levels);
    }

    constexpr bool safe_with_dampener(this const Report &self) {
        /*
            NOTE: There is definitely a way to do this which is
            less algorithmically complex, but I don't think it's
            worth the effort here.
        */

        if (self.safe_without_dampener()) {
            return true;
        }

        if (self.levels.size() <= 2) {
            return false;
        }

        for (const auto skipped_iterator : std::views::iota(self.levels.begin(), self.levels.end())) {
            if (_safe_without_dampener(
                std::views::concat(
                    std::ranges::subrange(self.levels.begin(),  skipped_iterator),
                    std::ranges::subrange(skipped_iterator + 1, self.levels.end())
                )
            )) {
                return true;
            }
        }

        return false;
    }
};

static_assert(Report("1 2 3 4").levels == std::vector{1z, 2z, 3z, 4z});

static_assert(!Report("1 5").safe_without_dampener());
static_assert(!Report("1 5").safe_with_dampener());

template<advent::string_viewable_range Rng, typename Predicate>
requires (std::predicate<Predicate &, Report>)
constexpr std::size_t count_reports_if(Rng &&rng, Predicate predicate) {
    std::size_t num_safe_reports = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        if (std::invoke(predicate, Report(line))) {
            ++num_safe_reports;
        }
    }

    return num_safe_reports;
}

template<advent::string_viewable_range Rng>
constexpr std::size_t count_safe_reports_without_dampener(Rng &&rng) {
    return count_reports_if(std::forward<Rng>(rng), &Report::safe_without_dampener);
}

template<advent::string_viewable_range Rng>
constexpr std::size_t count_safe_reports_with_dampener(Rng &&rng) {
    return count_reports_if(std::forward<Rng>(rng), &Report::safe_with_dampener);
}

constexpr std::size_t count_safe_reports_without_dampener_from_string_data(const std::string_view data) {
    return count_safe_reports_without_dampener(data | advent::views::split_lines);
}

constexpr std::size_t count_safe_reports_with_dampener_from_string_data(const std::string_view data) {
    return count_safe_reports_with_dampener(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "7 6 4 2 1\n"
    "1 2 7 8 9\n"
    "9 7 6 2 1\n"
    "1 3 2 4 5\n"
    "8 6 4 4 1\n"
    "1 3 6 7 9\n"
);

static_assert(count_safe_reports_without_dampener_from_string_data(example_data) == 2);
static_assert(count_safe_reports_with_dampener_from_string_data(example_data) == 4);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_safe_reports_without_dampener_from_string_data,
        count_safe_reports_with_dampener_from_string_data
    );
}
