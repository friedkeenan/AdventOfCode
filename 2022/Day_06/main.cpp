#include <advent/advent.hpp>

constexpr std::size_t find_duplicate_character(const std::string_view potential_marker) {
    for (const auto i : std::views::iota(0uz, potential_marker.length())) {
        const auto found_pos = potential_marker.find_first_of(potential_marker[i], i + 1);

        /* If we find a duplicate character. */
        if (found_pos != std::string_view::npos) {
            return i;
        }
    }

    /* If we find no duplicate characters, return 'npos'. */
    return std::string_view::npos;
}

constexpr std::size_t find_end_of_marker(const std::size_t marker_length, const std::string_view stream) {
    /* NOTE: We could maybe do this differently if I had an implementation of 'std::views::slide'. */

    advent::assume(stream.length() >= marker_length);

    /* This variable gets reused. */
    auto potential_marker = std::string_view(stream.data(), marker_length);
    while (true) {
        const auto duplicate_pos = find_duplicate_character(potential_marker);
        if (duplicate_pos == std::string_view::npos) {
            return potential_marker.data() - stream.data() + marker_length;
        }

        /* Slide 'potential_marker' over past the duplicate character. */
        potential_marker = std::string_view(potential_marker.data() + duplicate_pos + 1, marker_length);
    }
}

constexpr std::size_t find_start_of_packet(const std::string_view stream) {
    return find_end_of_marker(4, stream);
}

constexpr std::size_t find_start_of_message(const std::string_view stream) {
    return find_end_of_marker(14, stream);
}

static_assert(find_start_of_packet("mjqjpqmgbljsphdztnvjfqwrcgsmlb\n")    == 7);
static_assert(find_start_of_packet("bvwbjplbgvbhsrlpgdmjqwftvncz\n")      == 5);
static_assert(find_start_of_packet("nppdvjthqldpwncqszvftbrmjlhg\n")      == 6);
static_assert(find_start_of_packet("nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg\n") == 10);
static_assert(find_start_of_packet("zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw\n")  == 11);

static_assert(find_start_of_message("mjqjpqmgbljsphdztnvjfqwrcgsmlb\n")    == 19);
static_assert(find_start_of_message("bvwbjplbgvbhsrlpgdmjqwftvncz\n")      == 23);
static_assert(find_start_of_message("nppdvjthqldpwncqszvftbrmjlhg\n")      == 23);
static_assert(find_start_of_message("nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg\n") == 29);
static_assert(find_start_of_message("zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw\n")  == 26);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        find_start_of_packet,
        find_start_of_message
    );
}
