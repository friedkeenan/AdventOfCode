#pragma once

#include <advent/concepts.hpp>

namespace advent {

    template<advent::inheritable... Callables>
    struct overloaded : Callables... {
        /* Constructor and deduction guide are implicitly defined. */

        /* TODO: Maybe find a way we can accurately reflect this in our requirements. */
        using Callables::operator ()...;
    };

}
