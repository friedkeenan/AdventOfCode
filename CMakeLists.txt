cmake_minimum_required(VERSION 4.1)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

project(AdventOfCode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD         26)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD       ON)
set(CMAKE_COLOR_DIAGNOSTICS    ON)

set(SANITIZERS
    # -fsanitize=address
    # -fsanitize=undefined
)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Werror

    -fconcepts-diagnostics-depth=3
    -fconstexpr-ops-limit=9999999999999999

    ${SANITIZERS}
)

add_link_options(
    -s

    ${SANITIZERS}
)

add_subdirectory(advent)

set(ADVENT_YEARS
    2021
    2022
    2023
    2024
    2025
)

function (add_day year day)

    set(EXE ${year}_${day})

    add_executable(${EXE}
        ${year}/${day}/main.cpp
    )

    target_link_libraries(${EXE} advent)

endfunction ()

list(GET ADVENT_YEARS -1 LATEST_YEAR)

foreach (year IN LISTS ADVENT_YEARS)
    add_subdirectory(${year})

    # The last year will store its last day in the 'LATEST_DAY' variable.
    list(GET ADVENT_DAYS -1 LATEST_DAY)

    foreach (day IN LISTS ADVENT_DAYS)

        add_day(${year} ${day})

    endforeach ()
endforeach ()

add_custom_target(latest
    DEPENDS ${LATEST_YEAR}_${LATEST_DAY}
)
