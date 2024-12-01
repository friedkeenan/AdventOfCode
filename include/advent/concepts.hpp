#pragma once

#include <advent/common.hpp>

namespace advent {

    template<typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;

    template<typename T>
    concept signed_type = arithmetic<T> && (T{-1} < T{0});

    template<typename T>
    concept unsigned_type = arithmetic<T> && !signed_type<T>;

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

    template<typename Invocable, typename Return, typename... Args>
    concept invocable_r = std::invocable<Invocable, Args...> && requires {
        { std::invoke(std::declval<Invocable>(), std::declval<Args>()...) } -> std::same_as<Return>;
    };

    template<typename T>
    concept class_type = requires {
        typename std::type_identity<int T::*>;
    };

    template<typename T>
    concept inheritable = class_type<T> && !std::is_union_v<T>;

    template<typename T, typename U>
    concept addable_with = requires(T &&t, U &&u) {
        std::forward<T>(t) + std::forward<U>(u);
    };

    template<typename Rng>
    concept string_viewable_range = (
        std::ranges::input_range<Rng> &&

        std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>
    );

}
