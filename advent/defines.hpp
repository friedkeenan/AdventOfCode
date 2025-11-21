#pragma once

#define ADVENT_RIGHT_UNARY_OP_FROM_LEFT(cls, op, ...) \
    cls operator op(int) __VA_ARGS__ {                \
        const cls ret = *this;                        \
        op(*this);                                    \
        return ret;                                   \
    }
