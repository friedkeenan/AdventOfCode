#include <advent/advent.hpp>

struct PageOrdering {
    template<advent::string_viewable_iterator It>
    static constexpr std::vector<PageOrdering> ParseOrderingsAndAdvanceIterator(It &it) {
        std::vector<PageOrdering> result;

        while (true) {
            const std::string_view line = *it;

            /* NOTE: This will advance past the empty line. */
            ++it;

            if (line.empty()) {
                return result;
            }

            result.emplace_back(line);
        }
    }

    constexpr explicit PageOrdering(const std::string_view description) {
        const auto separator_pos = description.find_first_of('|');
        [[assume(separator_pos != std::string_view::npos)]];

        this->before = advent::to_integral<std::size_t>(description.substr(0, separator_pos));
        this->after  = advent::to_integral<std::size_t>(description.substr(separator_pos + 1));

        [[assume(this->before != this->after)]];
    }

    std::size_t before;
    std::size_t after;

    constexpr bool satisfied_by_update(this const PageOrdering self, const std::vector<std::size_t> &update) {
        const auto before_it = std::ranges::find(update, self.before);
        if (before_it == update.end()) {
            return true;
        }

        const auto after_it = std::ranges::find(update, self.after);

        /* NOTE: Will return true if 'after_it' is the end iterator. */
        return before_it < after_it;
    }
};

constexpr std::vector<std::size_t> parse_update(const std::string_view description) {
    return (
        description | advent::views::split_string(',') |

        std::views::transform(advent::to_integral<std::size_t>) |

        std::ranges::to<std::vector>()
    );
}

constexpr bool is_update_correct(const std::vector<PageOrdering> &orderings, const std::vector<std::size_t> &update) {
    /*
        NOTE: We could use 'std::ranges::is_sorted' with the
        comparator for part two, but that appears to be slightly slower.
    */

    return std::ranges::all_of(orderings, [&](const auto ordering) {
        return ordering.satisfied_by_update(update);
    });
}

template<advent::string_viewable_range Rng>
constexpr std::size_t sum_middle_of_correct_updates(Rng &&rng) {
    auto it = std::ranges::begin(rng);

    const auto orderings = PageOrdering::ParseOrderingsAndAdvanceIterator(it);

    std::size_t sum = 0;
    for (const std::string_view line : std::ranges::subrange(it, std::ranges::end(rng))) {
        if (line.empty()) {
            continue;
        }

        const auto update = parse_update(line);
        [[assume(update.size() > 0)]];

        if (!is_update_correct(orderings, update)) {
            continue;
        }

        sum += update[update.size() / 2];
    }

    return sum;
}

constexpr void make_update_correct(const std::vector<PageOrdering> &orderings, std::vector<std::size_t> &update) {
    std::ranges::sort(update, [&](const auto left, const auto right) {
        for (const auto ordering : orderings) {
            if (ordering.before == left && ordering.after == right) {
                /* Correctly ordered. */
                return true;
            }

            if (ordering.before == right && ordering.after == left) {
                /* Incorrectly ordered. */
                return false;
            }
        }

        /* Order doesn't matter. */
        return false;
    });
}

template<advent::string_viewable_range Rng>
constexpr std::size_t sum_middle_of_corrected_updates(Rng &&rng) {
    auto it = std::ranges::begin(rng);

    const auto orderings = PageOrdering::ParseOrderingsAndAdvanceIterator(it);

    std::size_t sum = 0;
    for (const std::string_view line : std::ranges::subrange(it, std::ranges::end(rng))) {
        if (line.empty()) {
            continue;
        }

        auto update = parse_update(line);
        [[assume(update.size() > 0)]];

        if (is_update_correct(orderings, update)) {
            /* We don't care about updates which are already correct. */

            continue;
        }

        make_update_correct(orderings, update);

        sum += update[update.size() / 2];
    }

    return sum;
}

constexpr std::size_t sum_middle_of_correct_updates_from_string_data(const std::string_view data) {
    return sum_middle_of_correct_updates(data | advent::views::split_lines);
}

constexpr std::size_t sum_middle_of_corrected_updates_from_string_data(const std::string_view data) {
    return sum_middle_of_corrected_updates(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "47|53\n"
    "97|13\n"
    "97|61\n"
    "97|47\n"
    "75|29\n"
    "61|13\n"
    "75|53\n"
    "29|13\n"
    "97|29\n"
    "53|29\n"
    "61|53\n"
    "97|53\n"
    "61|29\n"
    "47|13\n"
    "75|47\n"
    "97|75\n"
    "47|61\n"
    "75|61\n"
    "47|29\n"
    "75|13\n"
    "53|13\n"
    "\n"
    "75,47,61,53,29\n"
    "97,61,53,29,13\n"
    "75,29,13\n"
    "75,97,47,61,53\n"
    "61,13,29\n"
    "97,13,75,29,47\n"
);

static_assert(sum_middle_of_correct_updates_from_string_data(example_data) == 143);
static_assert(sum_middle_of_corrected_updates_from_string_data(example_data) == 123);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_middle_of_correct_updates_from_string_data,
        sum_middle_of_corrected_updates_from_string_data
    );
}
