#include <advent/advent.hpp>

struct HeightMap {
    /* NOTE: We can just keep our heights as 'char'. */

    using Node = const char *;

    static constexpr char TrailStart = '0';
    static constexpr char TrailEnd   = '9';

    advent::string_view_grid map;

    constexpr explicit HeightMap(const std::string_view description) : map(description) {}

    static constexpr bool _can_travel_to(const Node from, const Node to) {
        return *to == (*from + 1);
    }

    constexpr void _mark_trail_ends(this const HeightMap &self, advent::grid<bool> &trail_ends, const Node node) {
        if (*node == TrailEnd) {
            trail_ends[self.map.coords_of(node)] = true;

            return;
        }

        for (const auto [_, neighbor] : self.map.adjacent_neighbors_of(node)) {
            if (!_can_travel_to(node, neighbor)) {
                continue;
            }

            self._mark_trail_ends(trail_ends, neighbor);
        }
    }

    static constexpr std::size_t _count_and_reset(std::span<bool> elements) {
        std::size_t count = 0;

        for (auto &elem : elements) {
            if (!elem) {
                continue;
            }

            count += 1;
            elem = false;
        }

        return count;
    }

    constexpr std::size_t cumulative_trailhead_score(this const HeightMap &self) {
        std::size_t score = 0;

        /* TODO: Investigate using a 'std::flat_set' when I receive an implementation. */
        auto trail_ends = advent::grid<bool>(self.map.width(), self.map.height());
        for (const auto &elem : self.map.elements()) {
            if (elem != TrailStart) {
                continue;
            }

            self._mark_trail_ends(trail_ends, &elem);

            const auto individual_score = _count_and_reset(trail_ends.elements());

            score += individual_score;
        }

        return score;
    }

    constexpr std::size_t _trailhead_rating(this const HeightMap &self, const Node node) {
        if (*node == TrailEnd) {
            return 1;
        }

        std::size_t rating = 0;

        for (const auto [_, neighbor] : self.map.adjacent_neighbors_of(node)) {
            if (!_can_travel_to(node, neighbor)) {
                continue;
            }

            rating += self._trailhead_rating(neighbor);
        }

        return rating;
    }

    constexpr std::size_t cumulative_trailhead_rating(this const HeightMap &self) {
        std::size_t rating = 0;

        for (const auto &elem : self.map.elements()) {
            if (elem != TrailStart) {
                continue;
            }

            rating += self._trailhead_rating(&elem);
        }

        return rating;
    }
};

constexpr std::size_t cumulative_trailhead_score(const std::string_view data) {
    return HeightMap(data).cumulative_trailhead_score();
}

constexpr std::size_t cumulative_trailhead_rating(const std::string_view data) {
    return HeightMap(data).cumulative_trailhead_rating();
}

constexpr inline std::string_view example_data = (
    "89010123\n"
    "78121874\n"
    "87430965\n"
    "96549874\n"
    "45678903\n"
    "32019012\n"
    "01329801\n"
    "10456732\n"
);

static_assert(cumulative_trailhead_score(example_data) == 36);
static_assert(cumulative_trailhead_rating(example_data) == 81);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        cumulative_trailhead_score,
        cumulative_trailhead_rating
    );
}
