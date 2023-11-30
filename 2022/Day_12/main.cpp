#include <advent/advent.hpp>

enum class Side : std::uint8_t {
    Top,
    Bottom,
    Left,
    Right,
};

class HeightGrid {
    public:
        static constexpr char StartSignifier = 'S';
        static constexpr char EndSignifier   = 'E';

        class Node {
            public:
                /* Use a pointer so we can be default constructible and easily assignable. */
                const HeightGrid &_grid;

                std::size_t index;

                constexpr char height() const {
                    return this->_grid.height_at(this->index);
                }

                constexpr bool is_end() const {
                    return this->_grid.raw_height_at(this->index) == EndSignifier;
                }

                template<Side Edge>
                constexpr bool at_edge() const {
                    if constexpr (Edge == Side::Top) {
                        return this->index < this->_grid.grid_width();
                    } else if constexpr (Edge == Side::Bottom) {
                        return this->index >= (this->_grid.size() - this->_grid.grid_width());
                    } else if constexpr (Edge == Side::Left) {
                        return this->index % this->_grid.grid_width() == 0;
                    } else {
                        /* Right. */

                        return (this->index + 1) % this->_grid.grid_width() == 0;
                    }
                }

                template<Side Neighbor>
                constexpr bool has_neighbor() const {
                    return !this->at_edge<Neighbor>();
                }

                template<Side Neighbor>
                constexpr std::size_t neighbor_index() const {
                    if constexpr (Neighbor == Side::Top) {
                        return this->index - this->_grid.grid_width();
                    } else if constexpr (Neighbor == Side::Bottom) {
                        return this->index + this->_grid.grid_width();
                    } else if constexpr (Neighbor == Side::Left) {
                        return this->index - 1;
                    } else {
                        /* Right. */

                        return this->index + 1;
                    }
                }

                template<Side Neighbor>
                constexpr void _maybe_consume_neighbor(const auto &consumer) const {
                    if (!this->has_neighbor<Neighbor>()) {
                        return;
                    }

                    const auto neighbor_index  = this->neighbor_index<Neighbor>();
                    const auto neighbor_height = this->_grid.height_at(neighbor_index);

                    if (this->height() < neighbor_height - 1) {
                        return;
                    }

                    std::invoke(consumer, Node{this->_grid, neighbor_index});
                }

                template<typename Consumer>
                requires (std::invocable<const Consumer &, Node>)
                constexpr void for_each_traversable_neighbor(const Consumer consumer) const {
                    this->_maybe_consume_neighbor<Side::Top>   (consumer);
                    this->_maybe_consume_neighbor<Side::Bottom>(consumer);
                    this->_maybe_consume_neighbor<Side::Left>  (consumer);
                    this->_maybe_consume_neighbor<Side::Right> (consumer);
                }
        };

        /* We don't need to convert the characters to numbers. */
        std::vector<char> _heights;

        std::size_t _grid_width;

        template<std::ranges::input_range Rng>
        requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
        constexpr HeightGrid(Rng &&height_rows) {
            const std::string_view first_row = *std::ranges::begin(height_rows);

            this->_grid_width = first_row.length();

            for (const std::string_view row : height_rows) {
                if (row.empty()) {
                    continue;
                }

                [[assume(row.length() == this->_grid_width)]];

                this->_heights.insert(this->_heights.end(), row.begin(), row.end());
            }
        }

        constexpr std::size_t size() const {
            return this->_heights.size();
        }

        constexpr std::size_t grid_width() const {
            return this->_grid_width;
        }

        constexpr char raw_height_at(const std::size_t index) const {
            return this->_heights[index];
        }

        constexpr char height_at(const std::size_t index) const {
            const auto raw_height = this->raw_height_at(index);

            if (raw_height == StartSignifier) {
                return 'a';
            }

            if (raw_height == EndSignifier) {
                return 'z';
            }

            return raw_height;
        }

        constexpr Node node_at(const std::size_t index) const {
            return Node{*this, index};
        }

