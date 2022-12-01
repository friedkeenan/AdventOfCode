#pragma once

#include <advent/common.hpp>
#include <advent/scope_guard.hpp>

namespace advent {

    class puzzle_data : public std::optional<std::string> {
        public:
            /* TODO: 'constexpr' when I receive an implementation of constexpr 'std::string'. */

            /* 'argv' is a pointer to a const pointer to a const 'char'. */
            constexpr puzzle_data(int argc, const char * const *argv) : std::optional<std::string>() {
                /* NOTE: If we need more complex logic, we can put the args in a span. */
                if (argc < 2) {
                    return;
                }

                this->_populate_from_file(argv[1]);
            }

            inline void _populate_from_file(const char *path) {
                const auto fp = std::fopen(path, "r");
                if (fp == nullptr) {
                    return;
                }

                const advent::scope_guard fp_close = [&]() {
                    std::fclose(fp);
                };

                /* Get the size of the file. */
                if (std::fseek(fp, 0, SEEK_END) < 0) {
                    return;
                }

                const auto file_size = std::ftell(fp);

                if (std::fseek(fp, 0, SEEK_SET) < 0) {
                    return;
                }

                /* String with adequate buffer size. */
                std::string file_contents(file_size, '\0');

                if (std::cmp_not_equal(std::fread(file_contents.data(), 1, file_size, fp), file_size)) {
                    return;
                }

                this->emplace(file_contents);
            }
    };

    static_assert(!advent::puzzle_data(1, nullptr).has_value());

}
