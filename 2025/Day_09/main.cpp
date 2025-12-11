import std;
import advent;

using Coords = advent::vector_2d<advent::ssize_t>;

struct Rectangle {
    Coords first_corner;
    Coords second_corner;

    constexpr advent::ssize_t area(this const Rectangle &self) {
        auto displacement = self.first_corner - self.second_corner;

        for (auto &coord : displacement) {
            coord = advent::abs(coord) + 1;
        }

        return displacement.x() * displacement.y();
    }
};

struct TileGrid {
    struct CoordRange {
        advent::ssize_t first;
        advent::ssize_t second;

        constexpr CoordRange(const advent::ssize_t first, const advent::ssize_t second)
        :
            first(first),
            second(second)
        {
            if (this->second < this->first) {
                std::ranges::swap(this->first, this->second);
            }
        }

        constexpr bool contains(this const CoordRange self, const advent::ssize_t coord) {
            return coord >= self.first && coord <= self.second;
        }

        constexpr auto coords(this const CoordRange self) {
            return std::views::iota(self.first, self.second + 1);
        }
    };

    static constexpr Coords ParseCoords(const std::string_view description) {
        const auto separator_pos = description.find_first_of(',');
        [[assume(separator_pos != std::string_view::npos)]];

        return Coords{
            advent::to_integral<advent::ssize_t>(description.substr(0, separator_pos)),
            advent::to_integral<advent::ssize_t>(description.substr(separator_pos + 1))
        };
    }

    template<advent::string_viewable_range Rng>
    static constexpr std::vector<Coords> ParseCorners(Rng &&rng) {
        std::vector<Coords> result;

        for (const std::string_view line : std::forward<Rng>(rng)) {
            if (line.empty()) {
                continue;
            }

            result.push_back(ParseCoords(line));
        }

        return result;
    }

    std::vector<Coords> corners;

    template<advent::string_viewable_range Rng>
    constexpr explicit TileGrid(Rng &&rng) : corners(ParseCorners(std::forward<Rng>(rng))) {}

    constexpr auto corner_combinations(this const TileGrid &self) {
        return std::views::cartesian_product(self.corners, self.corners);
    }

    template<advent::dimension BoundDimension>
    constexpr auto find_bounds(this const TileGrid &self, const advent::ssize_t opposite_coord) {
        static constexpr auto OppositeDimension = []() {
            if constexpr (BoundDimension == advent::dimension::x) {
                return advent::dimension::y;
            } else {
                static_assert(BoundDimension == advent::dimension::y);

                return advent::dimension::x;
            }
        }();

        struct Bounds {
            std::vector<CoordRange> ranges;

            constexpr bool coord_is_contained(this const Bounds &self, const advent::ssize_t coord) {
                return std::ranges::any_of(self.ranges, [&](const auto range) {
                    return range.contains(coord);
                });
            }
        };

        std::vector<advent::ssize_t> filled_edges;

        for (const auto [first, second] : std::views::concat(self.corners, std::views::single(self.corners.front())) | std::views::pairwise) {
            const auto range = CoordRange(first[OppositeDimension], second[OppositeDimension]);
            if (!range.contains(opposite_coord)) {
                continue;
            }

            if (range.first == range.second) {
                auto edge = CoordRange(first[BoundDimension], second[BoundDimension]);

                [[assume(edge.first != edge.second)]];

                /* The ends will be covered by another corner pair. */
                edge.first  += 1;
                edge.second -= 1;

                filled_edges.append_range(edge.coords());

                continue;
            }

            filled_edges.push_back(first[BoundDimension]);
        }

        std::ranges::sort(filled_edges);

        Bounds bounds;

        auto it = filled_edges.begin();
        while (it != filled_edges.end()) {
            const auto first = *it;
            ++it;

            /* I don't know when this would happen, but it's needed apparently. */
            if (it == filled_edges.end()) {
                bounds.ranges.emplace_back(first, first);

                return bounds;
            }

            while (true) {
                if (*it != *(it - 1) + 1) {
                    break;
                }

                ++it;

                if (it == filled_edges.end()) {
                    bounds.ranges.emplace_back(first, *(it - 1));

                    return bounds;
                }
            }

            bounds.ranges.emplace_back(first, *it);
            ++it;
        }

        return bounds;
    }

