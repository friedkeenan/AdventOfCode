module;

/* NOTE: Needed for 'SEEK_SET' and such. */
#include <cstdio>

export module advent:puzzle_data;

import std;

import :scope_guard;
import :print;
import :timer;
import :split_string_view;

namespace advent {
    namespace impl {

        template<std::size_t Index>
        struct part_solver_storage;

        consteval auto part_has_solver(std::size_t index) -> bool {
            return is_complete_type(substitute(^^impl::part_solver_storage, {
                std::meta::reflect_constant(index)
            }));
        }

        template<std::meta::info Solver, std::meta::info... TemplateArgs>
        struct part_solver_storage_member {};

        template<typename T>
        concept not_void = not std::is_void_v<T>;

        /*
            NOTE: Unfortunately when splicing the 'TemplateArgs', we need
            to have different syntax for splicing values versus splicing types.

            And so to handle this we have multiple specializations of this
            variable so that we can accommodate the required specialized syntax.

            TODO: The above could be handled instead by code generation.
        */
        template<typename Input, std::meta::info Solver, std::meta::info... TemplateArgs>
        constexpr inline bool can_solve_with_input = false;

        template<typename Input, std::meta::info Solver, std::meta::info... TemplateArgs>
        requires ((is_value(TemplateArgs) or is_object(TemplateArgs)) and ...)
        constexpr inline bool can_solve_with_input<Input, Solver, TemplateArgs...> = requires {
            { template[: Solver :]<([: TemplateArgs :])...>(std::declval<Input>()) } -> impl::not_void;
        };

        template<typename Input, std::meta::info Solver, std::meta::info... TemplateArgs>
        requires (is_type(TemplateArgs) and ...)
        constexpr inline bool can_solve_with_input<Input, Solver, TemplateArgs...> = requires {
            { template[: Solver :]<typename[: TemplateArgs :]...>(std::declval<Input>()) } -> impl::not_void;
        };

        /* A specialization for calling the solver with no template arguments. */
        template<typename Input, std::meta::info Solver>
        constexpr inline bool can_solve_with_input<Input, Solver> = requires {
            { [: Solver :](std::declval<Input>()) } -> impl::not_void;
        };

        template<typename Input, std::meta::info Solver, std::meta::info... TemplateArgs>
        constexpr auto solver_function(Input &&input) -> decltype(auto) {
            if constexpr (impl::can_solve_with_input<Input, Solver, TemplateArgs...>) {
                if constexpr (sizeof...(TemplateArgs) == 0) {
                    return [: Solver :](std::forward<Input>(input));
                } else if constexpr ((is_type(TemplateArgs) and ...)) {
                    return template[: Solver :]<typename[: TemplateArgs :]...>(
                        std::forward<Input>(input)
                    );
                } else {
                    return template[: Solver :]<([: TemplateArgs :])...>(
                        std::forward<Input>(input)
                    );
                }
            } else if constexpr (std::convertible_to<Input, std::string_view>) {
                return impl::solver_function<advent::split_string_view, Solver, TemplateArgs...>(
                    advent::views::split_lines(std::forward<Input>(input))
                );
            } else {
                static_assert(false, "Unable to call solver with provided input");
            }
        }

        struct solver_info {
            std::vector<std::meta::info> _info;

            consteval explicit solver_info(std::size_t index) {
                const auto storage = substitute(^^impl::part_solver_storage, {
                    std::meta::reflect_constant(index)
                });

                const auto member = nonstatic_data_members_of(
                    storage, std::meta::access_context::unchecked()
                )[0];

                this->_info = template_arguments_of(type_of(member));

                for (auto &info : std::ranges::subrange(this->_info).next()) {
                    const auto extracted = extract<std::meta::info>(info);

                    if (remove_cvref(type_of(extracted)) == ^^typename std::meta::info) {
                        info = extracted;
                    }
                }
            }

