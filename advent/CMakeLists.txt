add_library(advent)

target_sources(advent
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ..
    FILES
    defines.hpp

    PUBLIC FILE_SET CXX_MODULES FILES

    advent.cpp
    concepts.cpp
    scope_guard.cpp
    type_aliases.cpp
    math.cpp
    digits.cpp
    split_string_view.cpp
    vector_nd.cpp
    regular_vector.cpp
    grid.cpp
    functional.cpp
    type_traits.cpp
    print.cpp
    timer.cpp
    puzzle_data.cpp
)
