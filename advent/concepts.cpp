export module advent:concepts;

import std;

namespace advent {

    export template<typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;

    export template<typename T>
    concept signed_type = arithmetic<T> && (T{-1} < T{0});

    export template<typename T>
    concept unsigned_type = arithmetic<T> && !signed_type<T>;

    export template<typename T>
    concept array_type = std::is_array_v<std::remove_reference_t<T>>;

    template<typename T, typename Element>
    concept _array_of_impl = advent::array_type<T> && std::same_as<std::remove_cvref_t<std::remove_extent_t<T>>, Element>;

    export template<typename T, typename Element>
    concept array_of = advent::_array_of_impl<std::remove_reference_t<T>, Element>;

    export template<typename T, typename ToForward>
    concept forwarder_for = std::same_as<std::remove_cvref_t<T>, ToForward>;

    export template<typename From, typename To>
    concept nothrow_convertible_to = (
        std::convertible_to<From, To> &&

        noexcept(
            static_cast<To>(std::declval<From>())
        )
    );

    export template<typename Invocable, typename... Args>
    concept nothrow_invocable = (
        std::invocable<Invocable, Args...> &&

        noexcept(
            std::invoke(std::declval<Invocable>(), std::declval<Args>()...)
        )
    );

    export template<typename Invocable, typename Return, typename... Args>
    concept invocable_r = std::invocable<Invocable, Args...> && requires {
        { std::invoke(std::declval<Invocable>(), std::declval<Args>()...) } -> std::same_as<Return>;
    };

    export template<typename T>
    concept class_type = requires {
        typename std::type_identity<int T::*>;
    };

    export template<typename T>
    concept inheritable = class_type<T> && !std::is_union_v<T>;

    export template<typename T, typename U>
    concept addable_with = requires(T &&t, U &&u) {
        std::forward<T>(t) + std::forward<U>(u);
    };

    export template<typename Rng>
    concept string_viewable_range = (
        std::ranges::input_range<Rng> &&

        std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>
    );

    export template<typename Iter>
    concept string_viewable_iterator = (
        std::input_iterator<Iter> &&

        std::convertible_to<std::iter_reference_t<Iter>, std::string_view>
    );

}
