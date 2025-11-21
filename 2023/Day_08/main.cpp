#include <advent/defines.hpp>

import std;
import advent;

enum class Direction : char {
    Left  = 'L',
    Right = 'R',
};

struct Directions {
    struct iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        using difference_type = std::ptrdiff_t;

        using value_type = Direction;
        using reference  = Direction;

        const Directions *directions;
        std::size_t pos;

        constexpr iterator() : directions(nullptr), pos(0) { }

        constexpr iterator(const Directions &directions) : directions(&directions), pos(0) { }

        constexpr iterator &operator ++() {
            this->pos = (this->pos + 1) % this->directions->representations.size();

            return *this;
        }

        constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

        constexpr iterator &operator --() {
            if (this->pos <= 0) {
                this->pos = this->directions->representations.size() - 1;
            } else {
                --this->pos;
            }

            return *this;
        }

        constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, --)

        constexpr reference operator *() const {
            return Direction{this->directions->representations[pos]};
        }

        constexpr bool operator ==(const iterator &rhs) const = default;
    };

    static_assert(std::bidirectional_iterator<iterator>);

    std::string_view representations;

    constexpr iterator begin() const {
        return iterator(*this);
    }

    constexpr std::unreachable_sentinel_t end() const {
        return std::unreachable_sentinel;
    }
};

static_assert(std::ranges::bidirectional_range<Directions>);

template<typename Derived, typename StartStorage>
struct Map {
    struct Node {
        bool is_end;

        std::size_t left_index;
        std::size_t right_index;
    };

    std::vector<Node> nodes;

    /*
        NOTE: We store the start info here instead
        of in the derived instance so that it will
        be constructed at the right time.
    */
    StartStorage start_storage;

    constexpr Derived &_derived() {
        static_assert(std::derived_from<Derived, Map>);

        return *static_cast<Derived *>(this);
    }

    template<advent::string_viewable_range Rng>
    constexpr explicit Map(Rng &&rng) {
        static constexpr std::size_t NameLength = 3;
        static constexpr std::size_t LeftStart  = NameLength + 4;
        static constexpr std::size_t RightStart = LeftStart  + NameLength + 2;

        [[assume(std::ranges::begin(rng) != std::ranges::end(rng))]];

        struct IntermediateNode {
            std::string_view name;
            std::string_view left;
            std::string_view right;

            constexpr bool is_degenerate() const {
                return this->name == this->left && this->name == this->right;
            }
        };

        std::vector<IntermediateNode> intermediate;

        /* TODO: Investigate if this could be done better when I receive a constexpr 'std::flat_map'. */

        /*
            NOTE: We could encode the node names directly into
            convenient numbers, but I don't want to do that.
        */

        for (const std::string_view line : std::forward<Rng>(rng)) {
            if (line.empty()) {
                continue;
            }

            const auto name  = line.substr(0,          NameLength);
            const auto left  = line.substr(LeftStart,  NameLength);
            const auto right = line.substr(RightStart, NameLength);

            this->_derived().detect_and_process_start_node(name, intermediate.size());

            intermediate.emplace_back(name, left, right);
        }

        this->nodes.reserve(intermediate.size());

        for (const auto &intermediate_node : intermediate) {
            /*
                NOTE: A potential optimization could be done here,
                where we treat "degenerate" nodes, which can only
                lead back to themselves, as end nodes, since they
                are never encountered unless they are a real end node.

                However *conceptually* it could be the case where we
                reach a degenerate node which is also a real end node,
                but then we need to keep following through it until our
                other start nodes also reach their end nodes.

                And I'm not sure the optimization would even be worth it anyhow.
            */

            std::optional<std::size_t> left_index;
            std::optional<std::size_t> right_index;

            for (const auto [i, other_intermediate] : intermediate | std::views::enumerate) {
                if (other_intermediate.name == intermediate_node.left) {
                    left_index = i;
                }

                if (other_intermediate.name == intermediate_node.right) {
                    right_index = i;
                }

                if (left_index.has_value() && right_index.has_value()) {
                    break;
                }
            }

            [[assume(left_index.has_value() && right_index.has_value())]];

            this->nodes.emplace_back(Derived::IsEndName(intermediate_node.name), *left_index, *right_index);
        }
    }

