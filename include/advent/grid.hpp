#pragma once

#include <advent/common.hpp>
#include <advent/vector_nd.hpp>

namespace advent {

    enum class neighbor : std::uint8_t {
        above_left,
        above,
        above_right,
        left,
        right,
        below_left,
        below,
        below_right,
    };

    enum class adjacent_neighbor : std::uint8_t {
        above = std::to_underlying(advent::neighbor::above),
        left  = std::to_underlying(advent::neighbor::left),
        right = std::to_underlying(advent::neighbor::right),
        below = std::to_underlying(advent::neighbor::below),
    };

    enum class diagonal_neighbor : std::uint8_t {
        above_left  = std::to_underlying(advent::neighbor::above_left),
        above_right = std::to_underlying(advent::neighbor::above_right),
        below_left  = std::to_underlying(advent::neighbor::below_left),
        below_right = std::to_underlying(advent::neighbor::below_right),
    };

    template<typename T>
    concept neighbor_enum = (
        std::same_as<T, advent::neighbor>          ||
        std::same_as<T, advent::adjacent_neighbor> ||
        std::same_as<T, advent::diagonal_neighbor>
    );

    constexpr bool operator ==(const advent::neighbor lhs, const advent::adjacent_neighbor rhs) noexcept {
        return std::to_underlying(lhs) == std::to_underlying(rhs);
    }

