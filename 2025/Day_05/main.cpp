import std;
import advent;

struct FreshIngredients {
    struct Range {
        std::size_t first;
        std::size_t last;

        constexpr explicit Range(const std::string_view description) {
            const auto separator = description.find_first_of('-');
            [[assume(separator != std::string_view::npos)]];

            this->first = advent::to_integral<std::size_t>(description.substr(0, separator));
            this->last  = advent::to_integral<std::size_t>(description.substr(separator + 1));

            [[assume(this->first <= this->last)]];
        }

        constexpr bool contains(this const Range self, const std::size_t id) {
            return id >= self.first && id <= self.last;
        }

        constexpr std::size_t num_ingredients(this const Range self) {
            return self.last - self.first + 1;
        }

        constexpr bool can_merge(this const Range self, const Range other) {
            if (self.first > other.last) {
                return false;
            }

            if (self.last < other.first) {
                return false;
            }

            return true;
        }

        constexpr bool try_merge(this Range &self, const Range other) {
            if (!self.can_merge(other)) {
                return false;
            }

            if (self.first <= other.first) {
                if (self.last <= other.last) {
                    self.last = other.last;

                    return true;
                }

                /* We wholly contain 'other' already. */
                return true;
            }

            self.first = other.first;

            if (self.last <= other.last) {
                /* We are wholly contained within 'other'. */

                self.last = other.last;
            }

            return true;
        }
    };

    template<advent::string_viewable_iterator It>
    static constexpr FreshIngredients ParseAndAdvance(It &it) {
        FreshIngredients result;

        while (true) {
            const std::string_view line = *it;
            ++it;

            if (line.empty()) {
                return result;
            }

            result.fresh_ranges.emplace_back(line);
        }
    }

    template<advent::string_viewable_iterator It>
    static constexpr FreshIngredients ParseDeduplicatedAndAdvance(It &it) {
        FreshIngredients result;

        while (true) {
            const std::string_view line = *it;
            ++it;

            if (line.empty()) {
                return result;
            }

            result.add_range_deduplicated(Range(line));
        }
    }

    std::vector<Range> fresh_ranges;

    constexpr bool try_deduplicate_into(this FreshIngredients &self, Range &range) {
        for (const auto it : self.fresh_ranges | advent::views::iterators) {
            if (range.try_merge(*it)) {
                self.fresh_ranges.erase(it);

                return true;
            }
        }

        return false;
    }

    constexpr void add_range_deduplicated(this FreshIngredients &self, Range range) {
        while (true) {
            if (!self.try_deduplicate_into(range)) {
                break;
            }
        }

        self.fresh_ranges.push_back(range);
    }

    constexpr bool contains(this const FreshIngredients &self, const std::size_t id) {
        return std::ranges::any_of(self.fresh_ranges, [&](const auto range) {
            return range.contains(id);
        });
    }
};

template<advent::string_viewable_range Rng>
constexpr std::size_t count_available_fresh_ingredients(Rng &&rng) {
    auto it = std::ranges::begin(rng);

    /* Parsing the deduplicated ranges is faster by about 60 microseconds. */
    const auto fresh_ingredients = FreshIngredients::ParseDeduplicatedAndAdvance(it);

    std::size_t num_fresh = 0;
    for (const std::string_view line : std::ranges::subrange(std::move(it), std::ranges::end(rng))) {
        if (line.empty()) {
            continue;
        }

        const auto id = advent::to_integral<std::size_t>(line);

        if (fresh_ingredients.contains(id)) {
            num_fresh += 1;
        }
    }

    return num_fresh;
}

template<advent::string_viewable_range Rng>
constexpr std::size_t count_possible_fresh_ingredients(Rng &&rng) {
    auto it = std::ranges::begin(std::forward<Rng>(rng));

    const auto fresh_ingredients = FreshIngredients::ParseDeduplicatedAndAdvance(it);

    std::size_t num_fresh = 0;
    for (const auto range : fresh_ingredients.fresh_ranges) {
        num_fresh += range.num_ingredients();
    }

    return num_fresh;
}

constexpr std::size_t count_available_fresh_ingredients_from_string_data(const std::string_view data) {
    return count_available_fresh_ingredients(data | advent::views::split_lines);
}

constexpr std::size_t count_possible_fresh_ingredients_from_string_data(const std::string_view data) {
    return count_possible_fresh_ingredients(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "3-5\n"
    "10-14\n"
    "16-20\n"
    "12-18\n"
    "\n"
    "1\n"
    "5\n"
    "8\n"
    "11\n"
    "17\n"
    "32\n"
);

static_assert(count_available_fresh_ingredients_from_string_data(example_data) == 3);
static_assert(count_possible_fresh_ingredients_from_string_data(example_data)  == 14);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_available_fresh_ingredients_from_string_data,
        count_possible_fresh_ingredients_from_string_data
    );
}
