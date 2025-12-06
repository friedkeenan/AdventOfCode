import std;
import advent;

struct BisectedInvalidID {
    std::size_t half_id;

    static constexpr BisectedInvalidID from_last_id(const std::size_t last_id) {
        const auto num_digits = advent::count_digits(last_id, 10uz);

        const auto halfway_raised_base = advent::pow(10uz, num_digits / 2);

        if (num_digits % 2 == 0) {
            /* NOTE: This branch saves a lot of time, but is not necessary. */
            return BisectedInvalidID{last_id / halfway_raised_base};
        }

        /*
            For ranges that ends with a number with
            an odd number of digits, the largest possible
            invalid ID is the longest string of 9's which
            does not surpass the last ID.

            For instance:

            95-115 -> 99 is the first invalid ID.

            9995-11111 -> 9999 is the first invalid ID.

            And so on.

            We store the halved ID, so for 99 we store 9; 9999 we store 99, etc.
        */
        return BisectedInvalidID{
            halfway_raised_base - 1
        };
    }

    constexpr void reduce(this BisectedInvalidID &self) {
        self.half_id -= 1;
    }

    constexpr std::size_t value(this const BisectedInvalidID self) {
        return advent::concat_digits{}(self.half_id, self.half_id);
    }
};

struct ProductIDsRange {
    static constexpr char Separator = '-';

    std::size_t first;
    std::size_t last;

    constexpr explicit ProductIDsRange(const std::string_view description) {
        const auto separator_pos = description.find_first_of(Separator);
        [[assume(separator_pos != std::string_view::npos)]];

        const auto first_description = description.substr(0, separator_pos);
        const auto last_description  = description.substr(separator_pos + 1);

        this->first = advent::to_integral<std::size_t>(first_description);
        this->last  = advent::to_integral<std::size_t>(last_description);
    }

    constexpr bool contains(this const ProductIDsRange self, const std::size_t id) {
        return id >= self.first && id <= self.last;
    }

    constexpr std::size_t sum_bisected_invalid_ids(this const ProductIDsRange self) {
        auto invalid_id = BisectedInvalidID::from_last_id(self.last);

        std::size_t sum = 0;
        while (true) {
            const auto value = invalid_id.value();

            if (value < self.first) {
                return sum;
            }

            if (self.contains(value)) {
                sum += value;
            }

            invalid_id.reduce();
        }
    }

    constexpr std::size_t sum_extended_invalid_ids(this const ProductIDsRange self) {
        /* The code for this sucks, I'm very sorry. */

        auto [num_first_digits, target_raised_base] = advent::count_digits_and_raise_base(self.first, 10uz);

        const auto digit_counts = std::views::iota(
            num_first_digits,
            advent::count_digits(self.last, 10uz) + 1
        );

        /*
            I couldn't think of a way to
            just avoid invalid IDs we've
            seen, so we track them here.

            We have to keep it outside of the loop
            to make up for some shoddy logic inside.
        */
        std::vector<std::size_t> seen_invalid_ids;

        std::size_t sum = 0;
        for (const auto _ : digit_counts) {
            std::size_t prev_raised_base    = 1;
            std::size_t current_raised_base = 10;

            while (current_raised_base * current_raised_base <= target_raised_base) {
                for (const auto sequence : std::views::iota(prev_raised_base, current_raised_base)) {
                    std::size_t concatenated = 0;
                    while (concatenated < target_raised_base) {
                        concatenated += sequence;
                        concatenated *= current_raised_base;
                    }

                    /* Undo the last concatenation that went over. */
                    concatenated /= current_raised_base;

                    if (concatenated > self.last) {
                        break;
                    }

                    if (!self.contains(concatenated)) {
                        continue;
                    }

                    if (std::ranges::contains(seen_invalid_ids, concatenated)) {
                        continue;
                    }

                    sum += concatenated;
                    seen_invalid_ids.push_back(concatenated);
                }

                prev_raised_base = current_raised_base;
                current_raised_base *= 10uz;
            }

            target_raised_base *= 10uz;
        }

        return sum;
    }
};

template<auto SumMethod>
requires (
    std::invocable<decltype(SumMethod), const ProductIDsRange &> &&

    std::same_as<
        std::invoke_result_t<decltype(SumMethod), const ProductIDsRange &>,
        std::size_t
    >
)
constexpr std::size_t sum_invalid_ids(const std::string_view ranges) {
    std::size_t sum = 0;

    advent::split_for_each(ranges, ',', [&](auto range_description) {
        if (range_description.ends_with('\n')) {
            range_description.remove_suffix(1);
        }

        const auto range = ProductIDsRange(range_description);

        sum += std::invoke(SumMethod, range);
    });

    return sum;
}

constexpr std::size_t sum_bisected_invalid_ids(const std::string_view data) {
    return sum_invalid_ids<&ProductIDsRange::sum_bisected_invalid_ids>(data);
}

constexpr std::size_t sum_extended_invalid_ids(const std::string_view data) {
    return sum_invalid_ids<&ProductIDsRange::sum_extended_invalid_ids>(data);
}

constexpr inline std::string_view example_data = (
    "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,"
    "1698522-1698528,446443-446449,38593856-38593862,565653-565659,"
    "824824821-824824827,2121212118-2121212124\n"
);

static_assert(sum_bisected_invalid_ids(example_data) == 1227775554);
static_assert(sum_extended_invalid_ids(example_data) == 4174379265);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_bisected_invalid_ids,
        sum_extended_invalid_ids
    );
}
