#include <advent/defines.hpp>

export module advent:grid;

import std;

import :vector_nd;
import :regular_vector;

namespace advent {

    export enum class neighbor : std::uint8_t {
        above_left,
        above,
        above_right,
        left,
        right,
        below_left,
        below,
        below_right,
    };

    export enum class adjacent_neighbor : std::uint8_t {
        above = std::to_underlying(advent::neighbor::above),
        left  = std::to_underlying(advent::neighbor::left),
        right = std::to_underlying(advent::neighbor::right),
        below = std::to_underlying(advent::neighbor::below),
    };

    export enum class diagonal_neighbor : std::uint8_t {
        above_left  = std::to_underlying(advent::neighbor::above_left),
        above_right = std::to_underlying(advent::neighbor::above_right),
        below_left  = std::to_underlying(advent::neighbor::below_left),
        below_right = std::to_underlying(advent::neighbor::below_right),
    };

    export template<typename T>
    concept neighbor_enum = (
        std::same_as<T, advent::neighbor>          ||
        std::same_as<T, advent::adjacent_neighbor> ||
        std::same_as<T, advent::diagonal_neighbor>
    );

    export constexpr bool operator ==(const advent::neighbor lhs, const advent::adjacent_neighbor rhs) noexcept {
        return std::to_underlying(lhs) == std::to_underlying(rhs);
    }

    export constexpr bool operator ==(const advent::neighbor lhs, const advent::diagonal_neighbor rhs) noexcept {
        return std::to_underlying(lhs) == std::to_underlying(rhs);
    }

    static_assert(advent::neighbor::above == advent::adjacent_neighbor::above);
    static_assert(advent::neighbor::left  == advent::adjacent_neighbor::left);
    static_assert(advent::neighbor::right == advent::adjacent_neighbor::right);
    static_assert(advent::neighbor::below == advent::adjacent_neighbor::below);

    static_assert(advent::neighbor::above_left  == advent::diagonal_neighbor::above_left);
    static_assert(advent::neighbor::above_right == advent::diagonal_neighbor::above_right);
    static_assert(advent::neighbor::below_left  == advent::diagonal_neighbor::below_left);
    static_assert(advent::neighbor::below_right == advent::diagonal_neighbor::below_right);

    /*
        NOTE: This was originally a variable template, but
        due to a bug, the different specializations weren't
        being curried to the linker. So it was converted to
        a function.

        TODO: Should it be changed back when available?
    */
    export template<advent::neighbor_enum Position>
    consteval auto neighbor_positions() {
        if constexpr (std::same_as<Position, advent::neighbor>) {
            return std::array{
                advent::neighbor::above_left,
                advent::neighbor::above,
                advent::neighbor::above_right,
                advent::neighbor::left,
                advent::neighbor::right,
                advent::neighbor::below_left,
                advent::neighbor::below,
                advent::neighbor::below_right
            };
        } else if constexpr (std::same_as<Position, advent::adjacent_neighbor>) {
            return std::array{
                advent::adjacent_neighbor::above,
                advent::adjacent_neighbor::left,
                advent::adjacent_neighbor::right,
                advent::adjacent_neighbor::below
            };
        } else if constexpr (std::same_as<Position, advent::diagonal_neighbor>) {
            return std::array{
                advent::diagonal_neighbor::above_left,
                advent::diagonal_neighbor::above_right,
                advent::diagonal_neighbor::below_left,
                advent::diagonal_neighbor::below_right
            };
        }
    }

    export constexpr advent::adjacent_neighbor next_clockwise_neighbor(const advent::adjacent_neighbor position) {
        switch (position) {
            using enum advent::adjacent_neighbor;

            case above: return right;
            case right: return below;
            case below: return left;
            case left:  return above;

            default: std::unreachable();
        }
    }

    export constexpr advent::adjacent_neighbor opposite_neighbor(const advent::adjacent_neighbor position) {
        switch (position) {
            using enum advent::adjacent_neighbor;

            case above: return below;
            case left:  return right;
            case right: return left;
            case below: return above;

            default: std::unreachable();
        }
    }

