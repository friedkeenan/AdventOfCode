#include <advent/advent.hpp>

enum class RowDirection : std::uint8_t {
    Right,
    Left,
};

enum class ColumnDirection : std::uint8_t {
    Down,
    Up,
};

/* Row directions can never be equal to column directions. */
constexpr bool operator ==(const RowDirection, const ColumnDirection) {
    return false;
}

/* NOTE: If we had something like 'std::mdarray' then we'd do this differently. */
class TreeGrid {
    public:
        class Tree {
            public:
                /* We don't actually need to convert single characters into integers. */
                char height;

                /* NOTE: Do not rely on this being set unless YOU yourself set it. */
                bool visible = false;

                /* Implicit constructor for vector insertion ease. */
                constexpr explicit(false) Tree(const char height) : height(height) { }
        };

        std::vector<Tree> _tree_storage;

        std::size_t _grid_width = 0;

        template<std::ranges::input_range Rng>
        requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
        constexpr explicit TreeGrid(Rng &&tree_rows) {
            const std::string_view first_row = *std::ranges::begin(tree_rows);

            this->_grid_width = first_row.length();

            for (const std::string_view row : tree_rows) {
                if (row.empty()) {
                    continue;
                }

                [[assume(row.length() == this->grid_width())]];

                this->_tree_storage.insert(this->_tree_storage.end(), row.begin(), row.end());
            }
        }

        constexpr std::size_t grid_width() const {
            return this->_grid_width;
        }

        constexpr std::size_t grid_height() const {
            return this->_tree_storage.size() / this->grid_width();
        }

        constexpr Tree *data() {
            return this->_tree_storage.data();
        }

        constexpr const Tree *data() const {
            return this->_tree_storage.data();
        }

        constexpr auto begin() {
            return this->_tree_storage.begin();
        }

        constexpr auto begin() const {
            return this->_tree_storage.begin();
        }

        constexpr auto end() {
            return this->_tree_storage.end();
        }

        constexpr auto end() const {
            return this->_tree_storage.end();
        }

        constexpr std::size_t size() const {
            return this->_tree_storage.size();
        }

        constexpr Tree &first_in_row(const std::size_t row_index) {
            return this->_tree_storage[row_index * this->grid_width()];
        }

        constexpr const Tree &first_in_row(const std::size_t row_index) const {
            return this->_tree_storage[row_index * this->grid_width()];
        }

        constexpr Tree &last_in_row(const std::size_t row_index) {
            return this->_tree_storage[(row_index + 1) * this->grid_width() - 1];
        }

        constexpr const Tree &last_in_row(const std::size_t row_index) const {
            return this->_tree_storage[(row_index + 1) * this->grid_width() - 1];
        }

        constexpr Tree &first_in_column(const std::size_t column_index) {
            return this->_tree_storage[column_index];
        }

        constexpr const Tree &first_in_column(const std::size_t column_index) const {
            return this->_tree_storage[column_index];
        }

        constexpr Tree &last_in_column(const std::size_t column_index) {
            return this->_tree_storage[this->size() - this->grid_width() + column_index];
        }

        constexpr const Tree &last_in_column(const std::size_t column_index) const {
            return this->_tree_storage[this->size() - this->grid_width() + column_index];
        }

        constexpr Tree &_at(const std::size_t column_index, const std::size_t row_index) {
            [[assume(column_index < this->grid_width())]];
            [[assume(row_index    < this->grid_height())]];

            return this->_tree_storage[column_index * this->grid_width() + row_index];
        }

        constexpr const Tree &_at(const std::size_t column_index, const std::size_t row_index) const {
            [[assume(column_index < this->grid_width())]];
            [[assume(row_index    < this->grid_height())]];

            return this->_tree_storage[row_index * this->grid_width() + column_index];
        }

        constexpr Tree &operator [](const std::size_t column_index, const std::size_t row_index) {
            return this->_at(column_index, row_index);
        }

        constexpr const Tree &operator [](const std::size_t column_index, const std::size_t row_index) const {
            return this->_at(column_index, row_index);
        }