    constexpr bool operator ==(const advent::neighbor lhs, const advent::diagonal_neighbor rhs) noexcept {
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

    template<advent::neighbor_enum Position>
    constexpr inline auto neighbor_positions = std::array{
        advent::neighbor::above_left,
        advent::neighbor::above,
        advent::neighbor::above_right,
        advent::neighbor::left,
        advent::neighbor::right,
        advent::neighbor::below_left,
        advent::neighbor::below,
        advent::neighbor::below_right
    };

    template<>
    constexpr inline auto neighbor_positions<advent::adjacent_neighbor> = std::array{
        advent::adjacent_neighbor::above,
        advent::adjacent_neighbor::left,
        advent::adjacent_neighbor::right,
        advent::adjacent_neighbor::below
    };

    template<>
    constexpr inline auto neighbor_positions<advent::diagonal_neighbor> = std::array{
        advent::diagonal_neighbor::above_left,
        advent::diagonal_neighbor::above_right,
        advent::diagonal_neighbor::below_left,
        advent::diagonal_neighbor::below_right
    };

    constexpr advent::diagonal_neighbor opposite_neighbor(const advent::diagonal_neighbor position) {
        switch (position) {
            using enum advent::diagonal_neighbor;

            case above_left:  return below_right;
            case above_right: return below_left;
            case below_left:  return above_right;
            case below_right: return above_left;

            default: std::unreachable();
        }
    }

    constexpr advent::diagonal_neighbor column_opposite_neighbor(const advent::diagonal_neighbor position) {
        switch (position) {
            using enum advent::diagonal_neighbor;

            case above_left:  return below_left;
            case above_right: return below_right;
            case below_left:  return above_left;
            case below_right: return above_right;

            default: std::unreachable();
        }
    }

    constexpr advent::diagonal_neighbor row_opposite_neighbor(const advent::diagonal_neighbor position) {
        switch (position) {
            using enum advent::diagonal_neighbor;

            case above_left:  return above_right;
            case above_right: return above_left;
            case below_left:  return below_right;
            case below_right: return below_left;

            default: std::unreachable();
        }
    }

    /* Forward declaration. */
    template<typename T>
    struct grid;

    namespace impl {
        template<typename T>
        constexpr bool has_neighbor(const advent::grid<T> &grid, advent::neighbor position, const T *elem) {
            switch (position) {
                using enum advent::neighbor;

                case above_left:  return grid.has_above_left_neighbor(elem);
                case above:       return grid.has_above_neighbor(elem);
                case above_right: return grid.has_above_right_neighbor(elem);
                case left:        return grid.has_left_neighbor(elem);
                case right:       return grid.has_right_neighbor(elem);
                case below_left:  return grid.has_below_left_neighbor(elem);
                case below:       return grid.has_below_neighbor(elem);
                case below_right: return grid.has_below_right_neighbor(elem);

                default: std::unreachable();
            }
        }

        template<typename T>
        constexpr bool has_neighbor(const advent::grid<T> &grid, advent::adjacent_neighbor position, const T *elem) {
            switch (position) {
                using enum advent::adjacent_neighbor;

                case above: return grid.has_above_neighbor(elem);
                case left:  return grid.has_left_neighbor(elem);
                case right: return grid.has_right_neighbor(elem);
                case below: return grid.has_below_neighbor(elem);

                default: std::unreachable();
            }
        }

        template<typename T>
        constexpr bool has_neighbor(const advent::grid<T> &grid, advent::diagonal_neighbor position, const T *elem) {
            switch (position) {
                using enum advent::diagonal_neighbor;

                case above_left:  return grid.has_above_left_neighbor(elem);
                case above_right: return grid.has_above_right_neighbor(elem);
                case below_left:  return grid.has_below_left_neighbor(elem);
                case below_right: return grid.has_below_right_neighbor(elem);

                default: std::unreachable();
            }
        }

        template<typename T>
        constexpr auto neighbor_of(auto &grid, advent::neighbor position, const T *elem) {
            switch (position) {
                using enum advent::neighbor;

                case above_left:  return grid.above_left_neighbor(elem);
                case above:       return grid.above_neighbor(elem);
                case above_right: return grid.above_right_neighbor(elem);
                case left:        return grid.left_neighbor(elem);
                case right:       return grid.right_neighbor(elem);
                case below_left:  return grid.below_left_neighbor(elem);
                case below:       return grid.below_neighbor(elem);
                case below_right: return grid.below_right_neighbor(elem);

                default: std::unreachable();
            }
        }

        template<typename T>
        constexpr auto neighbor_of(auto &grid, advent::adjacent_neighbor position, const T *elem) {
            switch (position) {
                using enum advent::adjacent_neighbor;

                case above: return grid.above_neighbor(elem);
                case left:  return grid.left_neighbor(elem);
                case right: return grid.right_neighbor(elem);
                case below: return grid.below_neighbor(elem);

                default: std::unreachable();
            }
        }

        template<typename T>
        constexpr auto neighbor_of(auto &grid, advent::diagonal_neighbor position, const T *elem) {
            switch (position) {
                using enum advent::diagonal_neighbor;

                case above_left:  return grid.above_left_neighbor(elem);
                case above_right: return grid.above_right_neighbor(elem);
                case below_left:  return grid.below_left_neighbor(elem);
                case below_right: return grid.below_right_neighbor(elem);

                default: std::unreachable();
            }
        }

        template<bool IsConst, typename T>
        struct grid_row_view {
            using element_type = std::conditional_t<IsConst, const T, T>;

            std::span<element_type> _elems;

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

        template<bool IsConst, typename T>
        struct grid_column_view {
            /* NOTE: This is essentially a 'std::ranges::stride_view'. */

            using element_type = std::conditional_t<IsConst, const T, T>;
            using pointer      = element_type *;
            using reference    = element_type &;

            struct iterator {
                using iterator_concept = std::random_access_iterator_tag;
                using value_type       = T;
                using difference_type  = std::ptrdiff_t;

                pointer _current = nullptr;
                pointer _last    = nullptr;

                std::size_t _grid_width = 0;

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
                        self._current += self._grid_width;
                    }

                    return self;
                }

                constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

                constexpr iterator &operator --(this iterator &self) {
                    if (self._is_one_past_end()) {
                        --self._current;
                    } else {
                        self._current -= self._grid_width;
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

                constexpr reference operator [](this const iterator &self, const difference_type index) {
                    [[assume(!self._is_one_past_end())]];
                    [[assume(self._current + (index * self._grid_width) <= self._last)]];

                    return self._current[index * self._grid_width];
                }

                friend constexpr auto operator <=>(const iterator &lhs, const iterator &rhs) {
                    [[assume(lhs._last       == rhs._last)]];
                    [[assume(lhs._grid_width == rhs._grid_width)]];

                    return lhs._current <=> rhs._current;
                }

                friend constexpr bool operator ==(const iterator &lhs, const iterator &rhs) {
                    return (lhs <=> rhs) == 0;
                }

                friend constexpr difference_type operator -(const iterator &lhs, const iterator &rhs) {
                    [[assume(lhs._last       == rhs._last)]];
                    [[assume(lhs._grid_width == rhs._grid_width)]];

                    /* NOTE: The rounding here is helpful to our edge cases. */
                    const auto divided_difference = (lhs._current - rhs._current) / lhs._grid_width;

                    if (lhs._is_one_past_end() && !rhs._is_one_past_end()) {
                        return 1 + divided_difference;
                    }

                    if (!lhs._is_one_past_end() && rhs._is_one_past_end()) {
                        return -1 + divided_difference;
                    }

                    return divided_difference;
                }

                constexpr iterator &operator +=(this iterator &self, difference_type difference) {
                    if (difference < 0) {
                        if (self._is_one_past_end()) {
                            --self._current;
                            ++difference;
                        }

                        self._current += difference * self._grid_width;
                    } else {
                        if (std::cmp_less((self._last - self._current) / self._grid_width, difference)) {
                            /* If the addition would go past the pointer bounds. */

                            self._current = self._last + 1;
                        } else {
                            self._current += difference * self._grid_width;
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
            std::size_t _grid_width;

            constexpr reference operator [](this const grid_column_view &self, const std::size_t index) {
                [[assume(self._first + (index * self._grid_width) <= self._last)]];

                return self._first[index * self._grid_width];
            }

            constexpr iterator begin(this const grid_column_view &self) {
                return iterator{self._first, self._last, self._grid_width};
            }

            constexpr std::default_sentinel_t end(this const grid_column_view &) {
                return std::default_sentinel;
            }

            constexpr std::size_t size(this const grid_column_view &self) {
                return (self._last - self._first) / self._grid_width + 1;
            }

            constexpr reference front(this const grid_column_view &self) {
                return *self._first;
            }

            constexpr reference back(this const grid_column_view &self) {
                return *self._last;
            }
        };

        template<bool IsConst, advent::neighbor_enum Position, typename T>
        struct grid_neighbors_view {
            using _grid_type = std::conditional_t<IsConst, const advent::grid<T>, advent::grid<T>>;

            static constexpr auto &_neighbor_positions = advent::neighbor_positions<Position>;

            struct iterator {
                /*
                    NOTE: This will skip over inaccessible neighbors.
                    If we wanted to more directly convey that we skipped
                    a possible neighbor, we could instead yield optionals
                    or similar, but at that point I feel we could just
                    pre-check everything and return an array-ish things.
                */

                using iterator_concept = std::bidirectional_iterator_tag;
                using value_type       = std::pair<Position, const T *>;
                using reference        = value_type;
                using difference_type  = std::ptrdiff_t;

                _grid_type *_grid;
                const T    *_center;

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

            _grid_type *_grid;
            const T    *_center;

            std::size_t _begin_neighbor_index;

            constexpr grid_neighbors_view(_grid_type *grid, const T *center) : _grid(grid), _center(center) {
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

    }

    template<typename T>
    struct grid {
        using mutable_row = impl::grid_row_view<false, T>;
        using const_row   = impl::grid_row_view<true, T>;

        static_assert(std::ranges::contiguous_range<mutable_row>);
        static_assert(std::ranges::contiguous_range<const_row>);

        static_assert(std::ranges::sized_range<mutable_row>);
        static_assert(std::ranges::sized_range<const_row>);

        template<typename Self>
        using _row_view_for_type = impl::grid_row_view<std::is_const_v<std::remove_reference_t<Self>>, T>;

        using mutable_column = impl::grid_column_view<false, T>;
        using const_column   = impl::grid_column_view<true, T>;

        static_assert(std::ranges::random_access_range<mutable_column>);
        static_assert(std::ranges::random_access_range<const_column>);

        static_assert(std::ranges::sized_range<mutable_column>);
        static_assert(std::ranges::sized_range<const_column>);

        template<typename Self>
        using _column_view_for_type = impl::grid_column_view<std::is_const_v<std::remove_reference_t<Self>>, T>;

        using mutable_neighbors = impl::grid_neighbors_view<false, advent::neighbor, T>;
        using const_neighbors   = impl::grid_neighbors_view<true,  advent::neighbor, T>;

        using mutable_adjacent_neighbors = impl::grid_neighbors_view<false, advent::adjacent_neighbor, T>;
        using const_adjacent_neighbors   = impl::grid_neighbors_view<true,  advent::adjacent_neighbor, T>;

        using mutable_diagonal_neighbors = impl::grid_neighbors_view<false, advent::diagonal_neighbor, T>;
        using const_diagonal_neighbors   = impl::grid_neighbors_view<true,  advent::diagonal_neighbor, T>;

        static_assert(std::ranges::bidirectional_range<mutable_neighbors>);
        static_assert(std::ranges::bidirectional_range<const_neighbors>);

        static_assert(std::ranges::bidirectional_range<mutable_adjacent_neighbors>);
        static_assert(std::ranges::bidirectional_range<const_adjacent_neighbors>);

        static_assert(std::ranges::bidirectional_range<mutable_diagonal_neighbors>);
        static_assert(std::ranges::bidirectional_range<const_diagonal_neighbors>);

        template<typename Self, advent::neighbor_enum Position>
        using _neighbors_view_for_type = impl::grid_neighbors_view<std::is_const_v<std::remove_reference_t<Self>>, Position, T>;

        struct builder {
            ADVENT_NON_COPYABLE(builder);

            std::size_t    _width   = 0;
            std::vector<T> _storage = {};

            constexpr builder() = default;

            constexpr builder(builder &&) = default;
            constexpr builder &operator =(builder &&) = default;

            constexpr advent::vector_2d<std::size_t> coords_of(this const builder &self, const T *elem) {
                [[assume(self._width > 0)]];

                [[assume(elem >= self._storage.data())]];
                [[assume(elem <  self._storage.data() + self._storage.size())]];

                const auto raw_index = static_cast<std::size_t>(elem - self._storage.data());

                return {raw_index % self._width, raw_index / self._width};
            }

            template<std::invocable<grid::mutable_row> RowFiller>
            constexpr void push_row(this builder &self, const std::size_t width, RowFiller &&row_filler) {
                /* TODO: If vector ever gets a 'resize_and_overwrite' method, use that. */

                [[assume(width > 0)]];

                if (self._width <= 0) {
                    self._width = width;
                }

                [[assume(self._width == width)]];

                self._storage.resize(self._storage.size() + width);

                auto new_row = grid::mutable_row{
                    std::span(self._storage.data() + self._storage.size() - width, width)
                };

                std::invoke(std::forward<RowFiller>(row_filler), std::move(new_row));
            }

            constexpr grid build(this builder &&self) {
                [[assume(self._width > 0)]];

                return grid{self._width, std::move(self._storage)};
            }
        };

        template<std::invocable<grid::builder &> BuilderFunction>
        static constexpr grid build(BuilderFunction &&builder_function) {
            auto builder = grid::builder{};

            std::invoke(std::forward<BuilderFunction>(builder_function), builder);

            return std::move(builder).build();
        }

        std::size_t    _width;
        std::vector<T> _storage;

        constexpr std::size_t width(this const grid &self) {
            [[assume(self._width > 0)]];

            return self._width;
        }

        constexpr std::size_t height(this const grid &self) {
            [[assume(self._storage.size() % self.width() == 0)]];

            return self._storage.size() / self.width();
        }

        constexpr std::size_t _to_raw_index(this const grid &self, const std::size_t column_index, const std::size_t row_index) {
            [[assume(column_index < self.height())]];
            [[assume(row_index    < self.width())]];

            return row_index * self.width() + column_index;
        }

        constexpr advent::vector_2d<std::size_t> _from_raw_index(this const grid &self, const std::size_t raw_index) {
            [[assume(raw_index < self._storage.size())]];

            return {raw_index % self.width(), raw_index / self.width()};
        }

        constexpr std::size_t _raw_index_of(this auto &self, const T *elem) {
            [[assume(self._contains(elem))]];

            return static_cast<std::size_t>(elem - self._storage.data());
        }

        constexpr bool _contains(this const grid &self, const T *elem) {
            return elem >= self._storage.data() && elem < self._storage.data() + self._storage.size();
        }

        constexpr advent::vector_2d<std::size_t> coords_of(this const grid &self, const T *elem) {
            [[assume(self._contains(elem))]];

            const auto raw_index = self._raw_index_of(elem);

            return {raw_index % self.width(), raw_index / self.width()};
        }

        constexpr auto _forward_const(this auto &self, const T *elem) {
            [[assume(self._contains(elem))]];

            /* NOTE: This only casts to whatever the const-ness of 'self' is. */
            return const_cast<std::conditional_t<std::is_const_v<decltype(self)>, const T *, T *>>(elem);
        }

        constexpr bool has_above_neighbor(this const grid &self, const T *elem) {
            [[assume(self._contains(elem))]];

            return self._raw_index_of(elem) >= self.width();
        }

        constexpr auto above_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_above_neighbor(elem))]];

            return self._forward_const(elem - self.width());
        }

        constexpr bool has_below_neighbor(this const grid &self, const T *elem) {
            [[assume(self._contains(elem))]];

            return self._raw_index_of(elem) < self._storage.size() - self.width();
        }

        constexpr auto below_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_below_neighbor(elem))]];

            return self._forward_const(elem + self.width());
        }

        constexpr bool has_left_neighbor(this const grid &self, const T *elem) {
            [[assume(self._contains(elem))]];

            return self._raw_index_of(elem) % self.width() > 0;
        }

        constexpr auto left_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_left_neighbor(elem))]];

            return self._forward_const(elem - 1);
        }

        constexpr bool has_right_neighbor(this const grid &self, const T *elem) {
            [[assume(self._contains(elem))]];

            return self._raw_index_of(elem) % self.width() != (self.width() - 1);
        }

        constexpr auto right_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_right_neighbor(elem))]];

            return self._forward_const(elem + 1);
        }

        constexpr bool has_above_left_neighbor(this const grid &self, const T *elem) {
            return self.has_above_neighbor(elem) && self.has_left_neighbor(elem);
        }

        constexpr auto above_left_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_above_left_neighbor(elem))]];

            return self._forward_const(elem - self.width() - 1);
        }

        constexpr bool has_above_right_neighbor(this const grid &self, const T *elem) {
            return self.has_above_neighbor(elem) && self.has_right_neighbor(elem);
        }

        constexpr auto above_right_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_above_right_neighbor(elem))]];

            return self._forward_const(elem - self.width() + 1);
        }

        constexpr bool has_below_left_neighbor(this const grid &self, const T *elem) {
            return self.has_below_neighbor(elem) && self.has_left_neighbor(elem);
        }

        constexpr auto below_left_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_below_left_neighbor(elem))]];

            return self._forward_const(elem + self.width() - 1);
        }

        constexpr bool has_below_right_neighbor(this const grid &self, const T *elem) {
            return self.has_below_neighbor(elem) && self.has_right_neighbor(elem);
        }

        constexpr auto below_right_neighbor(this auto &self, const T *elem) {
            [[assume(self.has_below_right_neighbor(elem))]];

            return self._forward_const(elem + self.width() + 1);
        }

        template<advent::neighbor_enum Position>
        constexpr bool has_neighbor(this const grid &self, const Position position, const T *elem) {
            return impl::has_neighbor(self, position, elem);
        }

        template<advent::neighbor_enum Position>
        constexpr auto neighbor(this auto &self, const Position position, const T *elem) {
            [[assume(self.has_neighbor(position, elem))]];

            return impl::neighbor_of(self, position, elem);
        }

        constexpr auto neighbors_of(this auto &self, const T *elem) {
            [[assume(self._contains(elem))]];

            return _neighbors_view_for_type<decltype(self), advent::neighbor>(
                &self, elem
            );
        }

        constexpr auto adjacent_neighbors_of(this auto &self, const T *elem) {
            [[assume(self._contains(elem))]];

            return _neighbors_view_for_type<decltype(self), advent::adjacent_neighbor>(
                &self, elem
            );
        }

        constexpr auto row(this auto &self, const std::size_t row_index){
            [[assume(row_index < self.height())]];

            return _row_view_for_type<decltype(self)>{
                std::span(self._storage.data() + (self.width() * row_index), self.width())
            };
        }

        constexpr auto column(this auto &self, const std::size_t column_index) {
            [[assume(column_index < self.width())]];

            const auto first_ptr = self._storage.data() + column_index;

            const auto last_index = self._storage.size() - self.width() + column_index;
            const auto last_ptr   = self._storage.data() + last_index;

            return _column_view_for_type<decltype(self)>{
                first_ptr, last_ptr, self.width()
            };
        }

        constexpr auto rows(this auto &self) {
            return std::views::iota(0uz, self.height()) | std::views::transform([self](const auto row_index) {
                return self.row(row_index);
            });
        }

        constexpr auto columns(this auto &self) {
            return std::views::iota(0uz, self.width()) | std::views::transform([self](const auto column_index) {
                return self.column(column_index);
            });
        }

        constexpr auto enumerate(this auto &self) {
            return std::views::iota(0uz, self._storage.size()) | std::views::transform([self](const auto raw_index) {
                return std::pair{self._from_raw_index(raw_index), self._storage[raw_index]};
            });
        }

        constexpr auto elements(this auto &self) {
            return std::span(self._storage);
        }

        constexpr auto &front(this auto &self) {
            return self._storage.front();
        }

        constexpr auto &back(this auto &self) {
            return self._storage.back();
        }

        constexpr const T *find(this const grid &self, const std::equality_comparable_with<T> auto &value) {
            const auto it = std::ranges::find(self._storage, value);

            if (it == self._storage.end()) {
                return nullptr;
            }

            return std::to_address(it);
        }

        constexpr const T *find_after(this const grid &self, const T *elem, const std::equality_comparable_with<T> auto &value) {
            [[assume(self._contains(elem))]];

            const auto ptr = std::ranges::find(elem + 1, std::to_address(self._storage.end()), value);

            if (ptr == std::to_address(self._storage.end())) {
                return nullptr;
            }

            return ptr;
        }

        constexpr auto &operator [](this auto &self, const std::size_t column_index, const std::size_t row_index) {
            return self._storage[self._to_raw_index(column_index, row_index)];
        }
    };

}

namespace std::ranges {

    /* Enable view. */
    template<bool IsConst, typename T>
    constexpr inline bool enable_view<advent::impl::grid_row_view<IsConst, T>> = true;

    template<bool IsConst, typename T>
    constexpr inline bool enable_view<advent::impl::grid_column_view<IsConst, T>> = true;

    template<bool IsConst, typename Position, typename T>
    constexpr inline bool enable_view<advent::impl::grid_neighbors_view<IsConst, Position, T>> = true;

    /* Enable borrowed range. */
    template<bool IsConst, typename T>
    constexpr inline bool enable_borrowed_range<advent::impl::grid_row_view<IsConst, T>> = true;

    template<bool IsConst, typename T>
    constexpr inline bool enable_borrowed_range<advent::impl::grid_column_view<IsConst, T>> = true;

    template<bool IsConst, typename Position, typename T>
    constexpr inline bool enable_borrowed_range<advent::impl::grid_neighbors_view<IsConst, Position, T>> = true;

}
