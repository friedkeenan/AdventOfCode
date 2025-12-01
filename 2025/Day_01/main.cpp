import std;
import advent;

struct Dial {
    static constexpr advent::ssize_t Length          = 100;
    static constexpr advent::ssize_t InitialRotation = 50;

    struct Rotation {
        advent::ssize_t amount;

        constexpr Rotation(std::string_view description) {
            [[assume(description.size() > 1)]];

            if (description.front() == 'R') {
                this->amount = 1;
            } else {
                this->amount = -1;
            }

            description.remove_prefix(1);

            this->amount *= advent::to_integral<advent::ssize_t>(description);
        }

        constexpr advent::ssize_t total_amount(this const Rotation self) {
            return self.amount;
        }

        constexpr auto reduce(this Rotation self) {
            struct ReducedRotation {
                advent::ssize_t complete_turns;
                Rotation remainder;
            };

            const auto complete_turns = advent::abs(self.total_amount()) / Length;

            /* Reuse 'self' because I don't feel like adding another constructor. */
            self.amount = self.total_amount() % Length;

            return ReducedRotation{
                .complete_turns = complete_turns,
                .remainder      = self
            };
        }
    };

    advent::ssize_t net_rotation = InitialRotation;

    constexpr std::size_t accumulate_password(this auto &self, const Rotation rotation) {
        return self.accumulate_password_impl(rotation);
    }
};

struct RestAtZeroDial : Dial {
    constexpr std::size_t accumulate_password_impl(this RestAtZeroDial &self, const Rotation rotation) {
        self.net_rotation += rotation.total_amount();
        self.net_rotation %= Length;

        if (self.net_rotation == 0) {
            return 1;
        }

        return 0;
    }
};

struct PassesZeroDial : Dial {
    constexpr std::size_t accumulate_password_impl(this PassesZeroDial &self, const Rotation rotation) {
        const auto reduced = rotation.reduce();

        auto num_zero_passes = reduced.complete_turns;

        const bool is_at_zero = self.net_rotation == 0;

        self.net_rotation += reduced.remainder.total_amount();

        if (!is_at_zero && (self.net_rotation <= 0 || self.net_rotation >= Length)) {
            num_zero_passes += 1;
        }

        /* Keeping the rotation positive is important for our calculations. */
        self.net_rotation = advent::modulo(self.net_rotation, Length);

        return num_zero_passes;
    }
};

template<std::derived_from<Dial> DialImpl, advent::string_viewable_range Rng>
constexpr std::size_t derive_password(Rng &&rng) {
    auto dial = DialImpl{};

    std::size_t password = 0;
    for (const std::string_view line : std::forward<Rng>(rng)) {
        if (line.empty()) {
            continue;
        }

        const auto rotation = Dial::Rotation(line);

        password += dial.accumulate_password(rotation);
    }

    return password;
}

template<std::derived_from<Dial> DialImpl>
constexpr std::size_t derive_password_from_string_data(const std::string_view data) {
    return derive_password<DialImpl>(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "L68\n"
    "L30\n"
    "R48\n"
    "L5\n"
    "R60\n"
    "L55\n"
    "L1\n"
    "L99\n"
    "R14\n"
    "L82\n"
);

static_assert(derive_password_from_string_data<RestAtZeroDial>(example_data) == 3);
static_assert(derive_password_from_string_data<PassesZeroDial>(example_data) == 6);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        derive_password_from_string_data<RestAtZeroDial>,
        derive_password_from_string_data<PassesZeroDial>
    );
}