        constexpr void _for_each_inner_row_right(const std::size_t row_index, const auto &consumer) {
            const auto start = this->_tree_storage.begin() + row_index * this->grid_width() + 1;
            const auto end   = start + this->grid_width() - 2;

            for (auto it = start; it != end; ++it) {
                std::invoke(consumer, *it);
            }
        }

        constexpr void _for_each_inner_row_left(const std::size_t row_index, const auto &consumer) {
            const auto end   = this->_tree_storage.begin() + row_index * this->grid_width();
            const auto start = end + this->grid_width() - 2;

            for (auto it = start; it != end; --it) {
                std::invoke(consumer, *it);
            }
        }

        template<RowDirection Direction, typename Consumer>
        requires (std::invocable<const Consumer &, Tree &>)
        constexpr void for_each_inner_row(const std::size_t row_index, const Consumer consumer) {
            if constexpr (Direction == RowDirection::Right) {
                this->_for_each_inner_row_right(row_index, consumer);
            } else {
                this->_for_each_inner_row_left(row_index, consumer);
            }
        }

        constexpr void _for_each_inner_column_down(const std::size_t column_index, const auto &consumer) {
            const auto start_index   = column_index + this->grid_width();
            const auto too_far_index = this->size() - this->grid_width();

            for (auto raw_index = start_index; raw_index < too_far_index; raw_index += this->grid_width()) {
                std::invoke(consumer, this->_tree_storage[raw_index]);
            }
        }

        constexpr void _for_each_inner_column_up(const std::size_t column_index, const auto &consumer) {
            const auto start_index   = this->size() - this->grid_width() + column_index;
            const auto too_far_index = this->grid_width();

            for (auto raw_index = start_index; raw_index > too_far_index; raw_index -= this->grid_width()) {
                std::invoke(consumer, this->_tree_storage[raw_index]);
            }
        }

        template<ColumnDirection Direction, typename Consumer>
        requires (std::invocable<const Consumer &, Tree &>)
        constexpr void for_each_inner_column(const std::size_t column_index, const Consumer consumer) {
            if constexpr (Direction == ColumnDirection::Down) {
                this->_for_each_inner_column_down(column_index, consumer);
            } else {
                this->_for_each_inner_column_up(column_index, consumer);
            }
        }

        constexpr std::size_t _viewing_distance_from_right_at(const std::size_t column_index, const std::size_t row_index) const {
            const auto &main_tree = this->_at(column_index, row_index);

            std::size_t viewing_distance = 0;
            for (const auto i : std::views::iota(column_index + 1, this->grid_width())) {
                ++viewing_distance;

                if (this->_at(i, row_index).height >= main_tree.height) {
                    return viewing_distance;
                }
            }

            return viewing_distance;
        }

        constexpr std::size_t _viewing_distance_from_left_at(const std::size_t column_index, const std::size_t row_index) const {
            const auto &main_tree = this->_at(column_index, row_index);

            std::size_t viewing_distance = 0;

            auto i = column_index - 1;
            while (true) {
                ++viewing_distance;

                if (i == 0) {
                    return viewing_distance;
                }

                if (this->_at(i, row_index).height >= main_tree.height) {
                    return viewing_distance;
                }

                --i;
            }
        }

        constexpr std::size_t _viewing_distance_from_down_at(const std::size_t column_index, const std::size_t row_index) const {
            const auto &main_tree = this->_at(column_index, row_index);

            std::size_t viewing_distance = 0;
            for (const auto i : std::views::iota(row_index + 1, this->grid_height())) {
                ++viewing_distance;

                if (this->_at(column_index, i).height >= main_tree.height) {
                    return viewing_distance;
                }
            }

            return viewing_distance;
        }

        constexpr std::size_t _viewing_distance_from_up_at(const std::size_t column_index, const std::size_t row_index) const {
            const auto &main_tree = this->_at(column_index, row_index);

            std::size_t viewing_distance = 0;

            auto i = row_index - 1;
            while (true) {
                ++viewing_distance;

                if (i == 0) {
                    return viewing_distance;
                }

                if (this->_at(column_index, i).height >= main_tree.height) {
                    return viewing_distance;
                }

                --i;
            }
        }

