#pragma once

/*
    TODO: Remove this once constexpr 'std::vector' is implemented.

    This code only exists to facilitate dynamic data in constexpr
    until I acquire an STL implementation that obsoletes this.
*/

#include <advent/common.hpp>
#include <advent/concepts.hpp>

namespace advent {

    /* I don't think we care about being allocator aware here. */
    template<typename Element>
    class basic_vector {
        public:
            using value_type      = Element;
            using size_type       = std::size_t;
            using difference_type = std::ptrdiff_t;
            using reference       = Element &;
            using const_reference = const Element &;
            using pointer         = Element *;
            using const_pointer   = const Element *;
            using iterator        = pointer;
            using const_iterator  = const_pointer;

            static constexpr size_type NewCapacityRatio = 2;

            pointer   _data     = nullptr;
            size_type _size     = 0;
            size_type _capacity = 0;

            [[no_unique_address]] std::allocator<Element> _allocator = std::allocator<Element>();

            constexpr basic_vector() = default;

            template<std::ranges::input_range R>
            constexpr explicit basic_vector(R &&rng) {
                if constexpr (std::ranges::sized_range<R>) {
                    this->reserve(std::ranges::size(rng));

                    std::ranges::copy(rng, this->data());
                    this->_size = this->capacity();
                } else {
                    for (auto &&elem : std::forward<R>(rng)) {
                        this->push_back(std::forward<decltype(elem)>(elem));
                    }
                }
            }

            constexpr ~basic_vector() {
                this->_cleanup_data();
            }

            constexpr void _free_data() {
                if (this->capacity() > 0) {
                    this->_allocator.deallocate(this->data(), this->capacity());
                }
            }

            constexpr void _cleanup_data() {
                for (const auto location : std::views::iota(this->data(), this->data() + this->size())) {
                    std::destroy_at(location);
                }

                this->_free_data();
            }

            constexpr void _reserve_and_move(const size_type new_capacity) {
                advent::assume(new_capacity >= this->size());

                const auto new_data = this->_allocator.allocate(new_capacity);

                /* Move each element into its new location. */
                auto new_location = new_data;
                for (auto &elem : *this) {
                    std::construct_at(new_location, std::move(elem));
                    std::destroy_at(std::addressof(elem));

                    new_location++;
                }

                this->_free_data();

                this->_data     = new_data;
                this->_capacity = new_capacity;
            }

            constexpr void reserve(const size_type new_capacity) {
                if (new_capacity <= this->capacity()) {
                    return;
                }

                this->_reserve_and_move(new_capacity);
            }

            template<typename... Args>
            requires (std::constructible_from<Element, Args...>)
            constexpr reference emplace_back(Args &&... args) {
                /* We call '_reserve_and_move' to avoid checks of current capacity. */
                if (this->capacity() == 0) {
                    this->_reserve_and_move(1);
                } else if (this->size() + 1 > this->capacity()) {
                    this->_reserve_and_move(NewCapacityRatio * this->capacity());
                }

                const auto location = this->data() + this->size();
                std::construct_at(location, std::forward<Args>(args)...);
                this->_size++;

                return *location;
            }

            template<advent::forwarder_for<Element> ElementFwd>
            constexpr void push_back(ElementFwd &&new_elem) {
                this->emplace_back(std::forward<ElementFwd>(new_elem));
            }

            constexpr size_type size() const {
                return this->_size;
            }

            constexpr size_type capacity() const {
                return this->_capacity;
            }

            constexpr pointer data() {
                return this->_data;
            }

            constexpr const_pointer data() const {
                return this->_data;
            }

            constexpr iterator begin() {
                return this->data();
            }

            constexpr const_iterator begin() const {
                return this->data();
            }

            constexpr iterator end() {
                return this->data() + this->size();
            }

            constexpr const_iterator end() const {
                return this->data() + this->size();
            }
    };

    template<std::ranges::input_range R>
    basic_vector(R &&) -> basic_vector<std::ranges::range_value_t<R>>;

}