            consteval auto solver_function_with_template_args(
                this const solver_info &self,

                std::initializer_list<std::meta::info> template_args,

                std::meta::info input_type
            ) -> std::meta::info {
                auto args = std::vector{input_type, self._info.front()};

                for (auto arg : template_args) {
                    if (remove_cvref(type_of(arg)) != ^^typename std::meta::info) {
                        arg = std::meta::reflect_constant(arg);
                    }

                    args.push_back(arg);
                }

                return substitute(^^impl::solver_function, args);
            }

            consteval auto solver_function(this const solver_info &self, std::meta::info input_type) -> std::meta::info {
                auto args = std::vector{input_type};

                args.append_range(self._info);

                return substitute(^^impl::solver_function, args);
            }
        };

        consteval auto find_example_data() -> std::string_view {
            for (const auto member : members_of(^^::, std::meta::access_context::unprivileged())) {
                if (not has_identifier(member)) {
                    continue;
                }

                if (identifier_of(member) != "example_data") {
                    continue;
                }

                /* We found a thing named 'example_data', now verify that it's correct. */

                if (not is_variable(member)) {
                    throw std::meta::exception("Example data must be a variable", ^^impl::find_example_data);
                }

                if (type_of(member) != ^^const std::string_view) {
                    throw std::meta::exception("Example data must be a 'std::string_view'", ^^impl::find_example_data);
                }

                return extract<const std::string_view &>(member);
            }

            throw std::meta::exception("Unable to find example data", ^^impl::find_example_data);
        }

        template<std::size_t Index>
        struct part_print_string_storage;

    }

    /*
        NOTE: Each 'part' having its own type allows us to
        act like a normal function when called, which is
        nice in general, but is particularly helpful for
        improving the diagnostic messages for 'static_assert'.
    */
    export template<std::size_t Index>
    struct part {
        static constexpr auto index() -> std::size_t {
            return Index;
        }

        static consteval auto is_solved_by(const std::meta::info solver, const auto &... template_args) -> void {
            const auto storage = ^^impl::part_solver_storage<Index>;

            const auto member_type = substitute(^^impl::part_solver_storage_member, {
                std::meta::reflect_constant(solver),

                std::meta::reflect_constant(
                    std::meta::reflect_constant(template_args)
                )...
            });

            define_aggregate(storage, {
                data_member_spec(member_type, {
                    .name = "dummy_field"
                })
            });
        }

        static consteval auto has_solver() -> bool {
            return is_complete_type(^^impl::part_solver_storage<Index>);
        }

        static consteval auto solver_function(std::meta::info input_type) -> std::meta::info {
            return impl::solver_info(Index).solver_function(input_type);
        }

        static consteval auto is_printed_with(std::string_view fmt) -> void {
            const auto storage = ^^impl::part_print_string_storage<Index>;

            define_aggregate(storage, {
                data_member_spec(^^int, {
                    .name = "dummy_field",

                    .annotations = {
                        std::meta::reflect_constant(
                            std::define_static_string(fmt)
                        )
                    }
                })
            });
        }

        static consteval auto print_string() -> std::string_view {
            const auto storage = ^^impl::part_print_string_storage<Index>;

            /* No string specified, try to use a fallback. */
            if (not is_complete_type(storage)) {
                if constexpr (Index == 0) {
                    return "Part one solution: {}\t(in {:.3})";
                } else if constexpr (Index == 1) {
                    return "Part two solution: {}\t(in {:.3})";
                } else {
                    throw std::meta::exception(
                        "No print string specified and no fallback available",
                        ^^print_string
                    );
                }
            }

            const auto member = nonstatic_data_members_of(
                storage, std::meta::access_context::unchecked()
            )[0];

            return extract<const char *>(
                annotations_of(member)[0]
            );
        }

        template<typename Input = std::string_view>
        static constexpr auto operator ()(Input &&input = impl::find_example_data()) -> decltype(auto) {
            return [: solver_function(^^Input) :](std::forward<Input>(input));
        }

