export module advent:type_traits;

import std;

import :concepts;

namespace advent {

    export template<typename T> requires advent::addable_with<T, T>
    using addition_result = decltype(std::declval<T>() + std::declval<T>());

    namespace impl {
        template <typename T>
        consteval auto type_name() {
            std::string_view name, prefix, suffix;

            /* TODO: Support more compilers. */
            #if defined(__GNUC__) && !defined(__clang__)

            name = __PRETTY_FUNCTION__;
            prefix = "consteval auto advent::impl::type_name@advent() [with T = ";
            suffix = "]";

            #else

            static_assert(false, "Unsupported compiler for retrieving type names");

            #endif

            name.remove_prefix(prefix.size());
            name.remove_suffix(suffix.size());

            return name;
        }

        struct type_name_func_tag {
            type_name_func_tag() = delete;
        };
    }

    export template<typename T = impl::type_name_func_tag>
    constexpr inline auto type_name = impl::type_name<T>();

    template<>
    constexpr inline auto type_name<impl::type_name_func_tag> = []<typename T>(T &&) {
        return impl::type_name<T &&>();
    };

}
