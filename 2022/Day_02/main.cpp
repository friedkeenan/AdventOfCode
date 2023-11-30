#include <advent/advent.hpp>

enum class Shape : std::uint8_t {
    Rock,
    Paper,
    Scissors,
};

class OpponentChoice {
    public:
        Shape shape;

        constexpr explicit OpponentChoice(const char code) : shape([&]() {
            switch (code) {
                case 'A': return Shape::Rock;
                case 'B': return Shape::Paper;
                case 'C': return Shape::Scissors;

                default: std::unreachable();
            }
        }()) { }

        constexpr Shape shape_for_loss() const {
            switch (this->shape) {
                case Shape::Rock:     return Shape::Scissors;
                case Shape::Paper:    return Shape::Rock;
                case Shape::Scissors: return Shape::Paper;

                default: std::unreachable();
            }
        }

        constexpr Shape shape_for_draw() const {
            return this->shape;
        }

        constexpr Shape shape_for_win() const {
            switch (this->shape) {
                case Shape::Rock:     return Shape::Paper;
                case Shape::Paper:    return Shape::Scissors;
                case Shape::Scissors: return Shape::Rock;

                default: std::unreachable();
            }
        }
};

template<typename ShapeDecoderT>
concept ShapeDecoder = requires(const ShapeDecoderT &decoder, const OpponentChoice &opponent, const char &code) {
    { std::invoke(decoder, opponent, code) } -> std::convertible_to<Shape>;
};

/*
    NOTE: We take in the decoder as a template parameter
    as conceptually, to me, player choices using different
    decoders are different things. This becomes more clear
    to me when realizing that we would not want player choices
    decoded using one decoder getting mixed up with player
    choices from another decoder, e.g. getting put in the
    same collection. Thus putting the decoder in a template
    parameter enforces that within the type system.
*/
template<ShapeDecoder auto Decoder>
struct PlayerChoice {
    public:
        OpponentChoice opponent;
        Shape shape;

        constexpr PlayerChoice(const OpponentChoice opponent, const char code)
            : opponent(opponent), shape(std::invoke(Decoder, opponent, code)) { }

        constexpr std::size_t points_for_outcome() const {
            if (this->shape == this->opponent.shape_for_draw()) {
                return 3;
            }

            if (this->shape == this->opponent.shape_for_win()) {
                return 6;
            }

            return 0;
        }

        constexpr std::size_t points_for_shape() const {
            switch (this->shape) {
                case Shape::Rock:     return 1;
                case Shape::Paper:    return 2;
                case Shape::Scissors: return 3;

                default: std::unreachable();
            }
        }
};

template<ShapeDecoder auto Decoder, std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t find_player_total_score(Rng &&strategies) {
    std::size_t total_score = 0;

    for (const std::string_view strategy : std::forward<Rng>(strategies)) {
        if (strategy.empty()) {
            continue;
        }

        const auto opponent = OpponentChoice(strategy[0]);
        const auto player   = PlayerChoice<Decoder>(opponent, strategy[2]);

        total_score += player.points_for_shape() + player.points_for_outcome();
    }

    return total_score;
}

constexpr std::size_t find_player_total_score_naive_decoder(const std::string_view data) {
    return find_player_total_score<
        [](const OpponentChoice, const char code) {
            switch (code) {
                case 'X': return Shape::Rock;
                case 'Y': return Shape::Paper;
                case 'Z': return Shape::Scissors;

                default: std::unreachable();
            }
        }
    >(data | advent::views::split_lines);
}

constexpr std::size_t find_player_total_score_outcome_decoder(const std::string_view data) {
    return find_player_total_score<
        [](const OpponentChoice opponent, const char code) {
            switch (code) {
                case 'X': return opponent.shape_for_loss();
                case 'Y': return opponent.shape_for_draw();
                case 'Z': return opponent.shape_for_win();

                default: std::unreachable();
            }
        }
    >(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "A Y\n"
    "B X\n"
    "C Z\n"
);

static_assert(find_player_total_score_naive_decoder(example_data) == 15);
static_assert(find_player_total_score_outcome_decoder(example_data) == 12);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        find_player_total_score_naive_decoder,
        find_player_total_score_outcome_decoder
    );
}
