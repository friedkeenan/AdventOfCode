module;

/* NOTE: Needed for 'SEEK_SET' and such. */
#include <cstdio>

export module advent:puzzle_data;

import std;

import :scope_guard;
import :print;
import :timer;

namespace advent {

    /* 'argv' is a pointer to a const pointer to a const 'char'. */
    export constexpr std::optional<std::string> puzzle_data(int argc, const char * const *argv) {
        /* NOTE: If we need more complex logic, we can put the args in a span. */
        if (argc < 2) {
            return std::nullopt;
        }

        const auto fp = std::fopen(argv[1], "r");
        if (fp == nullptr) {
            return std::nullopt;
        }

        advent::scope_guard _ = [&]() {
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

    export template<typename Solver>
    concept PuzzleSolver = std::invocable<Solver, const std::string &>;

    namespace impl {

        template<std::size_t N>
        struct fixed_string {
            std::array<char, N> _data;

            consteval explicit(false) fixed_string(const char (&data)[N + 1]) {
                std::ranges::copy_n(data + 0, N, this->_data.begin());
            }

            constexpr explicit(false) operator std::string_view(this const fixed_string &self) {
                return std::string_view(self._data);
            }
        };

        template<std::size_t N>
        fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

        template<std::size_t Index>
        requires (Index <= 1)
        constexpr inline auto default_solution_fmt_string = nullptr;

        template<>
        constexpr inline impl::fixed_string default_solution_fmt_string<0> = "Part one solution: {}\t(in {:.3})";

        template<>
        constexpr inline impl::fixed_string default_solution_fmt_string<1> = "Part two solution: {}\t(in {:.3})";

        template<typename Solution, typename Duration>
        struct solve_puzzle_result {
            Solution solution;
            Duration duration;
        };

        template<advent::PuzzleSolver Solver>
        constexpr auto solve_puzzle(Solver &&solver, const std::string &data) {
            advent::timer timer;

            return impl::solve_puzzle_result{
                [&]() {
                    auto _ = timer.measure_scope();

                    return std::invoke(std::forward<Solver>(solver), data);
                }(),

                timer.last_measured_duration()
            };
        }

        template<impl::fixed_string FormatString, advent::PuzzleSolver Solver>
        constexpr void print_solution(Solver &&solver, const std::string &data) {
            auto result = impl::solve_puzzle(std::forward<Solver>(solver), data);

            const auto print_for_duration = [&](const auto duration) {
                advent::println(FormatString, std::move(result.solution), duration);
            };

            if (result.duration >= std::chrono::seconds(1)) {
                using seconds = std::chrono::duration<std::float64_t>;

                const auto duration = std::chrono::duration_cast<seconds>(result.duration);

                print_for_duration(duration);
            } else if (result.duration >= std::chrono::milliseconds(1)) {
                using milliseconds = std::chrono::duration<std::float64_t, std::milli>;

                const auto duration = std::chrono::duration_cast<milliseconds>(result.duration);

                print_for_duration(duration);
            } else {
                using microseconds = std::chrono::duration<std::float64_t, std::micro>;

                const auto duration = std::chrono::duration_cast<microseconds>(result.duration);

                print_for_duration(duration);
            }
        }

    }

    export template<impl::fixed_string... FormatStrings, advent::PuzzleSolver... Solvers>
    requires (sizeof...(FormatStrings) == sizeof...(Solvers))
    constexpr int solve_puzzles(int argc, const char * const *argv, Solvers &&... solvers) {
        const auto data = advent::puzzle_data(argc, argv);
        if (!data.has_value()) {
            advent::println("Unable to read puzzle data!");

            return 1;
        }

        (impl::print_solution<FormatStrings>(std::forward<Solvers>(solvers), *data), ...);

        return 0;
    }

    export template<advent::PuzzleSolver... Solvers>
    requires (sizeof...(Solvers) <= 2)
    constexpr int solve_puzzles(int argc, char * const *argv, Solvers &&... solvers) {
        /*
            NOTE: We can't just call 'advent::solve_puzzles'
            specifying the default format strings because if
            we are passed no solvers then this function would
            be the one which gets called, resulting in an infinite loop.

            So for that reason we accept some mild code duplication.
        */

        const auto data = advent::puzzle_data(argc, argv);
        if (!data.has_value()) {
            advent::println("Unable to read puzzle data!");

            return 1;
        }

        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (
                impl::print_solution<
                    impl::default_solution_fmt_string<Indices>
                >(std::forward<Solvers>(solvers), *data),

                ...
            );
        }(std::make_index_sequence<sizeof...(Solvers)>{});

        return 0;
    }

}
