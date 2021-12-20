#pragma once

#define ADVENT_NON_COPYABLE(cls)          \
    cls(const cls &) = delete;            \
    cls &operator =(const cls &) = delete

#define ADVENT_NON_MOVEABLE(cls)     \
    cls(cls &&) = delete;            \
    cls &operator =(cls &&) = delete

#define ADVENT_RIGHT_UNARY_OP_FROM_LEFT(cls, op, ...) \
    cls operator op(int) __VA_ARGS__ {                \
        const cls ret = *this;                        \
        op(*this);                                    \
        return ret;                                   \
    }
