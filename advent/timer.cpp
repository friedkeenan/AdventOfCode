export module advent:timer;

import std;

namespace advent {

    export template<typename Clock = std::chrono::steady_clock>
    requires (std::chrono::is_clock_v<Clock> && Clock::is_steady)
    struct timer {
        Clock::duration _duration;

        constexpr Clock::duration last_measured_duration(this const timer self) {
            return self._duration;
        }

        [[nodiscard]]
        constexpr auto measure_scope(this timer &self) {
            struct scope_measurer {
                scope_measurer(const scope_measurer &) = delete;
                scope_measurer &operator =(const scope_measurer &) = delete;

                scope_measurer(scope_measurer &&) = delete;
                scope_measurer &operator =(scope_measurer &&) = delete;

                timer &_timer;

                Clock::time_point _start;

                constexpr explicit scope_measurer(timer &timer) : _timer(timer) {
                    if consteval {
                        this->_start = typename Clock::time_point();
                    } else {
                        this->_start = Clock::now();
                    }
                }

                constexpr ~scope_measurer() {
                    if consteval {
                        this->_timer._duration = typename Clock::duration();
                    } else {
                        this->_timer._duration = Clock::now() - this->_start;
                    }
                }
            };

            return scope_measurer(self);
        }
    };

}
