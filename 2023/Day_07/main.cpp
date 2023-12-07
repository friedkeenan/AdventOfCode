#include <advent/advent.hpp>

struct StandardBehavior {
    enum class Card : std::uint8_t {
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten,
        Jack,
        Queen,
        King,
        Ace
    };

    friend constexpr auto operator <=>(const Card lhs, const Card rhs) {
        return std::to_underlying(lhs) <=> std::to_underlying(rhs);
    }

    static_assert(Card::Three > Card::Two);
    static_assert(Card::Ace   > Card::King);

    static constexpr std::size_t NumCardValues = std::to_underlying(Card::Ace) - std::to_underlying(Card::Two) + 1;

    static constexpr Card CardFromCharacter(const char c) {
        switch (c) {
            case '2': return Card::Two;
            case '3': return Card::Three;
            case '4': return Card::Four;
            case '5': return Card::Five;
            case '6': return Card::Six;
            case '7': return Card::Seven;
            case '8': return Card::Eight;
            case '9': return Card::Nine;
            case 'T': return Card::Ten;
            case 'J': return Card::Jack;
            case 'Q': return Card::Queen;
            case 'K': return Card::King;
            case 'A': return Card::Ace;

            default: std::unreachable();
        }
    }

    static constexpr std::size_t HandSize = 5;

    static_assert(HandSize <= std::numeric_limits<std::uint8_t>::max());
    using CountStorage = std::array<std::uint8_t, HandSize>;

    struct CardCounter {
        std::array<std::uint8_t, NumCardValues> counts = {};

        constexpr void count_card(const Card card) {
            const std::size_t index = std::to_underlying(card) - std::to_underlying(Card::Two);
            [[assume(index < NumCardValues)]];

            ++this->counts[index];
        }

        constexpr CountStorage digest() const {
            CountStorage digested = {};

            std::size_t digested_index = 0;
            for (const auto &count : this->counts) {
                if (count <= 0) {
                    continue;
                }

                digested[digested_index] = count;

                ++digested_index;
                if (digested_index >= HandSize) {
                    break;
                }
            }

            std::ranges::sort(digested, std::greater{});

            return digested;
        }
    };
};

struct JokerBehavior {
    enum class Card : std::uint8_t {
        Joker,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten,
        Queen,
        King,
        Ace
    };

    friend constexpr auto operator <=>(const Card lhs, const Card rhs) {
        return std::to_underlying(lhs) <=> std::to_underlying(rhs);
    }

    static_assert(Card::Two > Card::Joker);
    static_assert(Card::Ace > Card::King);

    static constexpr std::size_t NumCardValues = std::to_underlying(Card::Ace) - std::to_underlying(Card::Joker) + 1;

    static constexpr Card CardFromCharacter(const char c) {
        switch (c) {
            case 'J': return Card::Joker;
            case '2': return Card::Two;
            case '3': return Card::Three;
            case '4': return Card::Four;
            case '5': return Card::Five;
            case '6': return Card::Six;
            case '7': return Card::Seven;
            case '8': return Card::Eight;
            case '9': return Card::Nine;
            case 'T': return Card::Ten;
            case 'Q': return Card::Queen;
            case 'K': return Card::King;
            case 'A': return Card::Ace;

            default: std::unreachable();
        }
    }

    static constexpr std::size_t HandSize = 5;

    static_assert(HandSize <= std::numeric_limits<std::uint8_t>::max());
    using CountStorage = std::array<std::uint8_t, HandSize>;

    struct CardCounter {
        std::array<std::uint8_t, NumCardValues> counts = {};

        constexpr static std::size_t IndexForCard(const Card card) {
            const std::size_t index = std::to_underlying(card) - std::to_underlying(Card::Joker);
            [[assume(index < NumCardValues)]];

            return index;
        }

        constexpr std::uint8_t &count_for_card(const Card card) {
            return this->counts[IndexForCard(card)];
        }

        constexpr void count_card(const Card card) {
            ++this->count_for_card(card);
        }

        constexpr std::uint8_t &max_non_joker_count() {
            static_assert(IndexForCard(Card::Joker) == 0);
            static_assert(NumCardValues >= 2);

            auto max_count_ptr = &this->counts[1];
            for (const auto i : std::views::iota(2uz, NumCardValues)) {
                auto &count = this->counts[i];
                if (count > *max_count_ptr) {
                    max_count_ptr = &count;
                }
            }

            return *max_count_ptr;
        }

        constexpr CountStorage digest() {
            /*
                We add the joker count onto the card with
                the max count and set the joker count to 0.

                This always gives us the optimal joker usage.

                We could maybe optimize this to only
                do one loop over the card counts but
                I don't think it's worth the effort.
            */
            auto &joker_count = this->count_for_card(Card::Joker);
            if (joker_count < HandSize) {
                auto &max_real_count = this->max_non_joker_count();

                max_real_count += std::exchange(joker_count, 0);
            }

            CountStorage digested = {};

            std::size_t digested_index = 0;
            for (const auto &count : this->counts) {
                if (count <= 0) {
                    continue;
                }

                digested[digested_index] = count;

                ++digested_index;
                if (digested_index >= HandSize) {
                    break;
                }
            }

            std::ranges::sort(digested, std::greater{});

            return digested;
        }
    };
};

template<typename Counter, typename Card, typename CountStorage>
concept CardCounter = std::default_initializable<Counter> && requires(Counter &counter, const Card &card) {
    { counter.count_card(card) } -> std::same_as<void>;
    { counter.digest() }         -> std::same_as<CountStorage>;
};

