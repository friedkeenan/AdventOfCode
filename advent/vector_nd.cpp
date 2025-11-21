export module advent:vector_nd;

import std;

import :concepts;
import :math;

namespace advent {

    export template<advent::arithmetic T, std::size_t Size>
    struct vector_nd {
        std::array<T, Size> _storage;

        constexpr T &_at(const std::size_t index) {
            [[assume(index < Size)]];

            return this->_storage[index];
        }

        constexpr const T &_at(const std::size_t index) const {
            [[assume(index < Size)]];

            return this->_storage[index];
        }

        constexpr T &operator [](const std::size_t index) {
            return this->_at(index);
        }

        constexpr const T &operator [](const std::size_t index) const {
            return this->_at(index);
        }

        constexpr T &x() requires (Size >= 1) {
            return this->_at(0);
        }

        constexpr const T &x() const requires (Size >= 1) {
            return this->_at(0);
        }

        constexpr T &y() requires (Size >= 2) {
            return this->_at(1);
        }

        constexpr const T &y() const requires (Size >= 2) {
            return this->_at(1);
        }

        constexpr T &z() requires (Size >= 3) {
            return this->_at(2);
        }

        constexpr const T &z() const requires (Size >= 3) {
            return this->_at(2);
        }

        constexpr T mag_squared() const {
            T result = 0;

            for (const auto &elem : *this) {
                result += elem * elem;
            }

            return result;
        }

        template<typename Other>
        constexpr auto dist_squared(const vector_nd<Other, Size> &other) const {
            using Result = decltype(std::declval<T>() - std::declval<Other>());

            Result result = 0;

            for (const auto i : std::views::iota(0uz, Size)) {
                const auto distance_1d = this->_at(i) - other[i];

                result += distance_1d * distance_1d;
            }

            return result;
        }

        template<typename Other>
        constexpr auto manhattan_distance(const vector_nd<Other, Size> &other) const {
            using Result = decltype(advent::abs(std::declval<T>() - std::declval<Other>()));

            Result result = 0;

            for (const auto i : std::views::iota(0uz, Size)) {
                result += advent::abs(this->_at(i) - other[i]);
            }

            return result;
        }

        constexpr auto operator <=>(const vector_nd &) const = default;

        template<typename Other>
        constexpr vector_nd &operator +=(const vector_nd<Other, Size> &other) {
            for (const auto i : std::views::iota(0uz, Size)) {
                this->_at(i) += other[i];
            }

            return *this;
        }

        template<typename Other>
        constexpr auto operator +(const vector_nd<Other, Size> &other) const {
            using Result = decltype(std::declval<T>() + std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = this->_at(i) + other[i];
            }

            return vector_nd{result_storage};
        }

        template<typename Other>
        constexpr vector_nd &operator -=(const vector_nd<Other, Size> &other) {
            for (const auto i : std::views::iota(0uz, Size)) {
                this->_at(i) -= other[i];
            }

            return *this;
        }

        template<typename Other>
        constexpr auto operator -(const vector_nd<Other, Size> &other) const {
            using Result = decltype(std::declval<T>() - std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = this->_at(i) - other[i];
            }

            return vector_nd{result_storage};
        }

        template<advent::arithmetic Other>
        constexpr vector_nd &operator *=(const Other other) {
            for (auto &elem : *this) {
                elem *= other;
            }

            return *this;
        }

        template<advent::arithmetic Other>
        constexpr auto operator *(const Other other) const {
            using Result = decltype(std::declval<T>() * std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = this->_at(i) * other[i];
            }

            return vector_nd{result_storage};
        }

        template<advent::arithmetic Other>
        constexpr vector_nd &operator /=(const Other other) {
            for (auto &elem : *this) {
                elem /= other;
            }

            return *this;
        }

        template<advent::arithmetic Other>
        constexpr auto operator /(const Other other) const {
            using Result = decltype(std::declval<T>() / std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = this->_at(i) / other[i];
            }

            return vector_nd{result_storage};
        }

        /* Range facilities. */

        constexpr T *data() {
            return this->_storage.data();
        }

        constexpr const T *data() const {
            return this->_storage.data();
        }

        constexpr std::size_t size() const {
            return Size;
        }

        constexpr auto begin() {
            return this->_storage.begin();
        }

        constexpr auto begin() const {
            return this->_storage.begin();
        }

        constexpr auto end() {
            return this->_storage.end();
        }

        constexpr auto end() const {
            return this->_storage.end();
        }
    };

    template<advent::arithmetic Head, advent::arithmetic... Tail>
    vector_nd(Head, Tail...) -> vector_nd<Head, sizeof...(Tail) + 1>;

    export template<advent::arithmetic T>
    using vector_2d = vector_nd<T, 2>;

    static_assert(std::ranges::contiguous_range<vector_2d<int>>);
    static_assert(std::ranges::sized_range<vector_2d<int>>);

}

namespace std {

    /* User-defined formatter for 'advent::vector_nd'. */
    template<typename T, std::size_t Size>
    struct formatter<advent::vector_nd<T, Size>> : std::formatter<T> {
        /* Since we inherit from 'std::formatter<T>', we inherit its format specifiers. */

        /*
            NOTE: We *could* skip parsing the format specifier for
            when we have 0 elements but I don't want to put time into
            that edge case for what is really just a debug feature, and
            I would want to make sure that having a format specifier
            for 0 elements doesn't error so as to not frustrate generic
            code, and so maybe it's still better to parse the format
            specifier to detect errors in it for those cases anyways.
        */

        std::format_context::iterator format(this const formatter &self, const advent::vector_nd<T, Size> &vector, auto &ctx) {
            auto it = ctx.out();

            *it = '(';
            ++it;

            if constexpr (Size == 0) {
                *it = ')';
                ++it;

                return it;
            }

            ctx.advance_to(it);

            for (const auto i : std::views::iota(0uz, Size - 1)) {
                const auto elem = vector[i];
                self.std::formatter<T>::format(elem, ctx);

                it = ctx.out();

                *it = ',';
                ++it;

                *it = ' ';
                ++it;

                ctx.advance_to(it);
            }

            self.std::formatter<T>::format(vector[Size - 1], ctx);

            it = ctx.out();
            *it = ')';
            ++it;

            return it;
        }
    };

}
