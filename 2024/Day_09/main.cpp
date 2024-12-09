#include <advent/advent.hpp>

struct Filesystem {
    struct Blocks {
        static constexpr std::size_t FreeBlockId = -1;

        std::size_t id;
        std::size_t count;

        constexpr bool is_free_space(this const Blocks self) {
            return self.id == FreeBlockId;
        }

        constexpr bool is_used_space(this const Blocks self) {
            return !self.is_free_space();
        }

        constexpr void make_free(this Blocks &self) {
            self.id = FreeBlockId;
        }

        constexpr std::size_t checksum_contribution(this const Blocks self, const std::size_t start_position) {
            if (self.is_free_space()) {
                return 0;
            }

            /* Formula to count the sum of our ID multiplied by the position of each block. */
            return self.id * (
                start_position * self.count +

                /* NOTE: No rounding will occur because n*(n-1) is always even. */
                (self.count * (self.count - 1)) / 2
            );
        }

        constexpr bool operator ==(const Blocks &) const = default;
    };

    std::vector<Blocks> blocks;

    constexpr explicit Filesystem(const std::string_view disk_map) {
        /* We must have one used block. */
        [[assume(disk_map.size() > 1)]];

        /* The disk map must end in a newline. */
        [[assume(disk_map.back() == '\n')]];

        /* NOTE: We reserve one more than the specified blocks in case we add a dummy free block.*/
        this->blocks.reserve(disk_map.size());

        auto free_space = false;
        std::size_t id  = 0;
        for (const auto count_repr : disk_map.substr(0, disk_map.size() - 1)) {
            const auto count = advent::to_integral<std::size_t>(count_repr);

            if (!free_space) {
                this->blocks.emplace_back(id, count);

                ++id;
            } else {
                this->blocks.emplace_back(Blocks::FreeBlockId, count);
            }

            free_space = !free_space;
        }

        [[assume(this->blocks.size() > 0 && this->blocks[0].is_used_space())]];

        if (free_space) {
            /*
                Last block was used space, so add a free block at the end to help us later.

                NOTE: For some reason this is faster for atomic compressing as well.
            */

            this->blocks.emplace_back(Blocks::FreeBlockId, 0uz);
        }
    }

    constexpr bool _fragmented_compress_last(this Filesystem &self) {
        /* NOTE: We could maybe be slightly more efficient searching for the last used block. */
        const auto last_used_it = std::ranges::find_last_if(self.blocks, &Blocks::is_used_space).begin();

        [[assume(last_used_it->count > 0)]];

        /* NOTE: Our first set of blocks are always used. */
        auto search_start = self.blocks.begin() + 1;
        while (true) {
            const auto first_free_it = std::ranges::find_if(search_start, last_used_it, &Blocks::is_free_space);

            if (first_free_it >= last_used_it) {
                /* There are no previous free blocks, so we cannot compress further. */

                return false;
            }

            /* NOTE: We could maybe be a little clever about the block shuffling here, but meh. */

            if (first_free_it->count > last_used_it->count) {
                /* We have a surplus of free space. */

                const auto surplus_space = first_free_it->count - last_used_it->count;

                *first_free_it = std::move(*last_used_it);

                last_used_it->make_free();

                self.blocks.emplace(first_free_it + 1, Blocks::FreeBlockId, surplus_space);

                return true;
            }

            first_free_it->id = last_used_it->id;

            last_used_it->count -= first_free_it->count;

            /*
                NOTE: Our last set of blocks is always free.

                We also technically don't need to do this, but
                conceptually I would like to preserve the count
                of all the blocks in the filesystem.
            */
            self.blocks.back().count  += first_free_it->count;

            if (last_used_it->count <= 0) {
                /* We had just enough free space. */

                /* Instead of deleting the blocks, just mark them as free. */
                last_used_it->make_free();

                return true;
            }

            /* We had too little free space, repeat process. */
            search_start = first_free_it + 1;
        }
    }

    constexpr void fragmented_compress(this Filesystem &self) {
        while (true) {
            if (!self._fragmented_compress_last()) {
                return;
            }
        }
    }

    constexpr void atomic_compress(this Filesystem &self) {
        auto it = self.blocks.end() - 1;
        while (true) {
            if (it == self.blocks.begin()) {
                return;
            }

            if (it->is_free_space()) {
                --it;

                continue;
            }

            auto free_space_it = std::ranges::find_if(self.blocks.begin(), it, [&](const auto blocks) {
                return blocks.is_free_space() && blocks.count >= it->count;
            });

            if (free_space_it == it) {
                /* We found no appropriate free space. */

                --it;

                continue;
            }

            if (free_space_it->count == it->count) {
                /* We have just enough free space. */

                std::ranges::swap(free_space_it->id, it->id);

                --it;

                continue;
            }

            /* We have a surplus of free space. */

            const auto surplus_space = free_space_it->count - it->count;

            *free_space_it = *it;

            it->make_free();

            const auto iter_distance = it - free_space_it;
            free_space_it = self.blocks.emplace(free_space_it + 1, Blocks::FreeBlockId, surplus_space);

            /* Reconstruct the iterator due to possible invalidation. */
            it = free_space_it + iter_distance;

            --it;

            continue;
        }
    }

    constexpr std::size_t checksum(this const Filesystem &self) {
        std::size_t checksum = 0;

        std::size_t position = 0;
        for (const auto &blocks : self.blocks) {
            checksum += blocks.checksum_contribution(position);

            position += blocks.count;
        }

        return checksum;
    }
};

static_assert(
    Filesystem("12345\n").blocks == std::vector<Filesystem::Blocks>{
        {0, 1},

        {Filesystem::Blocks::FreeBlockId, 2},

        {1, 3},

        {Filesystem::Blocks::FreeBlockId, 4},

        {2, 5},

        {Filesystem::Blocks::FreeBlockId, 0}
    }
);

constexpr std::size_t checksum_of_fragmented_compressed_filesystem(const std::string_view disk_map) {
    auto filesystem = Filesystem(disk_map);

    filesystem.fragmented_compress();

    return filesystem.checksum();
}

constexpr std::size_t checksum_of_atomic_compressed_filesystem(const std::string_view disk_map) {
    auto filesystem = Filesystem(disk_map);

    filesystem.atomic_compress();

    return filesystem.checksum();
}

constexpr inline std::string_view example_data = (
    "2333133121414131402\n"
);

static_assert(checksum_of_fragmented_compressed_filesystem(example_data) == 1928);
static_assert(checksum_of_atomic_compressed_filesystem(example_data) == 2858);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        checksum_of_fragmented_compressed_filesystem,
        checksum_of_atomic_compressed_filesystem
    );
}