        template<auto Direction>
        requires (
            Direction == RowDirection::Right ||
            Direction == RowDirection::Left  ||

            Direction == ColumnDirection::Down ||
            Direction == ColumnDirection::Up
        )
        constexpr std::size_t viewing_distance_at(const std::size_t column_index, const std::size_t row_index) const {
            if constexpr (Direction == RowDirection::Right) {
                return this->_viewing_distance_from_right_at(column_index, row_index);
            } else if constexpr (Direction == RowDirection::Left) {
                return this->_viewing_distance_from_left_at(column_index, row_index);
            } else if constexpr (Direction == ColumnDirection::Down) {
                return this->_viewing_distance_from_down_at(column_index, row_index);
            } else {
                return this->_viewing_distance_from_up_at(column_index, row_index);
            }
        }
};

static_assert(std::ranges::contiguous_range<TreeGrid> && std::ranges::sized_range<TreeGrid>);

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t num_visible_trees(Rng &&tree_rows) {
    auto trees = TreeGrid(std::forward<Rng>(tree_rows));

    /* All outer trees are visible. */
    std::size_t visible_trees = 2 * trees.grid_width() + 2 * trees.grid_height() - 4;

    /* Reused and reset multiple times. */
    TreeGrid::Tree *highest_so_far = nullptr;
    auto count_and_mark_visible_trees = [&](auto &tree) {
        if (tree.height > highest_so_far->height) {
            highest_so_far = &tree;

            if (!tree.visible) {
                ++visible_trees;

                tree.visible = true;
            }
        }
    };

    for (const auto row_index : std::views::iota(1uz, trees.grid_height() - 1)) {
        highest_so_far = &trees.first_in_row(row_index);
        trees.for_each_inner_row<RowDirection::Right>(row_index, count_and_mark_visible_trees);

        highest_so_far = &trees.last_in_row(row_index);
        trees.for_each_inner_row<RowDirection::Left>(row_index, count_and_mark_visible_trees);
    }

    for (const auto column_index : std::views::iota(1uz, trees.grid_width() - 1)) {
        highest_so_far = &trees.first_in_column(column_index);
        trees.for_each_inner_column<ColumnDirection::Down>(column_index, count_and_mark_visible_trees);

        highest_so_far = &trees.last_in_column(column_index);
        trees.for_each_inner_column<ColumnDirection::Up>(column_index, count_and_mark_visible_trees);
    }

    return visible_trees;
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t max_scenic_score(Rng &&tree_rows) {
    const auto trees = TreeGrid(std::forward<Rng>(tree_rows));

    /* NOTE: We don't need to care about the outer trees as their scenic score is always 0. */
    std::size_t highest_scenic_score = 0;
    for (const auto row_index : std::views::iota(1uz, trees.grid_height() - 1)) {
        for (const auto column_index : std::views::iota(1uz, trees.grid_width() - 1)) {
            std::size_t scenic_score = 1;

            scenic_score *= trees.viewing_distance_at<RowDirection::Right>  (column_index, row_index);
            scenic_score *= trees.viewing_distance_at<RowDirection::Left>   (column_index, row_index);
            scenic_score *= trees.viewing_distance_at<ColumnDirection::Down>(column_index, row_index);
            scenic_score *= trees.viewing_distance_at<ColumnDirection::Up>  (column_index, row_index);

            if (scenic_score > highest_scenic_score) {
                highest_scenic_score = scenic_score;
            }
        }
    }

    return highest_scenic_score;
}

constexpr std::size_t num_visible_trees_from_string_data(const std::string_view data) {
    return num_visible_trees(advent::views::split_lines(data));
}

constexpr std::size_t max_scenic_score_from_string_data(const std::string_view data) {
    return max_scenic_score(advent::views::split_lines(data));
}

constexpr inline std::string_view example_data = (
    "30373\n"
    "25512\n"
    "65332\n"
    "33549\n"
    "35390\n"
);

static_assert(num_visible_trees_from_string_data(example_data) == 21);
static_assert(max_scenic_score_from_string_data(example_data) == 8);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        num_visible_trees_from_string_data,
        max_scenic_score_from_string_data
    );
}
