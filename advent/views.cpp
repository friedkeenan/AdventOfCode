export module advent:views;

import std;

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

}
