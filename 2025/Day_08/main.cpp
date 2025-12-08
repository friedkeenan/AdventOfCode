import std;
import advent;

struct JunctionBoxes {
    using Coords = advent::vector_nd<std::size_t, 3>;

    struct BoxIndices {
        std::size_t first;
        std::size_t second;
    };

    struct CircuitConnections {
        std::vector<std::size_t> indices;

        constexpr explicit CircuitConnections(const std::size_t num_boxes)
        :
            indices(std::from_range, std::views::iota(0uz, num_boxes))
        {}

        constexpr std::size_t operator [](this const CircuitConnections &self, const std::size_t box_index) {
            return self.indices[box_index];
        }

        constexpr void replace(this CircuitConnections &self, const std::size_t old_index, const std::size_t new_index) {
            if (old_index == new_index) {
                return;
            }

            std::ranges::replace(self.indices, old_index, new_index);
        }

        constexpr std::vector<std::size_t> lengths(this const CircuitConnections &self) {
            auto lengths = std::vector<std::size_t>(self.indices.size());

            for (const auto index : self.indices) {
                lengths[index] += 1;
            }

            return lengths;
        }

        constexpr bool all_same_circuit(this const CircuitConnections &self) {
            return std::ranges::all_of(self.indices, [&](const auto index) {
                return index == self.indices.front();
            });
        }
    };

    static constexpr std::size_t MaxDistance = -1;

    std::size_t num_boxes;
    std::vector<std::size_t> distance_storage;

    template<advent::string_viewable_range Rng>
    static constexpr std::vector<Coords> ParseBoxLocations(Rng &&rng) {
        std::vector<Coords> result;

        for (const std::string_view line : std::forward<Rng>(rng)) {
            if (line.empty()) {
                continue;
            }

            auto &coords = result.emplace_back();

            std::size_t i = 0;
            advent::split_for_each(line, ',', [&](const auto coord) {
                coords[i] = advent::to_integral<std::size_t>(coord);

                i += 1;
            });
        }

        return result;
    }

    static constexpr std::size_t StorageSizeForBoxCount(const std::size_t num_boxes) {
        /*
            We take advantage of the fact that the distance
            is commutative to only store about half the values.
        */

        [[assume(num_boxes >= 2)]];

        return 1 + (num_boxes + 1) * (num_boxes - 2) / 2;
    }

    constexpr explicit JunctionBoxes(const std::vector<Coords> &locations) {
        this->num_boxes = locations.size();

        this->distance_storage.resize(StorageSizeForBoxCount(this->num_boxes));

        this->for_each_distance([&](const auto indices, std::size_t &distance) {
            distance = locations[indices.first].dist_squared(locations[indices.second]);
        });
    }

    template<typename Consumer>
    requires (std::invocable<Consumer &, BoxIndices, std::size_t &>)
    constexpr void for_each_distance(this JunctionBoxes &self, Consumer &&consumer) {
        std::size_t storage_index = 0;
        for (const auto first : std::views::iota(0uz, self.num_boxes)) {
            for (const auto second : std::views::iota(first + 1, self.num_boxes)) {
                std::invoke(consumer, BoxIndices{first, second}, self.distance_storage[storage_index]);

                storage_index += 1;
            }
        }
    }

    /* TODO: Do we need this? */
    constexpr std::size_t index_for_distance(this const JunctionBoxes &self, std::size_t first, std::size_t second) {
        [[assume(first != second)]];

        if (second < first) {
            std::ranges::swap(first, second);
        }

        return self.num_boxes * first + second - ((first + 1) * (first + 2)) / 2;
    }

    constexpr std::size_t distance_between(this const JunctionBoxes &self, const std::size_t first, const std::size_t second) {
        if (first == second) {
            return 0;
        }

        return self.distance_storage[self.index_for_distance(first, second)];
    }

