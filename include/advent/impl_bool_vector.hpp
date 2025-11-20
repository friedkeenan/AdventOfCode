#pragma once

#include <advent/common.hpp>

/*
    This is to be used where our code expects regular vector semantics,
    such as being contiguous and being able to get normal pointers and such.

    This does not provide the entire std::vector API, and only provides
    what our code needs. Its provided API may be expanded as new needs arise.
*/

namespace advent::impl {

    struct bool_vector {
        using allocator_type    = std::allocator<bool>;
        using _allocator_traits = std::allocator_traits<allocator_type>;

        /* It's faster to store the pointers, apparently. */
        bool *_data;
        bool *_size_end;
        bool *_capacity_end;

        /*
            NOTE: We don't bother with this in copying or moving
            since all 'std::allocator<T>' objects are identical.

            In fact we only have it to use with 'std::allocator_traits'.
        */
        [[no_unique_address]] allocator_type _allocator;

        constexpr bool_vector()
        :
            _data(nullptr),
            _size_end(nullptr),
            _capacity_end(nullptr)
        {}

        constexpr auto _allocate_at_least(this bool_vector &self, const std::size_t n) {
            /* TODO: Change to use 'allocate_at_least' when I receive an implementation. */

            struct allocation_result {
                bool *ptr;
                std::size_t count;
            };

            return allocation_result{_allocator_traits::allocate(self._allocator, n), n};
        }

        constexpr bool_vector(const std::size_t count) {
            const auto [data, capacity] = this->_allocate_at_least(count);

            this->_data         = data;
            this->_size_end     = data + count;
            this->_capacity_end = data + capacity;

            /* NOTE: 'std::ranges::uninitialized_default_construct' is not constexpr. */
            for (const auto ptr : std::views::iota(this->_data, this->_size_end)) {
                std::construct_at(ptr);
            }
        }

        constexpr void _destroy_elements(this bool_vector &self) {
            std::ranges::destroy(self);
        }

        constexpr void _deallocate(this bool_vector &self) {
            if (self._data == nullptr) {
                return;
            }

            _allocator_traits::deallocate(self._allocator, self.data(), self.capacity());
        }

        constexpr void _destroy_and_deallocate(this bool_vector &self) {
            self._destroy_elements();
            self._deallocate();
        }

        constexpr ~bool_vector() {
            this->_destroy_and_deallocate();
        }

        constexpr bool_vector(bool_vector &&other)
        :
            _data(std::exchange(other._data, nullptr)),
            _size_end(std::exchange(other._size_end, nullptr)),
            _capacity_end(std::exchange(other._capacity_end, nullptr))
        {}

        constexpr bool_vector &operator =(this bool_vector &self, bool_vector &&other) {
            if (&self == &other) {
                return self;
            }

            self._destroy_and_deallocate();

            self._data         = std::exchange(other._data,         nullptr);
            self._size_end     = std::exchange(other._size_end,     nullptr);
            self._capacity_end = std::exchange(other._capacity_end, nullptr);

            return self;
        }

        constexpr bool_vector(const bool_vector &other) {
            const auto [data, capacity] = this->_allocate_at_least(other.size());

            this->_data         = data;
            this->_size_end     = data + other.size();
            this->_capacity_end = data + capacity;

            /* NOTE: 'std::ranges::uninitialized_copy' is not constexpr. */
            for (const auto [self_ptr, other_elem] : std::views::zip(
                std::views::iota(this->_data),

                other
            )) {
                std::construct_at(self_ptr, other_elem);
            }
        }