    constexpr auto x_bounds_at_y(this const TileGrid &self, const advent::ssize_t y) {
        return self.find_bounds<advent::dimension::x>(y);
    }

    constexpr auto y_bounds_at_x(this const TileGrid &self, const advent::ssize_t x) {
        return self.find_bounds<advent::dimension::y>(x);
    }

    constexpr bool contains(this const TileGrid &self, const Rectangle &rect) {
        const auto first_x_bounds  = self.x_bounds_at_y(rect.first_corner.y());
        const auto second_x_bounds = self.x_bounds_at_y(rect.second_corner.y());

        for (const auto x : CoordRange(rect.first_corner.x(), rect.second_corner.x()).coords()) {
            if (!first_x_bounds.coord_is_contained(x)) {
                return false;
            }
        }

        const auto first_y_bounds  = self.y_bounds_at_x(rect.first_corner.x());
        const auto second_y_bounds = self.y_bounds_at_x(rect.second_corner.x());

        for (const auto y : CoordRange(rect.first_corner.y(), rect.second_corner.y()).coords()) {
            if (!first_y_bounds.coord_is_contained(y)) {
                return false;
            }

            if (!second_y_bounds.coord_is_contained(y)) {
                return false;
            }
        }

        return true;
    }
};

template<advent::string_viewable_range Rng>
constexpr advent::ssize_t max_unbounded_area_of_rectangles(Rng &&rng) {
    const auto tiles = TileGrid(std::forward<Rng>(rng));

    advent::ssize_t current_max_area = 0;

    for (const auto [first_corner, second_corner] : tiles.corner_combinations()) {
        const auto area = Rectangle{first_corner, second_corner}.area();

        if (area > current_max_area) {
            current_max_area = area;
        }
    }

    return current_max_area;
}

template<advent::string_viewable_range Rng>
constexpr advent::ssize_t max_bounded_area_of_rectangles(Rng &&rng) {
    /*
        I initially solved this by storing a grid of booleans
        that I filled in and which took up over half of my RAM.

        I don't do that anymore. This new solution is *much*
        better on memory but is slower, and the first solution
        was already pretty slow. But I'm done spending time on it.

        So yeah. Sorry.
    */

    const auto tiles = TileGrid(std::forward<Rng>(rng));

    advent::ssize_t current_max_area = 0;

    for (const auto [first_corner, second_corner] : tiles.corner_combinations()) {
        const auto rect = Rectangle{first_corner, second_corner};

        if (!tiles.contains(rect)) {
            continue;
        }

        const auto area = rect.area();

        if (area > current_max_area) {
            current_max_area = area;
        }
    }

    return current_max_area;
}

constexpr advent::ssize_t max_unbounded_area_of_rectangles_from_string_data(const std::string_view data) {
    return max_unbounded_area_of_rectangles(data | advent::views::split_lines);
}

constexpr advent::ssize_t max_bounded_area_of_rectangles_from_string_data(const std::string_view data) {
    return max_bounded_area_of_rectangles(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "7,1\n"
    "11,1\n"
    "11,7\n"
    "9,7\n"
    "9,5\n"
    "2,5\n"
    "2,3\n"
    "7,3\n"
);

static_assert(max_unbounded_area_of_rectangles_from_string_data(example_data) == 50);
static_assert(max_bounded_area_of_rectangles_from_string_data(example_data)   == 24);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        max_unbounded_area_of_rectangles_from_string_data,
        max_bounded_area_of_rectangles_from_string_data
    );
}
