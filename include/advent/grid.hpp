#pragma once

#include <advent/common.hpp>
#include <advent/vector_nd.hpp>

namespace advent {

    template<typename T>
    struct grid {
        template<bool IsConst>
        struct _row_view {
            using element_type = std::conditional_t<IsConst, const T, T>;

            std::span<element_type> _elems;

            constexpr auto data(this const _row_view &self) {
                return self._elems.data();
            }

            constexpr auto begin(this const _row_view &self) {
                return self._elems.begin();
            }

            constexpr auto end(this const _row_view &self) {
                return self._elems.end();
            }

            constexpr auto size(this const _row_view &self) {
                return self._elems.size();
            }

            constexpr auto &front(this const _row_view &self) {
                return self._elems.front();
            }

            constexpr auto &back(this const _row_view &self) {
                return self._elems.back();
            }

            constexpr decltype(auto) operator [](this const _row_view &self, const std::size_t index) {
                [[assume(index < self.size())]];

                return self._elems[index];
            }
        };

        using mutable_row = _row_view<false>;
        using const_row   = _row_view<true>;

        static_assert(std::ranges::contiguous_range<mutable_row>);
        static_assert(std::ranges::contiguous_range<const_row>);

        static_assert(std::ranges::sized_range<mutable_row>);
        static_assert(std::ranges::sized_range<const_row>);

        template<typename Self>
        using _row_view_for_type = _row_view<std::is_const_v<std::remove_reference_t<Self>>>;

        template<bool IsConst>
        struct _column_view {
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

            constexpr reference operator [](this const _column_view &self, const std::size_t index) {
                [[assume(self._first + (index * self._grid_width) <= self._last)]];

                return self._first[index * self._grid_width];
            }

            constexpr iterator begin(this const _column_view &self) {
                return iterator{self._first, self._last, self._grid_width};
            }

            constexpr std::default_sentinel_t end(this const _column_view &) {
                return std::default_sentinel;
            }

            constexpr std::size_t size(this const _column_view &self) {
                return (self._last - self._first) / self._grid_width + 1;
            }

            constexpr reference front(this const _column_view &self) {
                return *self._first;
            }

            constexpr reference back(this const _column_view &self) {
                return *self._last;
            }
        };

        using mutable_column = _column_view<false>;
        using const_column   = _column_view<true>;

        static_assert(std::ranges::random_access_range<mutable_column>);
        static_assert(std::ranges::random_access_range<const_column>);

        static_assert(std::ranges::sized_range<mutable_column>);
        static_assert(std::ranges::sized_range<const_column>);

        template<typename Self>
        using _column_view_for_type = _column_view<std::is_const_v<std::remove_reference_t<Self>>>;

        struct builder {
            std::size_t    _width   = 0;
            std::vector<T> _storage = {};

            constexpr advent::vector_2d<std::size_t> coords_of(this const builder &self, const T *elem) {
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

        constexpr auto &operator [](this auto &self, const std::size_t column_index, const std::size_t row_index) {
            return self._storage[self._to_raw_index(column_index, row_index)];
        }
    };

}

namespace std::ranges {

    /* Enable view. */
    template<typename T> requires (std::same_as<T, typename advent::grid<T>::mutable_row>)
    constexpr inline bool enable_view<T> = true;

    template<typename T> requires (std::same_as<T, typename advent::grid<T>::const_row>)
    constexpr inline bool enable_view<T> = true;

    template<typename T> requires (std::same_as<T, typename advent::grid<T>::mutable_column>)
    constexpr inline bool enable_view<T> = true;

    template<typename T> requires (std::same_as<T, typename advent::grid<T>::const_column>)
    constexpr inline bool enable_view<T> = true;

    /* Enable borrowed range. */
    template<typename T> requires (std::same_as<T, typename advent::grid<T>::mutable_row>)
    constexpr inline bool enable_borrowed_range<T> = true;

    template<typename T> requires (std::same_as<T, typename advent::grid<T>::const_row>)
    constexpr inline bool enable_borrowed_range<T> = true;

    template<typename T> requires (std::same_as<T, typename advent::grid<T>::mutable_column>)
    constexpr inline bool enable_borrowed_range<T> = true;

    template<typename T> requires (std::same_as<T, typename advent::grid<T>::const_column>)
    constexpr inline bool enable_borrowed_range<T> = true;

}
