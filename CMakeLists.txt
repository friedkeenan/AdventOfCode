cmake_minimum_required(VERSION 4.1)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

project(AdventOfCode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD      26)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(SANITIZERS
    # -fsanitize=address
    # -fsanitize=undefined
)

set(COMPILE_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Werror

    -fconcepts-diagnostics-depth=3
    -fconstexpr-ops-limit=9999999999999999

    ${SANITIZERS}
)

set(LINK_FLAGS
    -s

    ${SANITIZERS}
)

add_library(advent INTERFACE)

target_include_directories(advent
    INTERFACE
    include
)

target_compile_definitions(advent
    INTERFACE
    ADVENT_TIME_SOLUTIONS
)

function (add_day year day)

    set(EXE ${year}_${day})

    add_executable(${EXE}
        ${year}/${day}/main.cpp
    )

    target_link_libraries(${EXE} advent)

    target_compile_options(${EXE}
        PRIVATE
        ${COMPILE_FLAGS}
    )

    target_link_options(${EXE}
        PRIVATE
        ${LINK_FLAGS}
    )

endfunction ()

set(ADVENT_YEARS
    2021
    2022
    2023
    2024
)

foreach (year IN LISTS ADVENT_YEARS)
    add_subdirectory(${year})

    foreach (day IN LISTS ADVENT_DAYS)

        add_day(${year} ${day})

    endforeach ()
endforeach ()
