export module advent:vector_nd;

import std;

import :concepts;
import :math;

namespace advent {

    export struct dimension {
        static const dimension x;
        static const dimension y;
        static const dimension z;

        std::size_t _index;

        constexpr auto operator <=>(const dimension &) const = default;
    };

    constexpr advent::dimension dimension::x = {0};
    constexpr advent::dimension dimension::y = {1};
    constexpr advent::dimension dimension::z = {2};

    export template<advent::arithmetic T, std::size_t Size>
    struct vector_nd {
        std::array<T, Size> _storage;

        constexpr T &operator[](this vector_nd &self, const std::size_t index) {
            [[assume(index < Size)]];

            return self._storage[index];
        }

        constexpr const T &operator [](this const vector_nd &self, const std::size_t index) {
            [[assume(index < Size)]];

            return self._storage[index];
        }

        constexpr T &operator [](this vector_nd &self, const advent::dimension dimension) {
            return self[dimension._index];
        }

        constexpr const T &operator [](this const vector_nd &self, const advent::dimension dimension) {
            return self[dimension._index];
        }

        constexpr T &x(this vector_nd &self) requires (Size >= 1) {
            return self[advent::dimension::x];
        }

        constexpr const T &x(this const vector_nd &self) requires (Size >= 1) {
            return self[advent::dimension::x];
        }

        constexpr T &y(this vector_nd &self) requires (Size >= 2) {
            return self[advent::dimension::y];
        }

        constexpr const T &y(this const vector_nd &self) requires (Size >= 2) {
            return self[advent::dimension::y];
        }

        constexpr T &z(this vector_nd &self) requires (Size >= 3) {
            return self[advent::dimension::z];
        }

        constexpr const T &z(this const vector_nd &self) requires (Size >= 3) {
            return self[advent::dimension::z];
        }

        constexpr T mag_squared(this const vector_nd &self) {
            T result = 0;

            for (const auto &elem : self) {
                result += elem * elem;
            }

            return result;
        }

        template<typename Other>
        constexpr auto dist_squared(this const vector_nd &self, const vector_nd<Other, Size> &other) {
            using Result = decltype(std::declval<T>() - std::declval<Other>());

            Result result = 0;

            for (const auto i : std::views::iota(0uz, Size)) {
                const auto distance_1d = self[i] - other[i];

                result += distance_1d * distance_1d;
            }

            return result;
        }

        template<typename Other>
        constexpr auto manhattan_distance(this const vector_nd &self, const vector_nd<Other, Size> &other) {
            using Result = decltype(advent::abs(std::declval<T>() - std::declval<Other>()));

            Result result = 0;

            for (const auto i : std::views::iota(0uz, Size)) {
                result += advent::abs(self[i] - other[i]);
            }

            return result;
        }

        constexpr auto operator <=>(const vector_nd &) const = default;

        template<typename Other>
        constexpr vector_nd &operator +=(this vector_nd &self, const vector_nd<Other, Size> &other) {
            for (const auto i : std::views::iota(0uz, Size)) {
                self[i] += other[i];
            }

            return self;
        }

        template<typename Other>
        constexpr auto operator +(this const vector_nd &self, const vector_nd<Other, Size> &other) {
            using Result = decltype(std::declval<T>() + std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = self[i] + other[i];
            }

            return vector_nd{result_storage};
        }

        template<typename Other>
        constexpr vector_nd &operator -=(this vector_nd &self, const vector_nd<Other, Size> &other) {
            for (const auto i : std::views::iota(0uz, Size)) {
                self[i] -= other[i];
            }

            return self;
        }

        template<typename Other>
        constexpr auto operator -(this const vector_nd &self, const vector_nd<Other, Size> &other) {
            using Result = decltype(std::declval<T>() - std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = self[i] - other[i];
            }

            return vector_nd{result_storage};
        }

        template<advent::arithmetic Other>
        constexpr vector_nd &operator *=(this vector_nd &self, const Other other) {
            for (auto &elem : self) {
                elem *= other;
            }

            return self;
        }

        template<advent::arithmetic Other>
        constexpr auto operator *(this const vector_nd &self, const Other other) {
            using Result = decltype(std::declval<T>() * std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = self[i] * other[i];
            }

            return vector_nd{result_storage};
        }

        template<advent::arithmetic Other>
        constexpr vector_nd &operator /=(this vector_nd &self, const Other other) {
            for (auto &elem : self) {
                elem /= other;
            }

            return self;
        }

        template<advent::arithmetic Other>
        constexpr auto operator /(this const vector_nd &self, const Other other) {
            using Result = decltype(std::declval<T>() / std::declval<Other>());

            std::array<Result, Size> result_storage;
            for (const auto i : std::views::iota(0uz, Size)) {
                result_storage[i] = self[i] / other[i];
            }

            return vector_nd{result_storage};
        }

        /* Range facilities. */

        constexpr T *data(this vector_nd &self) {
            return self._storage.data();
        }

        constexpr const T *data(this const vector_nd &self) {
            return self._storage.data();
        }

        constexpr std::size_t size(this const vector_nd &) {
            return Size;
        }

        constexpr auto begin(this vector_nd &self) {
            return self._storage.begin();
        }

        constexpr auto begin(this const vector_nd &self) {
            return self._storage.begin();
        }

        constexpr auto end(this vector_nd &self) {
            return self._storage.end();
        }

        constexpr auto end(this const vector_nd &self) {
            return self._storage.end();
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

        auto format(this const formatter &self, const advent::vector_nd<T, Size> &vector, auto &ctx) {
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
