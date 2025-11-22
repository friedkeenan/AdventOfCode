#include <advent/defines.hpp>

export module advent:split_string_view;

import std;

namespace advent {

    /*
        Used to split strings and get strings back.

        I hope 'reconstructed_range' stuff gets in at some point.
    */
    export struct split_string_view : std::ranges::view_interface<split_string_view> {
        struct iterator {
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
                    this->_str = std::string_view();
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
                return this->_str.data() == nullptr;
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

        namespace impl {

            /* NOTE: We don't do any fancy generic stuff because we don't need it. */

            struct split_range_adaptor {
                struct closure : std::ranges::range_adaptor_closure<closure> {
                    char _delimiter;

                    constexpr explicit closure(const char delimiter) : _delimiter(delimiter) { }

                    constexpr auto operator ()(const std::string_view str) const {
                        return advent::split_string_view(str, this->_delimiter);
                    }
                };

                static constexpr auto operator ()(const std::string_view str, const char delimiter) {
                    return advent::split_string_view(str, delimiter);
                }

                static constexpr auto operator ()(const char delimiter) {
                    return closure{delimiter};
                }
            };

            struct split_lines_adaptor_closure : std::ranges::range_adaptor_closure<split_lines_adaptor_closure> {
                static constexpr auto operator ()(const std::string_view str) {
                    return advent::split_string_view(str, '\n');
                }
            };

        }

        export constexpr inline auto split_string = impl::split_range_adaptor{};

        export constexpr inline auto split_lines = impl::split_lines_adaptor_closure{};

    }

    export template<typename Callback>
    requires (std::invocable<Callback &, std::string_view>)
    constexpr void split_with_callback(std::string_view str, const std::string_view delimiter, Callback &&callback) {
        /* Used for cases where we just need to simply loop over each substring. */

        while (true) {
            const auto split_pos = str.find(delimiter);
            if (split_pos == std::string_view::npos) {
                std::invoke(callback, std::move(str));

                return;
            }

            std::invoke(callback, std::string_view(str.data(), split_pos));

            str.remove_prefix(split_pos + delimiter.length());
        }
    }

    export template<typename Callback>
    requires (std::invocable<Callback &, std::string_view>)
    constexpr void split_with_callback(std::string_view str, const char delimiter, Callback &&callback) {
        return split_with_callback(str, std::string_view(&delimiter, 1), std::forward<Callback>(callback));
    }

}

namespace std::ranges {

    template<>
    constexpr inline bool enable_borrowed_range<advent::split_string_view> = true;

}
