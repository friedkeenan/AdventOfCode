#pragma once

#include <advent/concepts.hpp>

namespace advent {

    template<typename T> requires advent::addable_with<T, T>
    using addition_result = decltype(std::declval<T>() + std::declval<T>());

}
