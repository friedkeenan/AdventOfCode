#include <advent/advent.hpp>

class Packet {
    public:
        struct EndTag { };

        std::string_view _representation;

        /* Don't make this the constructor so we can construct a packet with a pre-trimmed representation. */
        static constexpr Packet Parse(std::string_view representation) {
            advent::assume(representation.length() >= 2);

            /* Remove enclosing brackets. */
            representation.remove_prefix(1);
            representation.remove_suffix(1);

            return Packet{representation};
        }

        constexpr bool is_at_end() const {
            return this->_representation.empty();
        }

        constexpr auto next() {
            using Element = std::variant<std::size_t, Packet, EndTag>;

            /* We have no more elements. */
            if (this->is_at_end()) {
                return Element{EndTag{}};
            }

            /* Remove comma from previous element. */
            if (this->_representation[0] == ',') {
                this->_representation.remove_prefix(1);
            }

            /* The next element is a sub-packet. */
            if (this->_representation[0] == '[') {
                const auto end_pos = [&]() {
                    /*
                        Get the position of the ending bracket for this sub-packet.
                        Note that a sub-packet may contain its own sub-packets.
                    */

                    std::size_t num_packets = 1;

                    for (const auto pos : std::views::iota(1uz, this->_representation.length())) {
                        if (this->_representation[pos] == ']') {
                            --num_packets;

                            if (num_packets == 0) {
                                return pos;
                            }
                        } else if (this->_representation[pos] == '[') {
                            ++num_packets;
                        }
                    }

                    std::unreachable();
                }();

                const auto packet = Packet::Parse(std::string_view(this->_representation.data(), end_pos + 1));
                this->_representation.remove_prefix(end_pos + 1);

                return Element{packet};
            }

            /* Our next element is a number. */

            const auto comma_pos = this->_representation.find_first_of(',');

            /* We're at the last element. */
            if (comma_pos == std::string_view::npos) {
                const auto elem = advent::to_integral<std::size_t>(this->_representation);
                this->_representation.remove_prefix(this->_representation.length());

                return Element{elem};
            }

            const auto elem = advent::to_integral<std::size_t>(std::string_view(this->_representation.data(), comma_pos));
            this->_representation.remove_prefix(comma_pos);

            return Element{elem};
        }

        enum class CompareResult : std::uint8_t {
            DefinitelyCorrect,
            DefinitelyIncorrect,
            Continue,
        };

        constexpr Packet _packet_from_last_integer() const {
            /* This is super cursed and I am sorry. */

            auto integer_start = this->_representation.data();
            while (true) {
                if (*(integer_start - 1) == ',' || *(integer_start - 1) == '[') {
                    /* NOTE: We just slap the number string into the packet's representation. */
                    return Packet{std::string_view(integer_start, this->_representation.data())};
                }

                --integer_start;
            }
        }

        static constexpr CompareResult _correct_order_impl(Packet left, Packet right) {
            auto result = CompareResult::Continue;

            while (result == CompareResult::Continue) {
                bool both_reached_end = false;

                result = std::visit(
                    advent::overloaded{
                        [](const std::size_t left, const std::size_t right) {
                            if (left < right) {
                                return CompareResult::DefinitelyCorrect;
                            }

                            if (left > right) {
                                return CompareResult::DefinitelyIncorrect;
                            }

                            return CompareResult::Continue;
                        },

                        [](const Packet left, const Packet right) {
                            return _correct_order_impl(left, right);
                        },

                        [&](Packet left, const std::size_t) {
                            const auto right_packet = right._packet_from_last_integer();

                            return _correct_order_impl(left, right_packet);
                        },

                        [&](const std::size_t, Packet right) {
                            const auto left_packet = left._packet_from_last_integer();

                            return _correct_order_impl(left_packet, right);
                        },

                        [&](EndTag, EndTag) {
                            both_reached_end = true;

                            return CompareResult::Continue;
                        },

                        [](auto, EndTag) {
                            return CompareResult::DefinitelyIncorrect;
                        },

                        [](EndTag, auto) {
                            return CompareResult::DefinitelyCorrect;
                        }
                    },

                    left.next(), right.next()
                );

                if (both_reached_end) {
                    return CompareResult::Continue;
                }
            }

            return result;
        }

        constexpr bool correct_order(Packet other) const {
            const auto result = this->_correct_order_impl(*this, other);

            return result == CompareResult::DefinitelyCorrect || result == CompareResult::Continue;
        }

        constexpr bool operator ==(const Packet &) const = default;
};

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t sum_correct_indices(Rng &&packet_representations) {
    std::size_t index_sum = 0;

    /* The indices are 1-indexed. */
    std::size_t pair_index = 1;

    auto it = std::ranges::begin(packet_representations);
    while (it != std::ranges::end(packet_representations)) {
        if (static_cast<std::string_view>(*it).empty()) {
            ++it;

            continue;
        }

        const std::string_view left_representation = *it;
        ++it;

        const std::string_view right_representation = *it;
        ++it;

        const auto left  = Packet::Parse(left_representation);
        const auto right = Packet::Parse(right_representation);

        if (left.correct_order(right)) {
            index_sum += pair_index;
        }

        ++pair_index;
    }

    return index_sum;
}

constexpr inline auto Divider2 = Packet::Parse("[[2]]");
constexpr inline auto Divider6 = Packet::Parse("[[6]]");

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t decoder_key(Rng &&packet_representations) {
    auto packets = std::vector{Divider2, Divider6};

    for (const std::string_view representation : std::forward<Rng>(packet_representations)) {
        if (representation.empty()) {
            continue;
        }

        packets.push_back(Packet::Parse(representation));
    }

    std::ranges::sort(packets, &Packet::correct_order);

    std::size_t divider_2_index = 0;
    std::size_t divider_6_index = 0;

    for (const auto i : std::views::iota(0uz, packets.size())) {
        const auto &packet = packets[i];

        /* The indices are 1-indexed. */
        if (packet == Divider2) {
            divider_2_index = i + 1;
        } else if (packet == Divider6) {
            divider_6_index = i + 1;
        }

        if (divider_2_index != 0 && divider_6_index != 0) {
            return divider_2_index * divider_6_index;
        }
    }

    std::unreachable();
}

constexpr std::size_t sum_correct_indices_from_string_data(const std::string_view data) {
    return sum_correct_indices(advent::views::split_lines(data));
}

constexpr std::size_t decoder_key_from_string_data(const std::string_view data) {
    return decoder_key(advent::views::split_lines(data));
}

constexpr inline std::string_view example_data = (
    "[1,1,3,1,1]\n"
    "[1,1,5,1,1]\n"
    "\n"
    "[[1],[2,3,4]]\n"
    "[[1],4]\n"
    "\n"
    "[9]\n"
    "[[8,7,6]]\n"
    "\n"
    "[[4,4],4,4]\n"
    "[[4,4],4,4,4]\n"
    "\n"
    "[7,7,7,7]\n"
    "[7,7,7]\n"
    "\n"
    "[]\n"
    "[3]\n"
    "\n"
    "[[[]]]\n"
    "[[]]\n"
    "\n"
    "[1,[2,[3,[4,[5,6,7]]]],8,9]\n"
    "[1,[2,[3,[4,[5,6,0]]]],8,9]\n"
);

static_assert(sum_correct_indices_from_string_data(example_data) == 13);
static_assert(decoder_key_from_string_data(example_data) == 140);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_correct_indices_from_string_data,
        decoder_key_from_string_data
    );
}
