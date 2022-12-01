#pragma once

#include <advent/common.hpp>

namespace advent {

    /*
        Used to split strings and get strings back.

        I hope 'reconstructed_range' stuff gets in at some point.
    */
    class split_string_view : public std::ranges::view_interface<split_string_view> {
        public:
            class iterator {
                public:
                    using iterator_category = std::forward_iterator_tag;
                    using iterator_concept  = std::forward_iterator_tag;

                    using difference_type = std::ptrdiff_t;

                    using value_type = std::string_view;
                    using reference  = std::string_view;

                    std::string_view _str = {};
                    char             _delimiter = '\0';
                    std::size_t      _next_delimiter_pos = std::string_view::npos;

                    constexpr iterator() = default;

                    constexpr iterator(const std::string_view str, const char delimiter)
                        : _str(str), _delimiter(delimiter), _next_delimiter_pos(str.find_first_of(delimiter)) { }

                    constexpr iterator &operator ++() {
                        if (this->_next_delimiter_pos != std::string_view::npos) {
                            this->_str.remove_prefix(this->_next_delimiter_pos + 1);
                            this->_next_delimiter_pos = this->_str.find_first_of(this->_delimiter);
                        } else {
                            this->_str.remove_prefix(this->_str.size());
                        }

                        return *this;
                    }

                    constexpr ADVENT_RIGHT_UNARY_OP_FROM_LEFT(iterator, ++)

                    constexpr reference operator *() const {
                        if (this->_next_delimiter_pos == std::string_view::npos) {
                            return this->_str;
                        }

                        return std::string_view(this->_str.data(), this->_next_delimiter_pos);
                    }

                    constexpr bool operator ==(const iterator &rhs) const {
                        /* Whether each iterator is looking at the same position of the string. */
                        return this->_str.data() == rhs._str.data();
                    }

                    constexpr bool operator ==(std::default_sentinel_t) const {
                        return this->_str.empty();
                    }
            };

            std::string_view _str;
            char             _delimiter = '\0';

            constexpr split_string_view() = default;

            constexpr split_string_view(const std::string_view str, const char delimiter)
                : view_interface<split_string_view>(), _str(str), _delimiter(delimiter) { }

            constexpr iterator begin() const {
                return iterator(this->_str, this->_delimiter);
            }

            constexpr std::default_sentinel_t end() const {
                return std::default_sentinel;
            }
    };

    static_assert(std::ranges::forward_range<split_string_view>);
    static_assert(std::ranges::view<split_string_view>);

    namespace views {

        /* TODO: Make this a range adaptor when I receive an implementation of user-defined range adaptors. */
        constexpr inline auto split_string = [](const std::string_view str, const char delimiter) {
            return advent::split_string_view(str, delimiter);
        };

        constexpr inline auto split_lines = [](const std::string_view str) {
            return advent::views::split_string(str, '\n');
        };

    }

}
