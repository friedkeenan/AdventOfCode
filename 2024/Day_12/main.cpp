import std;
import advent;

struct Garden {
    struct Region {
        std::size_t area      = 0;
        std::size_t perimeter = 0;

        constexpr std::size_t fencing_price(this const Region self) {
            return self.area * self.perimeter;
        }
    };

    struct RegionTracker {
        using Coords = advent::vector_2d<std::size_t>;

        static constexpr std::size_t NoRegion = -1;

        std::vector<Region>       regions;
        advent::grid<std::size_t> region_indices;

        constexpr RegionTracker(const Garden &garden)
        :
            regions(),
            region_indices(garden.plots.width(), garden.plots.height(), NoRegion)
        {}

        constexpr void subsume_region(this RegionTracker &self, std::size_t new_region_index, std::size_t old_region_index) {
            [[assume(new_region_index != old_region_index)]];

            auto &new_region = self.regions[new_region_index];
            auto &old_region = self.regions[old_region_index];

            new_region.area      += old_region.area;
            new_region.perimeter += old_region.perimeter;

            /* NOTE: We don't delete the region, we just make it inert. */
            old_region = Region{};

            std::ranges::replace(self.region_indices.elements(), old_region_index, new_region_index);
        }

        constexpr void mark_same_region(this RegionTracker &self, const Coords target, const Coords neighbor) {
            auto &target_region_index   = self.region_indices[target];
            auto &neighbor_region_index = self.region_indices[neighbor];

            if (target_region_index == neighbor_region_index) {
                return;
            }

            if (neighbor_region_index == NoRegion) {
                return;
            }

            if (target_region_index == NoRegion) {
                target_region_index = neighbor_region_index;

                self.regions[target_region_index].area += 1;

                return;
            }

            self.subsume_region(target_region_index, neighbor_region_index);
        }

        constexpr Region &push_new_region_for(this RegionTracker &self, const Coords target) {
            self.region_indices[target] = self.regions.size();

            return self.regions.emplace_back(1uz, 0uz);
        }

        constexpr Region &get_or_make_region_for(this RegionTracker &self, const Coords target) {
            const auto region_index = self.region_indices[target];

            if (region_index == NoRegion) {
                return self.push_new_region_for(target);
            }

            return self.regions[region_index];
        }
    };

    advent::string_view_grid plots;

    constexpr explicit Garden(const std::string_view plots) : plots(plots) {}

    constexpr std::size_t fencing_price_without_discount(this const Garden &self) {
        auto tracker = RegionTracker(self);

        for (const auto [coords, plot] : self.plots.enumerate()) {
            [[assume(tracker.region_indices[coords] == tracker.NoRegion)]];

            std::size_t added_perimeter = 4;

            for (const auto [_, neighbor] : self.plots.adjacent_neighbors_of(&plot)) {
                if (*neighbor != plot) {
                    continue;
                }

                --added_perimeter;

                const auto neighbor_coords = self.plots.coords_of(neighbor);

                tracker.mark_same_region(coords, neighbor_coords);
            }

            auto &region = tracker.get_or_make_region_for(coords);

            region.perimeter += added_perimeter;
        }

        return std::ranges::fold_left(tracker.regions | std::views::transform(&Region::fencing_price), 0uz, std::plus{});
    }
};

constexpr std::size_t find_fencing_price_of_garden_without_discount(const std::string_view data) {
    return Garden(data).fencing_price_without_discount();
}

constexpr inline std::string_view simple_example_data = (
    "AAAA\n"
    "BBCD\n"
    "BBCC\n"
    "EEEC\n"
);

static_assert(find_fencing_price_of_garden_without_discount(simple_example_data) == 140);

constexpr inline std::string_view disjointed_example_data = (
    "OOOOO\n"
    "OXOXO\n"
    "OOOOO\n"
    "OXOXO\n"
    "OOOOO\n"
);

static_assert(find_fencing_price_of_garden_without_discount(disjointed_example_data) == 772);

constexpr inline std::string_view complex_example_data = (
    "RRRRIICCFF\n"
    "RRRRIICCCF\n"
    "VVRRRCCFFF\n"
    "VVRCCCJFFF\n"
    "VVVVCJJCFE\n"
    "VVIVCCJJEE\n"
    "VVIIICJJEE\n"
    "MIIIIIJJEE\n"
    "MIIISIJEEE\n"
    "MMMISSJEEE\n"
);

static_assert(find_fencing_price_of_garden_without_discount(complex_example_data) == 1930);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        find_fencing_price_of_garden_without_discount
    );
}