        constexpr std::size_t _index_with_raw_height(const char height) const {
            const auto it = std::ranges::find(this->_heights, height);
            [[assume(it >= this->_heights.begin())]];

            return static_cast<std::size_t>(it - this->_heights.begin());
        }

        constexpr std::size_t start_index() const {
            return this->_index_with_raw_height(StartSignifier);
        }
};

constexpr inline std::size_t infinite_distance = -1;

class DijkstraNode {
    public:
        std::size_t distance_from_start = infinite_distance;
        bool        visited             = false;
};

constexpr std::size_t shortest_distance_to_end_from_index(const HeightGrid &grid, const std::size_t start_index) {
    /* This code is not good. */

    auto dijkstra_nodes = std::vector<DijkstraNode>(grid.size());

    dijkstra_nodes[start_index].distance_from_start = 0;

    /* We keep a stack here instead of using recursion to avoid a stack overflow. */
    std::vector<std::size_t> to_visit_stack = {start_index};
    while (!to_visit_stack.empty()) {
        const auto current_index = to_visit_stack.back();
        to_visit_stack.pop_back();

        auto &current_dijkstra  = dijkstra_nodes[current_index];
        const auto current_node = grid.node_at(current_index);
        if (current_node.is_end()) {
            return current_dijkstra.distance_from_start;
        }

        std::vector<std::size_t> unvisited_neighbors;

        /* We have at most 4 unvisited neighbors. */
        unvisited_neighbors.reserve(4);

        current_node.for_each_traversable_neighbor([&](HeightGrid::Node neighbor) {
            auto &neighbor_dijkstra = dijkstra_nodes[neighbor.index];
            if (neighbor_dijkstra.visited) {
                return;
            }

            const auto tentative_distance = current_dijkstra.distance_from_start + 1;
            if (tentative_distance < neighbor_dijkstra.distance_from_start) {
                neighbor_dijkstra.distance_from_start = tentative_distance;
            }

            unvisited_neighbors.push_back(neighbor.index);
        });

        current_dijkstra.visited = true;

        /* Locally sort our unvisited neighbors. We want the shortest distance index at the end. */
        std::ranges::sort(unvisited_neighbors, std::ranges::greater{}, [&](std::size_t index) {
            return dijkstra_nodes[index].distance_from_start;
        });

        to_visit_stack.insert(to_visit_stack.end(), unvisited_neighbors.begin(), unvisited_neighbors.end());
    }

    return infinite_distance;
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t shortest_distance_to_end(Rng &&height_rows) {
    const auto grid = HeightGrid(std::forward<Rng>(height_rows));

    return shortest_distance_to_end_from_index(grid, grid.start_index());
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t shortest_distance_from_lowest_to_end(Rng &&height_rows) {
    const auto grid = HeightGrid(std::forward<Rng>(height_rows));

    auto shortest_distance = infinite_distance;

    /* This is gonna result in a lot of duplicated work I feel, but I don't care. */
    for (const auto i : std::views::iota(0uz, grid.size())) {
        if (grid.height_at(i) != 'a') {
            continue;
        }

        const auto distance = shortest_distance_to_end_from_index(grid, i);
        if (distance < shortest_distance) {
            shortest_distance = distance;
        }
    }

    return shortest_distance;
}

constexpr std::size_t shortest_distance_to_end_from_string_data(const std::string_view data) {
    return shortest_distance_to_end(data | advent::views::split_lines);
}

constexpr std::size_t shortest_distance_from_lowest_to_end_from_string_data(const std::string_view data) {
    return shortest_distance_from_lowest_to_end(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "Sabqponm\n"
    "abcryxxl\n"
    "accszExk\n"
    "acctuvwj\n"
    "abdefghi\n"
);

static_assert(shortest_distance_to_end_from_string_data(example_data) == 31);
static_assert(shortest_distance_from_lowest_to_end_from_string_data(example_data) == 29);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        shortest_distance_to_end_from_string_data,
        shortest_distance_from_lowest_to_end_from_string_data
    );
}
