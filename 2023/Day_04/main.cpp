import std;
import advent;

struct Card {
    std::vector<std::size_t> winning_numbers;
    std::vector<std::size_t> received_numbers;

    constexpr explicit Card(const std::string_view description) {
        const auto colon_pos = description.find_first_of(':');
        [[assume(colon_pos != std::string_view::npos)]];

        const auto separator_pos = description.find_first_of('|');
        [[assume(separator_pos != std::string_view::npos)]];

        /* Move past colon and trailing space. */
        const auto winning_start = colon_pos + 1 + 1;

        /* Move behind the preceding space padding of the separator. */
        const auto winning_end = separator_pos - 1;

        const auto winning_description = description.substr(winning_start, winning_end - winning_start);

        advent::split_with_callback(winning_description, ' ', [&](const std::string_view number) {
            if (number.empty()) {
                return;
            }

            this->winning_numbers.push_back(advent::to_integral<std::size_t>(number));
        });

        /* Move past the separator and the following space padding. */
        const auto receeived_description = description.substr(separator_pos + 1 + 1);

        advent::split_with_callback(receeived_description, ' ', [&](const std::string_view number) {
            if (number.empty()) {
                return;
            }

            this->received_numbers.push_back(advent::to_integral<std::size_t>(number));
        });
    }

    constexpr bool is_winning_number(const std::size_t number) const {
        return std::ranges::contains(this->winning_numbers, number);
    }

    constexpr std::size_t count_matches() const {
        std::size_t count = 0;

        for (const auto &received : this->received_numbers) {
            if (this->is_winning_number(received)) {
                ++count;
            }
        }

        return count;
    }

    constexpr std::size_t points() const {
        /*
            2**(n - 1) if n > 0 else 0.

            NOTE: Using bitshifting seems
            to result in better codegen.
        */

        return (1uz << this->count_matches()) >> 1uz;
    }
};

template<advent::string_viewable_range Rng>
constexpr std::size_t sum_card_points(Rng &&rng) {
    std::size_t sum = 0;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        const auto card = Card(line);

        sum += card.points();
    }

    return sum;
}

template<advent::string_viewable_range Rng>
constexpr std::size_t sum_remnant_scratch_cards(Rng &&rng) {
    struct CardTracker {
        Card        card;
        std::size_t amount;

        constexpr explicit CardTracker(const std::string_view description) : card(description), amount(1) { }
    };

    std::vector<CardTracker> trackers;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        trackers.emplace_back(line);
    }

    for (const auto [i, tracker] : trackers | std::views::enumerate) {
        for (const auto copy_distance : std::views::iota(1uz, tracker.card.count_matches() + 1)) {
            trackers[i + copy_distance].amount += tracker.amount;
        }
    }

    return std::ranges::fold_left(trackers | std::views::transform(&CardTracker::amount), 0uz, std::plus{});
}

constexpr std::size_t sum_card_points_from_string_data(const std::string_view data) {
    return sum_card_points(data | advent::views::split_lines);
}

constexpr std::size_t sum_remnant_scratch_cards_from_string_data(const std::string_view data) {
    return sum_remnant_scratch_cards(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53\n"
    "Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19\n"
    "Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1\n"
    "Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83\n"
    "Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36\n"
    "Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11\n"
);

static_assert(sum_card_points_from_string_data(example_data) == 13);
static_assert(sum_remnant_scratch_cards_from_string_data(example_data) == 30);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_card_points_from_string_data,
        sum_remnant_scratch_cards_from_string_data
    );
}
