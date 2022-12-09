#pragma once

#include <advent/common.hpp>

namespace advent {

    template<typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;

    template<typename T>
    concept singed_type = arithmetic<T> && (T{-1} < T{0});

    template<typename T>
    concept unsigned_type = arithmetic<T> && !singed_type<T>;

    template<typename T>
    concept array_type = std::is_array_v<std::remove_reference_t<T>>;

    template<typename T, typename Element>
    concept _array_of_impl = advent::array_type<T> && std::same_as<std::remove_cvref_t<std::remove_extent_t<T>>, Element>;

    template<typename T, typename Element>
    concept array_of = advent::_array_of_impl<std::remove_reference_t<T>, Element>;

    template<typename T, typename ToForward>
    concept forwarder_for = std::same_as<std::remove_cvref_t<T>, ToForward>;

    template<typename From, typename To>
    concept nothrow_convertible_to = (
        std::convertible_to<From, To> &&

        noexcept(
            static_cast<To>(std::declval<From>())
        )
    );

    template<typename Invocable, typename... Args>
    concept nothrow_invocable = (
        std::invocable<Invocable, Args...> &&

        noexcept(
            std::invoke(std::declval<Invocable>(), std::declval<Args>()...)
        )
    );

}
