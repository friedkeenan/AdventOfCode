#include <advent/advent.hpp>

constexpr std::size_t find_end_of_marker(const std::size_t marker_length, const std::string_view stream) {
    advent::assume(stream.length() >= marker_length);

    /* This variable gets reused. */
    auto potential_marker = std::string_view(stream.data(), marker_length);
    while (true) {
        bool found_duplicate = false;

        for (const auto i : std::views::iota(0uz, marker_length)) {
            const auto found_pos = potential_marker.find_last_of(potential_marker[i]);

            /* If the character we found is not the one we are currently checking, i.e. it is a duplicate. */
            if (found_pos > i) {
                /* Slide 'potential_marker' over past the duplicate character. */
                potential_marker = std::string_view(potential_marker.data() + i + 1, marker_length);

                found_duplicate = true;
                break;
            }
        }

        if (!found_duplicate) {
            return potential_marker.data() - stream.data() + marker_length;
        }
    }
}

constexpr std::size_t find_start_of_packet(const std::string_view stream) {
    return find_end_of_marker(4, stream);
}

constexpr std::size_t find_start_of_message(const std::string_view stream) {
    return find_end_of_marker(14, stream);
}

static_assert(find_start_of_packet("mjqjpqmgbljsphdztnvjfqwrcgsmlb")    == 7);
static_assert(find_start_of_packet("bvwbjplbgvbhsrlpgdmjqwftvncz")      == 5);
static_assert(find_start_of_packet("nppdvjthqldpwncqszvftbrmjlhg")      == 6);
static_assert(find_start_of_packet("nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg") == 10);
static_assert(find_start_of_packet("zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw")  == 11);

static_assert(find_start_of_message("mjqjpqmgbljsphdztnvjfqwrcgsmlb")    == 19);
static_assert(find_start_of_message("bvwbjplbgvbhsrlpgdmjqwftvncz")      == 23);
static_assert(find_start_of_message("nppdvjthqldpwncqszvftbrmjlhg")      == 23);
static_assert(find_start_of_message("nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg") == 29);
static_assert(find_start_of_message("zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw")  == 26);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        find_start_of_packet,
        find_start_of_message
    );
}
