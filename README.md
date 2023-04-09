# Advent of Code Solutions

This is a repository of my [Advent of Code](https://adventofcode.com/) solutions. They are all in C++, using the latest features provided to me by GCC, and my goal with them is to write code that I would be happy persisting in a genuine project indefinitely. My goal is decidedly *not* to find solutions with the least amount of code or come up with a solution as fast as possible. I want the resulting code to be pleasant to both read and write.

My solutions all work in `constexpr`, and in fact the example data provided by Advent of Code is used to static assert that my solutions are correct. For instance, here's an example of that for [day one of 2021](https://adventofcode.com/2021/day/1):

```cpp
constexpr inline std::string_view example_data = (
    "199\n"
    "200\n"
    "208\n"
    "210\n"
    "200\n"
    "207\n"
    "240\n"
    "269\n"
    "260\n"
    "263\n"
);

static_assert(count_depth_increases_from_string_data(example_data) == 7);

static_assert(count_chunked_depth_increases_from_string_data(example_data) == 5);
```

Being able to leverage constant evaluation is a great boon to my solutions, as constant evaluation will catch many errors at compile time, ensuring that my code is safe and functioning properly. I think this is a really good use case for such a thing: to use constant evaluation for "unit tests" to ensure that at runtime the code will work properly, with the added benefit of enhanced debugging.

In pursuit of this goal of `constexpr` capability, I have written a [small library](https://github.com/friedkeenan/AdventOfCode/tree/main/include/advent) that gives me many `constexpr` facilities that are not included in the STL, such as parsing strings to integers, exponentiation, and certain other utilities. This library is shared by all my solutions.

Additionally, I will sometimes continue to update solutions even after I have solved the problem. Sometimes it's because I discovered/thought of a better way to solve the problem, sometimes it's simply to improve the style/readability of the code, and other times it's simple maintenance.
