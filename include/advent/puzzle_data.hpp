#pragma once

#include <advent/common.hpp>
#include <advent/scope_guard.hpp>

namespace advent {

    class puzzle_data : public std::optional<std::string> {
        public:

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

    template<typename Solver>
    concept PuzzleSolver = std::invocable<Solver, const std::string &>;

    template<std::size_t Index>
    requires (Index <= 1)
    constexpr inline auto _solution_fmt_string = nullptr;

    template<>
    constexpr inline std::string_view _solution_fmt_string<0> = "Part one solution: {}\n";

    template<>
    constexpr inline std::string_view _solution_fmt_string<1> = "Part two solution: {}\n";

    template<PuzzleSolver... Solvers>
    requires (sizeof...(Solvers) <= 2)
    int solve_puzzles(int argc, const char * const *argv, Solvers &&... solvers) {
        const auto data = advent::puzzle_data(argc, argv);
        if (!data.has_value()) {
            fmt::print("Unable to read puzzle data!\n");

            return 1;
        }

        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (fmt::print(_solution_fmt_string<Indices>, std::invoke(std::forward<Solvers>(solvers), *data)), ...);
        }(std::make_index_sequence<sizeof...(Solvers)>{});

        // if constexpr (sizeof...(Solvers) > 0) {
        //     const auto part_one_solution = std::invoke(std::forward<PartOneSolver>(part_one_solver), *data);
        //     const auto part_two_solution = std::invoke(std::forward<PartTwoSolver>(part_two_solver), *data);

        //     fmt::print("Part one solution: {}\n", part_one_solution);
        //     fmt::print("Part two solution: {}\n", part_two_solution);
        // }

        return 0;
    }

}