    export constexpr advent::diagonal_neighbor opposite_neighbor(const advent::diagonal_neighbor position) {
        switch (position) {
            using enum advent::diagonal_neighbor;

            case above_left:  return below_right;
            case above_right: return below_left;
            case below_left:  return above_right;
            case below_right: return above_left;

            default: std::unreachable();
        }
    }

    export constexpr advent::diagonal_neighbor column_opposite_neighbor(const advent::diagonal_neighbor position) {
        switch (position) {
            using enum advent::diagonal_neighbor;

            case above_left:  return below_left;
            case above_right: return below_right;
            case below_left:  return above_left;
            case below_right: return above_right;

            default: std::unreachable();
        }
    }

    export constexpr advent::diagonal_neighbor row_opposite_neighbor(const advent::diagonal_neighbor position) {
        switch (position) {
            using enum advent::diagonal_neighbor;

            case above_left:  return above_right;
            case above_right: return above_left;
            case below_left:  return below_right;
            case below_right: return below_left;

            default: std::unreachable();
        }
    }

    namespace impl {
        template<advent::neighbor_enum Position>
        constexpr bool has_all_neighbors(const auto &grid, const auto *elem) {
            for (const auto position : advent::neighbor_positions<Position>()) {
                if (!grid.has_neighbor(position, elem)) {
                    return false;
                }
            }

            return true;
        }

        template<typename T>
        struct grid_row_view {
            std::span<T> _elems;

            constexpr auto data(this const grid_row_view &self) {
                return self._elems.data();
            }

            constexpr auto begin(this const grid_row_view &self) {
                return self._elems.begin();
            }

            constexpr auto end(this const grid_row_view &self) {
                return self._elems.end();
            }

            constexpr auto size(this const grid_row_view &self) {
                return self._elems.size();
            }

            constexpr auto &front(this const grid_row_view &self) {
                return self._elems.front();
            }

            constexpr auto &back(this const grid_row_view &self) {
                return self._elems.back();
            }

            constexpr decltype(auto) operator [](this const grid_row_view &self, const std::size_t index) {
                [[assume(index < self.size())]];

                return self._elems[index];
            }
        };

        template<typename T>
        struct grid_column_view {
            /* NOTE: This is essentially a 'std::ranges::stride_view'. */

            using element_type = T;
            using pointer      = element_type *;
            using reference    = element_type &;

            struct iterator {
                using iterator_concept = std::random_access_iterator_tag;
                using value_type       = T;
                using difference_type  = std::ptrdiff_t;

                pointer _current = nullptr;
                pointer _last    = nullptr;

                std::size_t _vertical_step = 0;

                constexpr bool _is_one_past_end(this const iterator &self) {
                    return self._current > self._last;
                }

                constexpr iterator &operator ++(this iterator &self) {
                    if (self._current >= self._last) {
                        /*
                            NOTE: This check is done so that we do not invoke UB by
                            going beyond the allowed pointer range of the storage.
                        */

                        ++self._current;
                    } else {
                        self._current += self._vertical_step;
                    }

                    return self;
                }

                constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

                constexpr iterator &operator --(this iterator &self) {
                    if (self._is_one_past_end()) {
                        --self._current;
                    } else {
                        self._current -= self._vertical_step;
                    }

                    return self;
                }

                constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, --)

                friend constexpr bool operator ==(const iterator &lhs, std::default_sentinel_t) {
                    return lhs._is_one_past_end();
                }

                constexpr reference operator *(this const iterator &self) {
                    [[assume(!self._is_one_past_end())]];

                    return *self._current;
                }

                constexpr pointer operator ->(this const iterator &self) {
                    [[assume(!self._is_one_past_end())]];

                    return self._current;
                }

                constexpr reference operator [](this const iterator &self, const difference_type index) {
                    [[assume(!self._is_one_past_end())]];
                    [[assume(self._current + (index * self._vertical_step) <= self._last)]];

                    return self._current[index * self._vertical_step];
                }