    constexpr auto boxes_with_minimum_distance(this JunctionBoxes &self) {
        struct DistanceInfo {
            BoxIndices indices;
            std::size_t &distance;
        };

        auto current_min_distance = &self.distance_storage[0];
        auto current_min_indices  = BoxIndices{0, 1};

        self.for_each_distance([&](const auto indices, std::size_t &distance) {
            if (distance < *current_min_distance) {
                current_min_distance = &distance;
                current_min_indices  = indices;
            }
        });

        return DistanceInfo{current_min_indices, *current_min_distance};
    }

    constexpr BoxIndices step_circuit_connections(this JunctionBoxes &self, CircuitConnections &circuit_indices) {
        const auto [box_indices, distance] = self.boxes_with_minimum_distance();

        distance = MaxDistance;

        const auto first_circuit  = circuit_indices[box_indices.first];
        const auto second_circuit = circuit_indices[box_indices.second];

        circuit_indices.replace(second_circuit, first_circuit);

        return box_indices;
    }

    template<std::size_t Connections>
    constexpr std::vector<std::size_t> circuit_lengths(this JunctionBoxes &&self) {
        auto circuit_connections = CircuitConnections(self.num_boxes);

        for (auto _ : std::views::iota(0uz, Connections)) {
            self.step_circuit_connections(circuit_connections);
        }

        /*
            NOTE: Tracking the circuit lengths
            as we iterate is not faster.
        */
        return circuit_connections.lengths();
    }

    constexpr BoxIndices connect_all(this JunctionBoxes &&self) {
        auto circuit_connections = CircuitConnections(self.num_boxes);

        while (true) {
            const auto connected = self.step_circuit_connections(circuit_connections);

            if (circuit_connections.all_same_circuit()) {
                return connected;
            }
        }
    }
};

template<std::size_t AmountLargest, std::size_t Connections, advent::string_viewable_range Rng>
constexpr std::size_t multiply_largest_circuit_lengths(Rng &&rng) {
    auto circuit_lengths = JunctionBoxes(JunctionBoxes::ParseBoxLocations(std::forward<Rng>(rng)))
        .circuit_lengths<Connections>();

    const auto max_lengths = advent::find_maxes<AmountLargest>(circuit_lengths);

    return std::ranges::fold_left(max_lengths, 1uz, std::multiplies{});
}

template<advent::string_viewable_range Rng>
constexpr std::size_t multiply_last_wall_distances(Rng &&rng) {
    const auto box_locations = JunctionBoxes::ParseBoxLocations(std::forward<Rng>(rng));

    const auto last_connected = JunctionBoxes(box_locations).connect_all();

    return box_locations[last_connected.first].x() * box_locations[last_connected.second].x();
}

template<std::size_t AmountLargest, std::size_t Connections>
constexpr std::size_t multiply_largest_circuit_lengths_from_string_data(const std::string_view data) {
    return multiply_largest_circuit_lengths<AmountLargest, Connections>(data | advent::views::split_lines);
}

constexpr std::size_t multiply_last_wall_distances_from_string_data(const std::string_view data) {
    return multiply_last_wall_distances(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "162,817,812\n"
    "57,618,57\n"
    "906,360,560\n"
    "592,479,940\n"
    "352,342,300\n"
    "466,668,158\n"
    "542,29,236\n"
    "431,825,988\n"
    "739,650,466\n"
    "52,470,668\n"
    "216,146,977\n"
    "819,987,18\n"
    "117,168,530\n"
    "805,96,715\n"
    "346,949,466\n"
    "970,615,88\n"
    "941,993,340\n"
    "862,61,35\n"
    "984,92,344\n"
    "425,690,689\n"
);

static_assert(multiply_largest_circuit_lengths_from_string_data<3, 10>(example_data) == 40);

static_assert(multiply_last_wall_distances_from_string_data(example_data) == 25272);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        multiply_largest_circuit_lengths_from_string_data<3, 1000>,
        multiply_last_wall_distances_from_string_data
    );
}