        /*
            Sometimes a day will require some different parameters
            between the example data and the real input data.

            For that purpose, this 'with_template_args' method is
            provided, where a different set of template arguments
            may be specified, as in the 'is_solved_by' function.
        */
        template<auto... TemplateArgs, typename Input = std::string_view>
        static constexpr auto with_template_args(Input &&input = impl::find_example_data()) -> decltype(auto) {
            static constexpr auto SolverFunction = impl::solver_info(Index).solver_function_with_template_args(
                {std::meta::reflect_constant(TemplateArgs)...},

                ^^Input
            );

            return [: SolverFunction :](std::forward<Input>(input));
        }
    };

    export constexpr inline auto part_one = advent::part<0>{};
    export constexpr inline auto part_two = advent::part<1>{};

    namespace impl {

        template<advent::part Part, typename Solution>
        constexpr auto perform_print(Solution &&solution, const auto duration) {
            static constexpr auto PrintString = Part.print_string();

            if (duration >= std::chrono::seconds(1)) {
                using seconds = std::chrono::duration<std::float64_t>;

                const auto scaled = std::chrono::duration_cast<seconds>(duration);

                advent::println(PrintString, std::forward<Solution>(solution), scaled);
            } else if (duration >= std::chrono::milliseconds(1)) {
                using milliseconds = std::chrono::duration<std::float64_t, std::milli>;

                const auto scaled = std::chrono::duration_cast<milliseconds>(duration);

                advent::println(PrintString, std::forward<Solution>(solution), scaled);
            } else {
                using microseconds = std::chrono::duration<std::float64_t, std::micro>;

                const auto scaled = std::chrono::duration_cast<microseconds>(duration);

                advent::println(PrintString, std::forward<Solution>(solution), scaled);
            }
        }

    }

    export template<advent::part Part>
    constexpr auto print_solution(const std::string &data) -> void {
        advent::timer timer;

        const auto solve = [&]() -> decltype(auto) {
            static constexpr auto SolverFunction = Part.solver_function(^^const std::string &);

            auto _ = timer.measure_scope();

            return [: SolverFunction :](data);
        };

        /* We use this lambda to make our evaluation order more explicit. */
        const auto perform_print = [&](auto &&solution) {
            impl::perform_print<Part>(std::forward<decltype(solution)>(solution), timer.last_measured_duration());
        };

        perform_print(solve());
    }

    /* 'argv' is a pointer to a const pointer to a const 'char'. */
    export constexpr auto puzzle_data(int argc, const char * const *argv) -> std::optional<std::string> {
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

        const auto pos = std::ftell(fp);
        if (pos < 0) {
            return std::nullopt;
        }

        const auto file_size = static_cast<std::size_t>(pos);

        if (std::fseek(fp, 0, SEEK_SET) < 0) {
            return std::nullopt;
        }

        std::string file_contents;
        file_contents.resize_and_overwrite(file_size, [&](char *data, const std::size_t size) {
            return std::fread(data, 1, size, fp);
        });

        if (file_contents.size() != file_size) {
            return std::nullopt;
        }

        return file_contents;
    }

    /*
        NOTE: We need a dependent name so that we don't
        evaluate our stateful metaprogramming too early.
    */
    export template<typename DependentName = int>
    constexpr auto solve_puzzles(int argc, const char * const *argv) -> int {
        const auto data = advent::puzzle_data(argc, argv);
        if (not data.has_value()) {
            advent::println("Unable to read puzzle data!");

            return 1;
        }

        /* This would plausibly be a good option for a 'cvl::expand_loop'. */
        static constexpr auto NumPartsToSolve  = [](auto) {
            auto index = 0uz;

            while (true) {
                if (not impl::part_has_solver(index)) {
                    return index;
                }

                ++index;
            }
        }(^^DependentName);

        template for (constexpr auto Index : std::views::indices(NumPartsToSolve)) {
            advent::print_solution<advent::part<Index>{}>(*data);
        }

        return 0;
    }
}
