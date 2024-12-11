project('AdventOfCode', 'cpp',
    default_options : [
        # -O3
        'buildtype=release',

        # Notably, this will prevent STL assertions as well.
        'b_ndebug=if-release',
    ],
)

fs = import('fs')

dependencies = [
    dependency('fmt'),
]

sanitizers = [
    # '-fsanitize=address',
    # '-fsanitize=undefined',
]

link_args = [
    '-s',
]

cpp_args  = [
    '-std=gnu++2c',

    '-Wall',
    '-Wextra',
    '-Wpedantic',
    '-Wconversion',
    '-Werror',

    '-fconcepts-diagnostics-depth=3',
    '-fconstexpr-ops-limit=9999999999999999',

    # Flag to enable timing solutions.
    #
    # Using chrono slows down compile times a lot.
    #
    # TODO: Remove when we have modules.
    '-DADVENT_TIME_SOLUTIONS',
]

link_args += sanitizers
cpp_args  += sanitizers

advent_years = [
    '2021',
    '2022',
    '2023',
    '2024',
]

foreach year : advent_years

    subdir(year)

    foreach day : days

    # It would be nice to use a precompiled header,
    # but it is not supported to use the same PCH
    # for multiple targets.
    executable('@0@_@1@'.format(year, day), year / day / 'main.cpp',
        include_directories : 'include',

        dependencies : dependencies,

        link_args : link_args,
        cpp_args  : cpp_args,
    )

    endforeach

endforeach
