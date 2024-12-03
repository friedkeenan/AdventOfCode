#pragma once

#include <advent/common.hpp>
#include <advent/scope_guard.hpp>
#include <advent/print.hpp>

namespace advent {

    /* 'argv' is a pointer to a const pointer to a const 'char'. */
    constexpr std::optional<std::string> puzzle_data(int argc, const char * const *argv) {
        /* NOTE: If we need more complex logic, we can put the args in a span. */
        if (argc < 2) {
            return std::nullopt;
        }

        const auto fp = std::fopen(argv[1], "r");
        if (fp == nullptr) {
            return std::nullopt;
        }

        const advent::scope_guard fp_close = [&]() {
            std::fclose(fp);
        };

        /* Get the size of the file. */
        if (std::fseek(fp, 0, SEEK_END) < 0) {
            return std::nullopt;
        }

        const auto file_size = std::ftell(fp);

        if (std::fseek(fp, 0, SEEK_SET) < 0) {
            return std::nullopt;
        }

        std::string file_contents;
        bool read_succeeded;
        file_contents.resize_and_overwrite(file_size, [&](char *data, const std::size_t size) {
            read_succeeded = (std::fread(data, 1, size, fp) == size);

            return size;
        });

        if (!read_succeeded) {
            return std::nullopt;
        }

        return file_contents;
    }

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
    constexpr int solve_puzzles(int argc, const char * const *argv, Solvers &&... solvers) {
        const auto data = advent::puzzle_data(argc, argv);
        if (!data.has_value()) {
            advent::print("Unable to read puzzle data!\n");

            return 1;
        }

        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (advent::print(_solution_fmt_string<Indices>, std::invoke(std::forward<Solvers>(solvers), *data)), ...);
        }(std::make_index_sequence<sizeof...(Solvers)>{});

        return 0;
    }

}
