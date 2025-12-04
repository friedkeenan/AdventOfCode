export module advent:functional;

import :concepts;

namespace advent {

    export template<advent::inheritable... Callables>
    struct overloaded : Callables... {
        /* Constructor and deduction guide are implicitly defined. */

        /* TODO: Maybe find a way we can accurately reflect this in our requirements. */
        using Callables::operator ()...;
    };

    namespace impl {

        struct dereference_fn {
            template<typename T>
            requires (requires { *std::declval<T>(); })
            static constexpr decltype(auto) operator ()(T &&obj) {
                return *std::forward<T>(obj);
            }
        };

    }

    export constexpr inline auto dereference = impl::dereference_fn{};

}
