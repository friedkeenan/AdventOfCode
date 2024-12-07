#pragma once

#include <advent/common.hpp>
#include <advent/scope_guard.hpp>
#include <advent/timer.hpp>
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

        #ifdef ADVENT_TIME_SOLUTIONS

        template<>
        constexpr inline impl::fixed_string default_solution_fmt_string<0> = "Part one solution: {}\t(in {:.3})\n";

        template<>
        constexpr inline impl::fixed_string default_solution_fmt_string<1> = "Part two solution: {}\t(in {:.3})\n";

        #else

        template<>
        constexpr inline impl::fixed_string default_solution_fmt_string<0> = "Part one solution: {}\n";

        template<>
        constexpr inline impl::fixed_string default_solution_fmt_string<1> = "Part two solution: {}\n";

        #endif

        #ifdef ADVENT_TIME_SOLUTIONS

        template<typename Solution, typename Duration>
        struct solve_puzzle_result {
            Solution solution;
            Duration duration;
        };

        #else

        template<typename Solution>
        struct solve_puzzle_result {
            Solution solution;
        };

        #endif

        template<advent::PuzzleSolver Solver>
        constexpr auto solve_puzzle(Solver &&solver, const std::string &data) {
            #ifdef ADVENT_TIME_SOLUTIONS

            advent::timer timer;

            return impl::solve_puzzle_result{
                [&]() {
                    const auto measurer = timer.measure_scope();

                    return std::invoke(std::forward<Solver>(solver), data);
                }(),

                timer.last_measured_duration()
            };

            #else

            return impl::solve_puzzle_result{
                std::invoke(std::forward<Solver>(solver), data)
            };

            #endif
        }

        template<impl::fixed_string FormatString, advent::PuzzleSolver Solver>
        constexpr void print_solution(Solver &&solver, const std::string &data) {
            /*
                NOTE: We need to make the format string into a string view
                so that it'll play nice with 'fmt::format_string'.
            */
            static constexpr std::string_view FormatStringView = FormatString;

            auto result = impl::solve_puzzle(std::forward<Solver>(solver), data);

            #ifdef ADVENT_TIME_SOLUTIONS

            const auto print_for_duration = [&](const auto duration) {
                advent::print(FormatStringView, std::move(result.solution), duration);
            };

            /* NOTE: fmtlib doesn't know about 'std::float64_t'. */

            if (result.duration >= std::chrono::seconds(1)) {
                using seconds = std::chrono::duration<double>;

                const auto duration = std::chrono::duration_cast<seconds>(result.duration);

                print_for_duration(duration);
            } else if (result.duration >= std::chrono::milliseconds(1)) {
                using milliseconds = std::chrono::duration<double, std::milli>;

                const auto duration = std::chrono::duration_cast<milliseconds>(result.duration);

                print_for_duration(duration);
            } else {
                using microseconds = std::chrono::duration<double, std::micro>;

                const auto duration = std::chrono::duration_cast<microseconds>(result.duration);

                print_for_duration(duration);
            }

            #else

            advent::print(FormatStringView, std::move(result.solution));

            #endif
        }

    }

    template<impl::fixed_string... FormatStrings, advent::PuzzleSolver... Solvers>
    requires (sizeof...(FormatStrings) == sizeof...(Solvers))
    constexpr int solve_puzzles(int argc, const char * const *argv, Solvers &&... solvers) {
        const auto data = advent::puzzle_data(argc, argv);
        if (!data.has_value()) {
            advent::print("Unable to read puzzle data!\n");

            return 1;
        }

        (impl::print_solution<FormatStrings>(std::forward<Solvers>(solvers), *data), ...);

        return 0;
    }

    template<advent::PuzzleSolver... Solvers>
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
            advent::print("Unable to read puzzle data!\n");

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
