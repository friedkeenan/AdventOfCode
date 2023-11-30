#pragma once

#include <advent/common.hpp>
#include <advent/concepts.hpp>

namespace advent {

    template<std::invocable Invocable>
    struct scope_guard {
        ADVENT_NON_COPYABLE(scope_guard);

        [[no_unique_address]] Invocable _invocable;
        bool _active = true;

        template<std::convertible_to<Invocable> OtherInvocable>
        constexpr scope_guard(OtherInvocable &&invocable)
        noexcept(
            advent::nothrow_convertible_to<OtherInvocable, Invocable>
        ) : _invocable(std::forward<OtherInvocable>(invocable)) { }

        constexpr scope_guard(scope_guard &&other)
            : _invocable(std::move(other._invocable)), _active(std::exchange(other._active, false)) { }

        constexpr scope_guard &operator =(scope_guard &&other) {
            this->_invocable = std::move(other._invocable);
            this->_active    = std::exchange(other._active, false);

            return *this;
        }

        constexpr ~scope_guard() {
            /* If the invocable throws, 'std::terminate' will be called. */
            if (this->_active) [[likely]] {
                std::invoke(std::move(this->_invocable));
            }
        }

        constexpr void cancel() {
            this->_active = false;
        }
    };

    template<std::invocable Invocable>
    scope_guard(Invocable &&) -> scope_guard<std::decay_t<Invocable>>;

    static_assert(std::movable<scope_guard<decltype([]() { })>>);

}