    constexpr const Node &follow(const Node &node, const Direction direction) const {
        switch (direction) {
            case Direction::Left:  return this->nodes[node.left_index];
            case Direction::Right: return this->nodes[node.right_index];

            default: std::unreachable();
        }
    }
};

struct NormalMap : Map<NormalMap, std::size_t> {
    using Base = Map<NormalMap, std::size_t>;

    static constexpr bool IsEndName(const std::string_view name) {
        return name == "ZZZ";
    }

    using Base::Base;

    constexpr void detect_and_process_start_node(const std::string_view name, const std::size_t index) {
        if (name == "AAA") {
            this->start_storage = index;
        }
    }

    constexpr auto start_nodes() const {
        return std::views::single(&this->nodes[this->start_storage]);
    }
};

struct GhostMap : Map<GhostMap, std::vector<std::size_t>> {
    using Base = Map<GhostMap, std::vector<std::size_t>>;

    static constexpr bool IsEndName(const std::string_view name) {
        return name.ends_with('Z');
    }

    using Base::Base;

    constexpr void detect_and_process_start_node(const std::string_view name, const std::size_t index) {
        if (name.ends_with('A')) {
            this->start_storage.push_back(index);
        }
    }

    constexpr auto start_nodes() const {
        std::vector<const Node *> nodes;
        nodes.reserve(this->start_storage.size());

        for (const auto &index : this->start_storage) {
            nodes.push_back(&this->nodes[index]);
        }

        return nodes;
    }
};

template<typename Map, advent::string_viewable_range Rng>
constexpr std::size_t count_steps_until_end(Rng &&rng) {
    auto it = std::ranges::begin(rng);

    const auto directions = Directions{*it};

    /* Move past directions and empty line. */
    ++it;
    ++it;

    const auto map = Map(std::ranges::subrange(it, std::ranges::end(rng)));

    std::size_t steps_until_end = 1;

    for (auto node : map.start_nodes()) {
        std::size_t steps = 0;

        for (const auto direction : directions) {
            if (node->is_end) {
                [[assume(steps > 0)]];

                /* The data happens to be cyclical in this way. */
                steps_until_end = advent::lcm(steps_until_end, steps);

                break;
            }

            node = &map.follow(*node, direction);
            ++steps;
        }
    }

    return steps_until_end;
}

template<typename Map>
constexpr std::size_t count_steps_until_end_from_string_data(const std::string_view data) {
    return count_steps_until_end<Map>(data | advent::views::split_lines);
}

constexpr inline std::string_view basic_example_data = (
    "RL\n"
    "\n"
    "AAA = (BBB, CCC)\n"
    "BBB = (DDD, EEE)\n"
    "CCC = (ZZZ, GGG)\n"
    "DDD = (DDD, DDD)\n"
    "EEE = (EEE, EEE)\n"
    "GGG = (GGG, GGG)\n"
    "ZZZ = (ZZZ, ZZZ)\n"
);

static_assert(count_steps_until_end_from_string_data<NormalMap>(basic_example_data) == 2);
static_assert(count_steps_until_end_from_string_data<GhostMap>(basic_example_data) == 2);

constexpr inline std::string_view cycled_example_data = (
    "LLR\n"
    "\n"
    "AAA = (BBB, BBB)\n"
    "BBB = (AAA, ZZZ)\n"
    "ZZZ = (ZZZ, ZZZ)\n"
);

static_assert(count_steps_until_end_from_string_data<NormalMap>(cycled_example_data) == 6);
static_assert(count_steps_until_end_from_string_data<GhostMap>(cycled_example_data) == 6);

constexpr inline std::string_view spooky_example_data = (
    "LR\n"
    "\n"
    "11A = (11B, XXX)\n"
    "11B = (XXX, 11Z)\n"
    "11Z = (11B, XXX)\n"
    "22A = (22B, XXX)\n"
    "22B = (22C, 22C)\n"
    "22C = (22Z, 22Z)\n"
    "22Z = (22B, 22B)\n"
    "XXX = (XXX, XXX)\n"
);

static_assert(count_steps_until_end_from_string_data<GhostMap>(spooky_example_data) == 6);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        count_steps_until_end_from_string_data<NormalMap>,
        count_steps_until_end_from_string_data<GhostMap>
    );
}