        constexpr bool_vector &operator =(this bool_vector &self, const bool_vector &other) {
            if (&self == &other) {
                return self;
            }

            if (self.size() >= other.size()) {
                std::ranges::copy(other, self.begin());

                std::ranges::destroy_n(self.begin() + other.size(), self.size() - other.size());

                self._size_end = self._data + other.size();

                return self;
            }

            if (other.size() <= self.capacity()) {
                std::ranges::copy_n(other.begin(), self.size(), self.begin());

                /* NOTE: 'std::ranges::uninitialized_copy' is not constexpr. */
                for (const auto [self_ptr, other_elem] : std::views::zip(
                    std::views::iota(self._size_end),

                    std::ranges::subrange(other.begin() + self.size(), other.end())
                )) {
                    std::construct_at(self_ptr, other_elem);
                }

                self._size_end = self._data + other.size();

                return self;
            }

            /* We must reallocate, so just discard everything. */
            self._destroy_and_deallocate();

            const auto [data, capacity] = self._allocate_at_least(other.size());

            self._data         = data;
            self._size_end     = data + other.size();
            self._capacity_end = data + capacity;

            /* NOTE: 'std::ranges::uninitialized_copy' is not constexpr. */
            for (const auto [self_ptr, other_elem] : std::views::zip(
                std::views::iota(self._data),

                other
            )) {
                std::construct_at(self_ptr, other_elem);
            }

            return self;
        }

        constexpr void reserve(this bool_vector &self, const std::size_t requested_capacity) {
            static constexpr std::size_t GrowthFactor = 2;

            if (requested_capacity <= self.capacity()) {
                return;
            }

            const auto grown_capacity = [&]() {
                if (GrowthFactor * self.capacity() <= requested_capacity) {
                    return requested_capacity;
                }

                return GrowthFactor * self.capacity();
            }();

            const auto [data, capacity] = self._allocate_at_least(grown_capacity);

            /* NOTE: 'std::ranges::uninitialized_move' is not constexpr. */
            for (const auto [new_ptr, elem] : std::views::zip(
                std::views::iota(data),

                self
            )) {
                std::construct_at(new_ptr, std::move(elem));
            }

            const auto size = self.size();

            self._destroy_and_deallocate();

            self._data         = data;
            self._size_end     = data + size;
            self._capacity_end = data + capacity;
        }

        constexpr void resize(this bool_vector &self, const std::size_t count) {
            if (count <= self.size()) {
                return;
            }

            if (count > self.capacity()) {
                self.reserve(count);
            }

            /* NOTE: 'std::ranges::uninitialized_default_construct' is not constexpr. */
            for (const auto ptr : std::views::iota(self._size_end, self._data + count)) {
                std::construct_at(ptr);
            }

            self._size_end = self._data + count;
        }

        constexpr bool *data(this bool_vector &self) {
            return self._data;
        }

        constexpr const bool *data(this const bool_vector &self) {
            return self._data;
        }

        constexpr std::size_t size(this const bool_vector &self) {
            return self._size_end - self._data;
        }

        constexpr std::size_t capacity(this const bool_vector &self) {
            return self._capacity_end - self._data;
        }

        constexpr bool *begin(this bool_vector &self) {
            return self._data;
        }

        constexpr const bool *begin(this const bool_vector &self) {
            return self._data;
        }

        constexpr bool *end(this bool_vector &self) {
            return self._size_end;
        }

        constexpr const bool *end(this const bool_vector &self) {
            return self._size_end;
        }

        constexpr auto &front(this auto &self) {
            [[assume(self.size() > 0)]];

            return *self.begin();
        }

        constexpr auto &back(this auto &self) {
            [[assume(self.size() > 0)]];

            return *(self.end() - 1);
        }

        constexpr auto &operator [](this auto &self, const std::size_t index) {
            [[assume(index < self.size())]];

            return self.data()[index];
        }
    };

    static_assert(std::ranges::contiguous_range<impl::bool_vector>);
    static_assert(std::ranges::sized_range<impl::bool_vector>);
    static_assert(std::ranges::common_range<impl::bool_vector>);

    template<typename T>
    using regular_vector = std::conditional_t<std::same_as<T, bool>, impl::bool_vector, std::vector<T>>;

}
