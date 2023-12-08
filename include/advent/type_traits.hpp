#pragma once

#include <advent/concepts.hpp>

namespace advent {

    template<typename T> requires advent::addable_with<T, T>
    using addition_result = decltype(std::declval<T>() + std::declval<T>());

    namespace impl {
        template <typename T>
        consteval auto type_name() {
            std::string_view name, prefix, suffix;

            #if defined(__clang__)

            name = __PRETTY_FUNCTION__;
            prefix = "auto advent::impl::type_name() [T = ";
            suffix = "]";

            #elif defined(__GNUC__)

            name = __PRETTY_FUNCTION__;
            prefix = "consteval auto advent::impl::type_name() [with T = ";
            suffix = "]";

            #elif defined(_MSC_VER)

            name = __FUNCSIG__;
            prefix = "auto __cdecl advent::impl::type_name<";
            suffix = ">(void)";

            #else

            #error "Unsupported compiler"

            #endif

            name.remove_prefix(prefix.size());
            name.remove_suffix(suffix.size());

            return name;
        }

        struct type_name_func_tag {
            type_name_func_tag() = delete;
        };
    }

    template<typename T = impl::type_name_func_tag>
    constexpr inline auto type_name = impl::type_name<T>();

    template<>
    constexpr inline auto type_name<impl::type_name_func_tag> = []<typename T>(T &&) {
        return impl::type_name<T &&>();
    };

}
