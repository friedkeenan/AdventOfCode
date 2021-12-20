project('AdventOfCode', 'cpp',
    default_options : 'buildtype=release', # -O3
)

fs = import('fs')

dependencies = [
    dependency('fmt'),
]

sanitizers = [
    # '-fsanitize=address',
    # '-fsanitize=undefined',
]

link_args = ['-s']

cpp_args  = [
    '-std=gnu++2b',

    '-Wall',
    '-Wextra',
    '-Wpedantic',
    '-Wconversion',
    '-Werror',

    '-fconcepts-diagnostics-depth=3',
]

link_args += sanitizers
cpp_args  += sanitizers

advent_years = ['2021']

foreach year : advent_years

    foreach day : range(1, 26)

    if day < 10

        day = '0' + day.to_string()

    else
        day = day.to_string()

    endif

    # TODO: If we need more complex structures than this, make that work.
    path = '@0@/Day_@1@/main.cpp'.format(year, day)

    if not fs.exists(path)

        break

    endif

    # It would be nice to use a precompiled header,
    # but it is not supported to use the same PCH
    # for multiple targets.
    executable('@0@_Day_@1@'.format(year, day), path,
        include_directories : 'include',

        dependencies : dependencies,

        link_args : link_args,
        cpp_args  : cpp_args,
    )

    endforeach

endforeach