                friend constexpr auto operator <=>(const iterator &lhs, const iterator &rhs) {
                    [[assume(lhs._last       == rhs._last)]];
                    [[assume(lhs._vertical_step == rhs._vertical_step)]];

                    return lhs._current <=> rhs._current;
                }

                friend constexpr bool operator ==(const iterator &lhs, const iterator &rhs) {
                    return (lhs <=> rhs) == 0;
                }

                friend constexpr difference_type operator -(const iterator &lhs, const iterator &rhs) {
                    [[assume(lhs._last       == rhs._last)]];
                    [[assume(lhs._vertical_step == rhs._vertical_step)]];

                    /* NOTE: The rounding here is helpful to our edge cases. */
                    const auto raw_steps_between = (lhs._current - rhs._current) / lhs._vertical_step;

                    if (lhs._is_one_past_end() && !rhs._is_one_past_end()) {
                        return 1 + raw_steps_between;
                    }

                    if (!lhs._is_one_past_end() && rhs._is_one_past_end()) {
                        return -1 + raw_steps_between;
                    }

                    return raw_steps_between;
                }

                constexpr iterator &operator +=(this iterator &self, difference_type difference) {
                    if (difference < 0) {
                        if (self._is_one_past_end()) {
                            --self._current;
                            ++difference;
                        }

                        self._current += difference * self._vertical_step;
                    } else {
                        if (std::cmp_less((self._last - self._current) / self._vertical_step, difference)) {
                            /* If the addition would go past the pointer bounds. */

                            self._current = self._last + 1;
                        } else {
                            self._current += difference * self._vertical_step;
                        }
                    }

                    return self;
                }

                constexpr iterator &operator -=(this iterator &self, difference_type difference) {
                    self += -difference;

                    return self;
                }

                constexpr iterator operator +(this iterator self, difference_type difference) {
                    /* NOTE: We took 'self' by value. */

                    self += difference;

                    return self;
                }

                friend constexpr iterator operator +(difference_type difference, iterator self) {
                    /* NOTE: We took 'self' by value. */

                    self += difference;

                    return self;
                }

                constexpr iterator operator -(this iterator self, difference_type difference) {
                    /* NOTE: We took 'self' by value. */

                    self -= difference;

                    return self;
                }
            };

            pointer _first;
            pointer _last;
            std::size_t _vertical_step;

            constexpr reference operator [](this const grid_column_view &self, const std::size_t index) {
                [[assume(self._first + (index * self._vertical_step) <= self._last)]];

                return self._first[index * self._vertical_step];
            }

            constexpr iterator begin(this const grid_column_view &self) {
                return iterator{self._first, self._last, self._vertical_step};
            }

            constexpr std::default_sentinel_t end(this const grid_column_view &) {
                return std::default_sentinel;
            }

            constexpr std::size_t size(this const grid_column_view &self) {
                return (self._last - self._first) / self._vertical_step + 1;
            }

            constexpr reference front(this const grid_column_view &self) {
                return *self._first;
            }

            constexpr reference back(this const grid_column_view &self) {
                return *self._last;
            }
        };

        template<typename Grid, advent::neighbor_enum Position>
        struct grid_neighbors_view {
            static constexpr auto _neighbor_positions = advent::neighbor_positions<Position>();

            using _grid_element = std::remove_reference_t<decltype(std::declval<Grid &>()._storage[0])>;

            struct iterator {
                /*
                    NOTE: This will skip over inaccessible neighbors.
                    If we wanted to more directly convey that we skipped
                    a possible neighbor, we could instead yield optionals
                    or similar, but at that point I feel we could just
                    pre-check everything and return an array-ish thing.
                */

                using iterator_concept = std::bidirectional_iterator_tag;
                using value_type       = std::pair<Position, _grid_element *>;
                using reference        = value_type;
                using difference_type  = std::ptrdiff_t;

                Grid *_grid;
                const _grid_element *_center;

                std::size_t _neighbor_index;

