#include <advent/defines.hpp>

export module advent:views;

import std;

import :type_traits;

namespace advent::views {

    namespace impl {

        struct iterators_adaptor_closure : std::ranges::range_adaptor_closure<iterators_adaptor_closure> {
            template<std::ranges::viewable_range Rng>
            requires (std::ranges::borrowed_range<std::views::all_t<Rng>>)
            static constexpr auto operator ()(Rng &&rng) {
                auto view = std::views::all(std::forward<Rng>(rng));

                return std::views::iota(std::ranges::begin(view), std::ranges::end(view));
            }
        };

    }

    export constexpr inline auto iterators = impl::iterators_adaptor_closure{};

    namespace impl {

        template<typename Rng>
        concept sized_random_access_range = (
            std::ranges::random_access_range<Rng> &&

            std::ranges::sized_range<Rng>
        );

        template<typename Rng>
        concept bidirectional_common = (
            std::ranges::bidirectional_range<Rng> &&

            std::ranges::common_range<Rng>
        );

        template<typename Rng>
        requires (std::ranges::common_range<Rng> || impl::sized_random_access_range<Rng>)
        constexpr decltype(auto) end_iterator(Rng &&rng) {
            if constexpr (std::ranges::common_range<Rng>) {
                return std::ranges::end(std::forward<Rng>(rng));
            } else {
                return std::ranges::begin(rng) + std::ranges::distance(rng);
            }
        }

        template<typename It>
        concept has_arrow = std::input_iterator<It> && (std::is_pointer_v<It> || requires(const It it) { it.operator ->(); });

        /* Modeled when the const range is equivalent to the non-const range. */
        template<typename Rng>
        concept simple_view = (
            std::ranges::view<Rng> &&

            std::ranges::range<const Rng> &&

            std::same_as<std::ranges::iterator_t<Rng>, std::ranges::iterator_t<const Rng>> &&

            std::same_as<std::ranges::sentinel_t<Rng>, std::ranges::sentinel_t<const Rng>>
        );

    }

    export template<std::ranges::view V>
    requires (std::ranges::forward_range<V>)
    struct cycle_view : std::ranges::view_interface<cycle_view<V>> {
        template<bool IsConst>
        struct iterator {
            using parent_view     = advent::maybe_const<IsConst, cycle_view>;
            using base_view       = advent::maybe_const<IsConst, V>;
            using base_iterator   = std::ranges::iterator_t<base_view>;
            using base_difference = std::ranges::range_difference_t<base_view>;

            using iterator_concept = std::conditional_t<
                impl::sized_random_access_range<base_view>,

                std::random_access_iterator_tag,

                std::conditional_t<
                    impl::bidirectional_common<base_view>,

                    std::bidirectional_iterator_tag,

                    std::forward_iterator_tag
                >
            >;

            using base_category = std::iterator_traits<base_iterator>::iterator_category;

            using iterator_category = std::conditional_t<
                (
                    std::derived_from<base_category, std::random_access_iterator_tag> &&

                    std::ranges::sized_range<base_view>
                ),

                std::random_access_iterator_tag,

                std::conditional_t<
                    (
                        std::derived_from<base_category, std::bidirectional_iterator_tag> &&

                        std::ranges::common_range<base_view>
                    ),

                    std::bidirectional_iterator_tag,

                    std::forward_iterator_tag
                >
            >;

            using value_type = std::ranges::range_value_t<base_view>;

            /* NOTE: This might not be precisely correct, but it's fine. */
            using difference_type = std::ranges::range_difference_t<
                std::ranges::iota_view<base_iterator, std::unreachable_sentinel_t>
            >;

            base_iterator _current = base_iterator();

            parent_view *_parent = nullptr;

            base_difference _n = 0;

            constexpr iterator(parent_view &parent, base_iterator current)
            :
                _current(std::move(current)),
                _parent(&parent)
            {}

            constexpr iterator() requires (std::default_initializable<base_iterator>) = default;

            constexpr iterator(iterator<!IsConst> other)
            requires (
                IsConst &&
                std::convertible_to<std::ranges::iterator_t<V>, base_iterator>
            )
            :
                _current(std::move(other)),
                _parent(other.parent),
                _n(other._n)
            {}

            constexpr base_iterator base(this const iterator &self) {
                return self._current;
            }

            constexpr decltype(auto) operator *(this const iterator &self) {
                return *self._current;
            }

            constexpr base_iterator operator ->(this const iterator &self)
            requires (impl::has_arrow<base_iterator>)
            {
                return self._current;
            }

            constexpr iterator &operator ++(this iterator &self) {
                ++self._current;

                if (self._current == std::ranges::end(self._parent->_base)) {
                    self._current = std::ranges::begin(self._parent->_base);

                    ++self._n;
                }

                return self;
            }

            constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

            constexpr iterator &operator --(this iterator &self)
            requires (impl::sized_random_access_range<base_view> || impl::bidirectional_common<base_view>)
            {
                if (self._current == std::ranges::begin(self._parent->_base)) {
                    self._current = impl::end_iterator(self._parent->_base);

                    --self._n;
                }

                --self._current;

                return self;
            }

            constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(
                iterator, --,

                requires (impl::sized_random_access_range<base_view> || impl::bidirectional_common<base_view>)
            )

            constexpr iterator &operator +=(this iterator &self, difference_type n)
            requires (impl::sized_random_access_range<base_view>)
            {
                const auto first = std::ranges::begin(self._parent->_base);
                const auto size  = std::ranges::distance(self._parent->_base);

                const auto offset = self._current - first;

                const auto new_n      = n + offset;
                const auto new_offset = new_n % size;

                self._n += new_n / size;

                self._current = first + [&]() {
                    if (new_offset >= 0) {
                        return static_cast<base_difference>(new_offset);
                    }

                    return static_cast<base_difference>(new_offset + size);
                }();

                return self;
            }

            constexpr iterator &operator -=(this iterator &self, difference_type n)
            requires (impl::sized_random_access_range<base_view>)
            {
                self += -n;

                return self;
            }

            constexpr decltype(auto) operator [](this const iterator &self, difference_type n)
            requires (impl::sized_random_access_range<base_view>)
            {
                return *(self + n);
            }

            friend constexpr bool operator ==(const iterator &lhs, const iterator &rhs) {
                return lhs._n == rhs._n && lhs._current == rhs._current;
            }

            friend constexpr bool operator ==(const iterator &lhs, std::default_sentinel_t) {
                /* If the range is empty, there's nothing to cycle. */
                return std::ranges::empty(lhs._parent->_base);
            }

            #define RANDOM_ACCESS_CMP(op)                                                   \
                friend constexpr bool operator op(const iterator &lhs, const iterator &rhs) \
                requires (std::ranges::random_access_range<base_view>)                      \
                {                                                                           \
                    if (lhs._n != rhs._n) {                                                 \
                        return lhs._n op rhs._n;                                            \
                    }                                                                       \
                    return lhs._current op rhs._current;                                    \
                }

            RANDOM_ACCESS_CMP(<)
            RANDOM_ACCESS_CMP(>)
            RANDOM_ACCESS_CMP(<=)
            RANDOM_ACCESS_CMP(>=)

            #undef RANDOM_ACCESS_CMP

            friend constexpr auto operator <=>(const iterator &lhs, const iterator &rhs)
            requires (std::three_way_comparable<base_iterator>)
            {
                using Result = std::compare_three_way_result_t<base_iterator>;

                if (lhs._n != rhs._n) {
                    return Result(lhs._n <=> rhs._n);
                }

                return Result(lhs._current <=> rhs._current);
            }

            friend constexpr iterator operator +(const iterator &lhs, difference_type rhs)
            requires (impl::sized_random_access_range<base_view>)
            {
                auto result = lhs;

                result += rhs;

                return result;
            }

            friend constexpr iterator operator +(difference_type lhs, const iterator &rhs)
            requires (impl::sized_random_access_range<base_view>)
            {
                return rhs + lhs;
            }

            friend constexpr iterator operator -(const iterator &lhs, difference_type rhs)
            requires (impl::sized_random_access_range<base_view>)
            {
                auto result = lhs;

                result -= rhs;

                return result;
            }

            friend constexpr difference_type operator -(const iterator &lhs, const iterator &rhs)
            requires (
                std::sized_sentinel_for<base_iterator, base_iterator> &&
                std::ranges::sized_range<base_view>
            ) {
                const auto size = std::ranges::distance(lhs._parent->_base);

                return (lhs._n - rhs._n) * size + (lhs._current - rhs._current);
            }

            friend constexpr std::ranges::range_rvalue_reference_t<base_view> iter_move(const iterator &it)
            noexcept(noexcept(std::ranges::iter_move(it._current)))
            {
                return std::ranges::iter_move(it._current);
            }
        };

        V _base = V();

        constexpr cycle_view() requires (std::default_initializable<V>) = default;

        constexpr explicit cycle_view(V base) : _base(std::move(base)) {}

        constexpr V base(this const cycle_view &self)
        requires (std::copy_constructible<V>)
        {
            return self._base;
        }

        constexpr V base(this cycle_view &&self) {
            return std::move(self._base);
        }

        constexpr auto begin(this cycle_view &self)
        requires (!impl::simple_view<V>)
        {
            return iterator<false>(self, std::ranges::begin(self._base));
        }

        constexpr auto begin(this const cycle_view &self)
        requires (std::ranges::forward_range<const V>)
        {
            return iterator<true>(self, std::ranges::begin(self._base));
        }

        constexpr std::default_sentinel_t end(this const cycle_view &) noexcept {
            return std::default_sentinel;
        }
    };

    template<typename R>
    cycle_view(R &&) -> cycle_view<std::views::all_t<R>>;

    namespace impl {

        struct cycle_adaptor_closure : std::ranges::range_adaptor_closure<cycle_adaptor_closure> {
            template<std::ranges::viewable_range Rng>
            requires (std::ranges::forward_range<std::views::all_t<Rng>>)
            static constexpr auto operator ()(Rng &&rng) {
                return views::cycle_view(std::forward<Rng>(rng));
            }
        };

    }

    export constexpr inline auto cycle = impl::cycle_adaptor_closure{};

}