template<typename Behavior>
concept HandBehavior = requires(const char &c) {
    typename Behavior::Card;
    requires std::three_way_comparable<typename Behavior::Card>;

    { Behavior::NumCardValues } -> std::same_as<const std::size_t &>;
    { Behavior::HandSize }      -> std::same_as<const std::size_t &>;

    { Behavior::CardFromCharacter(c) } -> std::same_as<typename Behavior::Card>;

    typename Behavior::CountStorage;

    typename Behavior::CardCounter;

    requires CardCounter<typename Behavior::CardCounter, typename Behavior::Card, typename Behavior::CountStorage>;
};

template<HandBehavior Behavior>
struct Hand {
    using Card         = typename Behavior::Card;
    using CountStorage = typename Behavior::CountStorage;
    using Count        = typename CountStorage::value_type;

    static constexpr auto HandSize = Behavior::HandSize;

    enum class Type : std::uint8_t {
        HighCard,
        OnePair,
        TwoPair,
        ThreeOfAKind,
        FullHouse,
        FourOfAKind,
        FiveOfAKind,
    };

    friend constexpr auto operator <=>(const Type lhs, const Type rhs) {
        return std::to_underlying(lhs) <=> std::to_underlying(rhs);
    }

    std::array<Card, HandSize> cards;

    constexpr explicit Hand(const std::string_view description) {
        [[assume(description.size() == HandSize)]];

        for (auto [card, repr] : std::views::zip(this->cards, description)) {
            card = Behavior::CardFromCharacter(repr);
        }
    }

    constexpr CountStorage card_counts() const {
        typename Behavior::CardCounter counter;

        for (const auto &card : this->cards) {
            counter.count_card(card);
        }

        return counter.digest();
    }

    template<std::convertible_to<Count>... Others>
    requires (sizeof...(Others) <= HandSize)
    static constexpr bool CompareCounts(const CountStorage &counts, Others &&... others) {
        CountStorage other_storage = {static_cast<Count>(std::forward<Others>(others))...};

        return counts == other_storage;
    }

    constexpr Type type() const {
        const auto counts = this->card_counts();

        if (CompareCounts(counts, 5)) {
            return Type::FiveOfAKind;
        }

        if (CompareCounts(counts, 4, 1)) {
            return Type::FourOfAKind;
        }

        if (CompareCounts(counts, 3, 2)) {
            return Type::FullHouse;
        }

        if (CompareCounts(counts, 3, 1, 1)) {
            return Type::ThreeOfAKind;
        }

        if (CompareCounts(counts, 2, 2, 1)) {
            return Type::TwoPair;
        }

        if (CompareCounts(counts, 2, 1, 1, 1)) {
            return Type::OnePair;
        }

        return Type::HighCard;
    }

    constexpr auto operator <=>(const Hand &rhs) const {
        const auto type_cmp = (this->type() <=> rhs.type());
        if (type_cmp != 0) {
            return type_cmp;
        }

        return this->cards <=> rhs.cards;
    }
};

using StandardHand = Hand<StandardBehavior>;

static_assert(StandardHand("33332").type() == StandardHand::Type::FourOfAKind);
static_assert(StandardHand("2AAAA").type() == StandardHand::Type::FourOfAKind);
static_assert(StandardHand("33332") > StandardHand("2AAAA"));

static_assert(StandardHand("77888").type() == StandardHand::Type::FullHouse);
static_assert(StandardHand("77788").type() == StandardHand::Type::FullHouse);
static_assert(StandardHand("77888") > StandardHand("77788"));

using JokerHand = Hand<JokerBehavior>;

static_assert(JokerHand("33332").type() == JokerHand::Type::FourOfAKind);
static_assert(JokerHand("2AAAA").type() == JokerHand::Type::FourOfAKind);
static_assert(JokerHand("33332") > JokerHand("2AAAA"));

static_assert(JokerHand("77888").type() == JokerHand::Type::FullHouse);
static_assert(JokerHand("77788").type() == JokerHand::Type::FullHouse);
static_assert(JokerHand("77888") > JokerHand("77788"));

static_assert(JokerHand("QJJQ2").type() == JokerHand::Type::FourOfAKind);

static_assert(JokerHand("JKKK2") < JokerHand("QQQQ2"));

template<HandBehavior Behavior>
struct Bet {
    Hand<Behavior> hand;
    std::size_t    bid;

    static constexpr Bet Parse(const std::string_view description) {
        const auto hand_end = description.find_first_of(' ');
        [[assume(hand_end != std::string_view::npos)]];

        return Bet{
            .hand = Hand<Behavior>(description.substr(0, hand_end)),
            .bid  = advent::to_integral<std::size_t>(description.substr(hand_end + 1))
        };
    }
};

template<HandBehavior Behavior, std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t find_total_winnings(Rng &&rng) {
    std::vector<Bet<Behavior>> bets;

    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        bets.push_back(Bet<Behavior>::Parse(line));
    }

    std::ranges::sort(bets, std::less{}, &Bet<Behavior>::hand);

    std::size_t winnings = 0;

    for (auto [i, bet] : bets | std::views::enumerate) {
        winnings += bet.bid * (i + 1);
    }

    return winnings;
}

template<HandBehavior Behavior>
constexpr std::size_t find_total_winnings_from_string_data(const std::string_view data) {
    return find_total_winnings<Behavior>(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "32T3K 765\n"
    "T55J5 684\n"
    "KK677 28\n"
    "KTJJT 220\n"
    "QQQJA 483\n"
);

static_assert(find_total_winnings_from_string_data<StandardBehavior>(example_data) == 6440);
static_assert(find_total_winnings_from_string_data<JokerBehavior>(example_data) == 5905);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        find_total_winnings_from_string_data<StandardBehavior>,
        find_total_winnings_from_string_data<JokerBehavior>
    );
}
