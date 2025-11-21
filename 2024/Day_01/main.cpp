import std;
import advent;

struct LocationIDLists {
    /* NOTE: We use 'advent::ssize_t' here so that subtraction won't overflow. */
    std::vector<advent::ssize_t> left;
    std::vector<advent::ssize_t> right;

    template<advent::string_viewable_range Rng>
    constexpr explicit LocationIDLists(Rng &&rng) {
        for (const std::string_view line : std::forward<Rng>(rng)) {
            if (line.empty()) {
                continue;
            }

            const auto separator_start = line.find_first_of(' ');
            [[assume(separator_start != std::string_view::npos)]];

            const auto separator_end = line.find_last_of(' ');
            [[assume(separator_end != std::string_view::npos)]];

            const auto left  = advent::to_integral<advent::ssize_t>(line.substr(0, separator_start));
            const auto right = advent::to_integral<advent::ssize_t>(line.substr(separator_end + 1));

            [[assume(left  >= 0)]];
            [[assume(right >= 0)]];

            this->left.push_back(left);
            this->right.push_back(right);
        }
    }

    constexpr void sort(this LocationIDLists &self) {
        std::ranges::sort(self.left);
        std::ranges::sort(self.right);
    }
};

template<advent::string_viewable_range Rng>
constexpr std::size_t sum_sorted_distances(Rng &&rng) {
    auto lists = LocationIDLists(std::forward<Rng>(rng));

    lists.sort();

    std::size_t distance_sum = 0;
    for (const auto [left, right] : std::views::zip(lists.left, lists.right)) {
        distance_sum += advent::abs(left - right);
    }

    return distance_sum;
}

template<advent::string_viewable_range Rng>
constexpr std::size_t find_similarity_score(Rng &&rng) {
    const auto lists = LocationIDLists(std::forward<Rng>(rng));

    /*
        NOTE: It *might* be more optimal to sort the lists beforehand
        here as well so that we don't need to look through the entire
        list to count equal elements, but I sorta doubt it'd be worth it.
    */

    std::size_t similarity_score = 0;
    for (const auto &left : lists.left) {
        const auto right_occurrences = std::ranges::count(lists.right, left);

        similarity_score += left * right_occurrences;
    }

    return similarity_score;
}

constexpr std::size_t sum_sorted_distances_from_string_data(const std::string_view data) {
    return sum_sorted_distances(data | advent::views::split_lines);
}

constexpr std::size_t find_similarity_score_from_string_data(const std::string_view data) {
    return find_similarity_score(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "3   4\n"
    "4   3\n"
    "2   5\n"
    "1   3\n"
    "3   9\n"
    "3   3\n"
);

static_assert(sum_sorted_distances_from_string_data(example_data)  == 11);
static_assert(find_similarity_score_from_string_data(example_data) == 31);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_sorted_distances_from_string_data,
        find_similarity_score_from_string_data
    );
}