                constexpr reference operator *(this const iterator &self) {
                    const auto position = _neighbor_positions[self._neighbor_index];

                    return {position, self._grid->neighbor(position, self._center)};
                }

                constexpr bool _is_one_past_end(this const iterator &self) {
                    return self._neighbor_index >= _neighbor_positions.size();
                }

                constexpr iterator &operator ++(this iterator &self) {
                    [[assume(!self._is_one_past_end())]];

                    for (const auto i : std::views::iota(self._neighbor_index + 1, _neighbor_positions.size())) {
                        const auto position = _neighbor_positions[i];

                        if (self._grid->has_neighbor(position, self._center)) {
                            self._neighbor_index = i;

                            return self;
                        }
                    }

                    /* No further neighbors. */

                    self._neighbor_index = _neighbor_positions.size();

                    return self;
                }

                constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

                constexpr iterator &operator --(this iterator &self) {
                    if (self._is_one_past_end()) {
                        --self._neighbor_index;

                        return self;
                    }

                    for (const auto i : std::views::iota(0uz, self._neighbor_index) | std::views::reverse) {
                        const auto position = _neighbor_positions[i];

                        if (self._grid->has_neighbor(position, self._center)) {
                            self._neighbor_index = i;

                            return self;
                        }
                    }

                    /* No previous neighbors. */

                    return self;
                }

                constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, --)

                friend constexpr bool operator ==(const iterator &lhs, std::default_sentinel_t) {
                    return lhs._is_one_past_end();
                }

                friend constexpr bool operator ==(const iterator &lhs, const iterator &rhs) {
                    [[assume(lhs._grid   == rhs._grid)]];
                    [[assume(lhs._center == rhs._center)]];

                    return lhs._neighbor_index == rhs._neighbor_index;
                }
            };

            Grid *_grid;
            const _grid_element *_center;

            std::size_t _begin_neighbor_index;

            constexpr grid_neighbors_view(Grid *grid, const _grid_element *center) : _grid(grid), _center(center) {
                for (const auto i : std::views::iota(0uz, _neighbor_positions.size())) {
                    const auto position = _neighbor_positions[i];

                    if (grid->has_neighbor(position, center)) {
                        this->_begin_neighbor_index = i;

                        return;
                    }
                }

                /* No neighbors. */

                this->_begin_neighbor_index = _neighbor_positions.size();
            }

            constexpr iterator begin(this const grid_neighbors_view &self) {
                return iterator{self._grid, self._center, self._begin_neighbor_index};
            }

            constexpr std::default_sentinel_t end(this const grid_neighbors_view &) {
                return std::default_sentinel;
            }
        };

        template<typename T>
        struct grid {
            /* CRTP base for our grid implementations. */

            using coords_t = advent::vector_2d<std::size_t>;

            static constexpr coords_t neighbor_of_coords(const advent::neighbor position, const coords_t coords) {
                switch (position) {
                    using enum advent::neighbor;

                    case above_left:  return {coords.x() - 1, coords.y() - 1};
                    case above:       return {coords.x(),     coords.y() - 1};
                    case above_right: return {coords.x() + 1, coords.y() - 1};
                    case left:        return {coords.x() - 1, coords.y()};
                    case right:       return {coords.x() + 1, coords.y()};
                    case below_left:  return {coords.x() - 1, coords.y() + 1};
                    case below:       return {coords.x(),     coords.y() + 1};
                    case below_right: return {coords.x() + 1, coords.y() + 1};

                    default: std::unreachable();
                }
            }

            static constexpr coords_t neighbor_of_coords(const advent::adjacent_neighbor position, const coords_t coords) {
                switch (position) {
                    using enum advent::adjacent_neighbor;

                    case above: return {coords.x(),     coords.y() - 1};
                    case left:  return {coords.x() - 1, coords.y()};
                    case right: return {coords.x() + 1, coords.y()};
                    case below: return {coords.x(),     coords.y() + 1};

                    default: std::unreachable();
                }
            }

            static constexpr coords_t neighbor_of_coords(const advent::diagonal_neighbor position, const coords_t coords) {
                switch (position) {
                    using enum advent::diagonal_neighbor;

                    case above_left:  return {coords.x() - 1, coords.y() - 1};
                    case above_right: return {coords.x() + 1, coords.y() - 1};
                    case below_left:  return {coords.x() - 1, coords.y() + 1};
                    case below_right: return {coords.x() + 1, coords.y() + 1};

                    default: std::unreachable();
                }
            }

            constexpr std::size_t height(this const auto &self) {
                [[assume(self._storage.size() % self.vertical_step() == 0)]];

                return self._storage.size() / self.vertical_step();
            }

            constexpr std::size_t _to_raw_index(this const auto &self, const std::size_t column_index, const std::size_t row_index) {
                [[assume(column_index < self.width())]];
                [[assume(row_index    < self.height())]];

                return row_index * self.vertical_step() + column_index;
            }

            constexpr coords_t _from_raw_index(this const auto &self, const std::size_t raw_index) {
                [[assume(raw_index < self._storage.size())]];

                return {raw_index % self.vertical_step(), raw_index / self.vertical_step()};
            }

            constexpr std::size_t _raw_index_of(this const auto &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return static_cast<std::size_t>(elem - self._storage.data());
            }

            constexpr bool _contains(this const auto &self, const T *elem) {
                return elem >= self._storage.data() && elem < self._storage.data() + self._storage.size();
            }

            constexpr coords_t coords_of(this const auto &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return self._from_raw_index(self._raw_index_of(elem));
            }

            constexpr auto _forward_const(this auto &self, const T *elem) {
                [[assume(self._contains(elem))]];

                using element_type = std::remove_reference_t<decltype(self._storage[0])>;

                /* NOTE: This only casts to whatever the real const-ness is. */
                return const_cast<element_type *>(elem);
            }

            constexpr bool has_above_neighbor(this const auto &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return self._raw_index_of(elem) >= self.vertical_step();
            }

            constexpr auto above_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_above_neighbor(elem))]];

                return self._forward_const(elem - self.vertical_step());
            }

            constexpr bool has_below_neighbor(this const auto &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return self._raw_index_of(elem) < self._storage.size() - self.vertical_step();
            }

            constexpr auto below_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_below_neighbor(elem))]];

                return self._forward_const(elem + self.vertical_step());
            }

            constexpr bool has_left_neighbor(this const auto &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return self._raw_index_of(elem) % self.vertical_step() > 0;
            }

            constexpr auto left_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_left_neighbor(elem))]];

                return self._forward_const(elem - 1);
            }

            constexpr bool has_right_neighbor(this const auto &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return self._raw_index_of(elem) % self.vertical_step() != (self.width() - 1);
            }

            constexpr auto right_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_right_neighbor(elem))]];

                return self._forward_const(elem + 1);
            }

            constexpr bool has_above_left_neighbor(this const auto &self, const T *elem) {
                return self.has_above_neighbor(elem) && self.has_left_neighbor(elem);
            }

            constexpr auto above_left_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_above_left_neighbor(elem))]];

                return self._forward_const(elem - self.vertical_step() - 1);
            }

            constexpr bool has_above_right_neighbor(this const auto &self, const T *elem) {
                return self.has_above_neighbor(elem) && self.has_right_neighbor(elem);
            }

            constexpr auto above_right_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_above_right_neighbor(elem))]];

                return self._forward_const(elem - self.vertical_step() + 1);
            }

            constexpr bool has_below_left_neighbor(this const auto &self, const T *elem) {
                return self.has_below_neighbor(elem) && self.has_left_neighbor(elem);
            }

            constexpr auto below_left_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_below_left_neighbor(elem))]];

                return self._forward_const(elem + self.vertical_step() - 1);
            }

            constexpr bool has_below_right_neighbor(this const auto &self, const T *elem) {
                return self.has_below_neighbor(elem) && self.has_right_neighbor(elem);
            }

            constexpr auto below_right_neighbor(this auto &self, const T *elem) {
                [[assume(self.has_below_right_neighbor(elem))]];

                return self._forward_const(elem + self.vertical_step() + 1);
            }

            constexpr bool has_neighbor(this const auto &self, advent::neighbor position, const auto *elem) {
                switch (position) {
                    using enum advent::neighbor;

                    case above_left:  return self.has_above_left_neighbor(elem);
                    case above:       return self.has_above_neighbor(elem);
                    case above_right: return self.has_above_right_neighbor(elem);
                    case left:        return self.has_left_neighbor(elem);
                    case right:       return self.has_right_neighbor(elem);
                    case below_left:  return self.has_below_left_neighbor(elem);
                    case below:       return self.has_below_neighbor(elem);
                    case below_right: return self.has_below_right_neighbor(elem);

                    default: std::unreachable();
                }
            }

            constexpr bool has_neighbor(this const auto &self, advent::adjacent_neighbor position, const auto *elem) {
                switch (position) {
                    using enum advent::adjacent_neighbor;

                    case above: return self.has_above_neighbor(elem);
                    case left:  return self.has_left_neighbor(elem);
                    case right: return self.has_right_neighbor(elem);
                    case below: return self.has_below_neighbor(elem);

                    default: std::unreachable();
                }
            }

            constexpr bool has_neighbor(this const auto &self, advent::diagonal_neighbor position, const auto *elem) {
                switch (position) {
                    using enum advent::diagonal_neighbor;

                    case above_left:  return self.has_above_left_neighbor(elem);
                    case above_right: return self.has_above_right_neighbor(elem);
                    case below_left:  return self.has_below_left_neighbor(elem);
                    case below_right: return self.has_below_right_neighbor(elem);

                    default: std::unreachable();
                }
            }

            constexpr auto neighbor(this auto &self, advent::neighbor position, const auto *elem) {
                switch (position) {
                    using enum advent::neighbor;

                    case above_left:  return self.above_left_neighbor(elem);
                    case above:       return self.above_neighbor(elem);
                    case above_right: return self.above_right_neighbor(elem);
                    case left:        return self.left_neighbor(elem);
                    case right:       return self.right_neighbor(elem);
                    case below_left:  return self.below_left_neighbor(elem);
                    case below:       return self.below_neighbor(elem);
                    case below_right: return self.below_right_neighbor(elem);

                    default: std::unreachable();
                }
            }

            constexpr auto neighbor(this auto &self, advent::adjacent_neighbor position, const auto *elem) {
                switch (position) {
                    using enum advent::adjacent_neighbor;

                    case above: return self.above_neighbor(elem);
                    case left:  return self.left_neighbor(elem);
                    case right: return self.right_neighbor(elem);
                    case below: return self.below_neighbor(elem);

                    default: std::unreachable();
                }
            }

            constexpr auto neighbor(this auto &self, advent::diagonal_neighbor position, const auto *elem) {
                switch (position) {
                    using enum advent::diagonal_neighbor;

                    case above_left:  return self.above_left_neighbor(elem);
                    case above_right: return self.above_right_neighbor(elem);
                    case below_left:  return self.below_left_neighbor(elem);
                    case below_right: return self.below_right_neighbor(elem);

                    default: std::unreachable();
                }
            }

            constexpr bool has_all_neighbors(this const auto &self, const T *elem) {
                return impl::has_all_neighbors<advent::neighbor>(self, elem);
            }

            constexpr bool has_all_adjacent_neighbors(this const auto &self, const T *elem) {
                return impl::has_all_neighbors<advent::adjacent_neighbor>(self, elem);
            }

            constexpr bool has_all_diagonal_neighbors(this const auto &self, const T *elem) {
                return impl::has_all_neighbors<advent::diagonal_neighbor>(self, elem);
            }

            constexpr bool has_neighbors(this const auto &self, const T *elem, const advent::neighbor_enum auto... positions) {
                return (self.has_neighbor(positions, elem) && ...);
            }

            constexpr bool contains_coords(this const auto &self, const coords_t coords) {
                return coords.x() < self.width() && coords.y() < self.height();
            }

            template<typename Self>
            constexpr auto neighbors_of(this Self &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return impl::grid_neighbors_view<Self, advent::neighbor>(
                    &self, elem
                );
            }

            template<typename Self>
            constexpr auto adjacent_neighbors_of(this Self &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return impl::grid_neighbors_view<Self, advent::adjacent_neighbor>(
                    &self, elem
                );
            }

            template<typename Self>
            constexpr auto diagonal_neighbors_of(this Self &self, const T *elem) {
                [[assume(self._contains(elem))]];

                return impl::grid_neighbors_view<Self, advent::diagonal_neighbor>(
                    &self, elem
                );
            }

            constexpr auto row(this auto &self, const std::size_t row_index) {
                [[assume(row_index < self.height())]];

                return impl::grid_row_view{
                    std::span(self._storage.data() + (self.vertical_step() * row_index), self.width())
                };
            }

            constexpr auto last_row(this auto &self) {
                return impl::grid_row_view{
                    std::span(self._storage.data() + self._storage.size() - self.vertical_step(), self.width())
                };
            }

            constexpr auto column(this auto &self, const std::size_t column_index) {
                [[assume(column_index < self.width())]];

                const auto first_ptr = self._storage.data() + column_index;

                const auto last_index = self._storage.size() - self.vertical_step() + column_index;
                const auto last_ptr   = self._storage.data() + last_index;

                return impl::grid_column_view{
                    first_ptr, last_ptr, self.vertical_step()
                };
            }

            constexpr auto rows(this auto &self) {
                return std::views::iota(0uz, self.height()) | std::views::transform([&](const auto row_index) {
                    return self.row(row_index);
                });
            }

            constexpr auto columns(this auto &self) {
                return std::views::iota(0uz, self.width()) | std::views::transform([&](const auto column_index) {
                    return self.column(column_index);
                });
            }

            constexpr auto coords(this const auto &self) {
                return (
                    std::views::cartesian_product(
                        std::views::iota(0uz, self.height()),
                        std::views::iota(0uz, self.width())
                    ) |

                    std::views::transform([](const auto pair) {
                        const auto [y, x] = pair;

                        return coords_t{x, y};
                    })
                );
            }

            constexpr auto enumerate(this auto &self) {
                return self.coords() | std::views::transform([&](const auto coords) {
                    using coords_and_elem = std::pair<coords_t, decltype(self[coords])>;

                    return coords_and_elem{coords, self[coords]};
                });
            }

            constexpr auto &operator [](this auto &self, const std::size_t column_index, const std::size_t row_index) {
                return self._storage[self._to_raw_index(column_index, row_index)];
            }

            constexpr auto &operator [](this auto &self, const coords_t coords) {
                return self[coords.x(), coords.y()];
            }
        };

    }

    export template<typename T>
    struct grid : impl::grid<T> {
        using storage_type = impl::regular_vector<T>;

        struct builder {
            grid &_grid;

            constexpr impl::grid<T>::coords_t coords_of(this const builder &self, const T *elem) {
                return self._grid.coords_of(elem);
            }

            template<std::invocable<impl::grid_row_view<T>> RowFiller>
            constexpr void push_row(this const builder &self, const std::size_t width, RowFiller &&row_filler) {
                /* TODO: If vector ever gets a 'resize_and_overwrite' method, use that. */

                [[assume(width > 0)]];

                if (self._grid._width <= 0) {
                    self._grid._width = width;
                }

                [[assume(self._grid._width == width)]];

                self._grid._storage.resize(self._grid._storage.size() + width);

                std::invoke(std::forward<RowFiller>(row_filler), self._grid.last_row());
            }

            template<std::ranges::input_range Rng>
            requires (std::ranges::sized_range<Rng>)
            constexpr void push_row(this const builder &self, Rng &&rng) {
                self.push_row(std::ranges::size(rng), [&](const auto row) {
                    std::ranges::copy(rng, row.begin());
                });
            }
        };

        std::size_t  _width;
        storage_type _storage;

        /* NOTE: The default constructor leaves the grid in a degenerate state. */
        constexpr grid() = default;

        template<std::invocable<grid::builder &> BuilderFunction>
        constexpr explicit grid(BuilderFunction &&builder_function) : _width(0), _storage() {
            auto builder = grid::builder{*this};

            std::invoke(std::forward<BuilderFunction>(builder_function), builder);

            [[assume(this->width()  > 0)]];
            [[assume(this->height() > 0)]];
        }

        constexpr grid(std::size_t width, std::size_t height)
        :
            _width(width),
            _storage(width * height)
        {
            [[assume(width  > 0)]];
            [[assume(height > 0)]];
        }

        constexpr grid(std::size_t width, std::size_t height, const T &fill_value)
        :
            _width(width),
            _storage(width * height, fill_value)
        {
            [[assume(width  > 0)]];
            [[assume(height > 0)]];
        }

        constexpr std::size_t width(this const grid &self) {
            [[assume(self._width > 0)]];

            return self._width;
        }

        constexpr std::size_t vertical_step(this const grid &self) {
            return self.width();
        }

        constexpr auto elements(this auto &self) {
            return std::span(self._storage);
        }
    };

    export struct string_view_grid : impl::grid<char> {
        static constexpr char row_separator = '\n';

        std::size_t      _width;
        std::string_view _storage;

        constexpr std::size_t width(this const string_view_grid &self) {
            [[assume(self._width > 0)]];

            return self._width;
        }

        constexpr std::size_t vertical_step(this const string_view_grid &self) {
            return self._width + 1;
        }

        constexpr explicit string_view_grid(const std::string_view storage) : _storage(storage) {
            this->_width = storage.find_first_of(row_separator);

            [[assume(this->_width != std::string_view::npos)]];
            [[assume(this->_width > 0)]];

            [[assume(this->_storage.size() % this->vertical_step() == 0)]];

            /* TODO: It would be nice to add assumes for consistent row widths. */
        }

        constexpr auto elements(this auto &self) {
            return self.coords() | std::views::transform([&](const auto coords) -> decltype(auto) {
                return self[coords];
            });
        }
    };

    export template<typename T>
    struct grid_view : impl::grid<char> {
        /* Must begin at the start of the grid, and end at the last vertical step. */
        std::span<T> _storage;

        std::size_t  _width;
        std::size_t  _vertical_step;

        constexpr std::size_t width(this const grid_view &self) {
            return self._width;
        }

        constexpr std::size_t vertical_step(this const grid_view &self) {
            return self._vertical_step;
        }

        constexpr grid_view(std::span<T> storage, std::size_t width, std::size_t vertical_step)
        :
            _storage(storage),
            _width(width),
            _vertical_step(vertical_step)
        {
            [[assume(width > 0)]];
            [[assume(vertical_step >= width)]];
        }

        constexpr auto elements(this auto &self) {
            return self.coords() | std::views::transform([&](const auto coords) -> decltype(auto) {
                return self[coords];
            });
        }
    };
}

namespace std::ranges {

    /* Enable view. */
    template<typename T>
    constexpr inline bool enable_view<advent::impl::grid_row_view<T>> = true;

    template<typename T>
    constexpr inline bool enable_view<advent::impl::grid_column_view<T>> = true;

    template<typename Grid, typename Position>
    constexpr inline bool enable_view<advent::impl::grid_neighbors_view<Grid, Position>> = true;

    /* Enable borrowed range. */
    template<typename T>
    constexpr inline bool enable_borrowed_range<advent::impl::grid_row_view<T>> = true;

    template<typename T>
    constexpr inline bool enable_borrowed_range<advent::impl::grid_column_view<T>> = true;

    template<typename Grid, typename Position>
    constexpr inline bool enable_borrowed_range<advent::impl::grid_neighbors_view<Grid, Position>> = true;

}
