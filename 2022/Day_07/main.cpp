#include <advent/advent.hpp>

class FileSystem {
    public:
        static constexpr std::size_t InvalidSize = -1;

        static constexpr std::size_t TotalSpace  = 70'000'000;
        static constexpr std::size_t NeededSpace = 30'000'000;

        class File {
            public:
                std::size_t size;
        };

        class Directory {
            public:
                Directory &parent;

                std::vector<Directory> _directories;

                std::size_t size = 0;

                /* Default construction creates a root directory. */
                constexpr Directory() : parent(*this) { }

                /* Take in a pointer to not thrash with copy constructor. */
                constexpr Directory(Directory *parent) : parent(*parent) { }

                constexpr bool is_root() const {
                    return &this->parent == this;
                }

                constexpr Directory &add_directory(/* const std::string_view name */) {
                    return this->_directories.emplace_back(this);
                }

                constexpr void add_file_with_size(const std::size_t file_size) {
                    this->size += file_size;

                    if (!this->is_root()) {
                        /* Update our parent's sizes. */
                        this->parent.add_file_with_size(file_size);
                    }
                }
        };

        Directory root;

        template<typename Consumer>
        requires (std::invocable<const Consumer &, const Directory &>)
        static constexpr void _for_each_directory(const Consumer &consumer, const Directory &directory) {
            for (const auto &child : directory._directories) {
                _for_each_directory(consumer, child);
            }

            std::invoke(consumer, directory);
        }

        template<typename Consumer>
        requires (std::invocable<const Consumer &, const Directory &>)
        constexpr void for_each_directory(const Consumer consumer) const {
            return _for_each_directory(consumer, this->root);
        }
};

class Terminal {
    public:
        static constexpr std::string_view LsCommand = "$ ls";

        /* We add a space at the end for parsing ease. */
        static constexpr std::string_view CdCommand = "$ cd ";

        static constexpr std::string_view DirPrefix = "dir ";

        FileSystem fs;

        /*
            NOTE: The first line will be "$ cd /" so this nullptr is not an issue.

            Additionally, this pointer will not be invalidated while it's
            being used, as when adding a directory or file to the current
            directory, the parent of the current directory is not changing.
        */
        FileSystem::Directory *current_directory = nullptr;

        constexpr void change_directory(const std::string_view new_directory) {
            /*
                Theoretically we know what directory we should be changing to
                without needing to parse it, it acts sorta-ish like a stack,
                definitely calculable...

                But I don't want to do that, probably wouldn't be better in any way.
            */

            if (new_directory == "/") {
                this->current_directory = &this->fs.root;

                return;
            }

            if (new_directory == "..") {
                this->current_directory = &this->current_directory->parent;

                return;
            }

            /* We only 'cd' into a directory once, so we can just add a nameless directory. */
            this->current_directory = &this->current_directory->add_directory();
        }

        constexpr void consume_line(const std::string_view line) {
            /* Thank you Eric Wastl for only having each directory entered and listed once. */

            if (line == LsCommand) {
                /* We don't need to handle 'ls'. */
                return;
            }

            if (line.starts_with(DirPrefix)) {
                /* We don't need to handle 'dir'. */

                return;
            }

            if (line.starts_with(CdCommand)) {
                const auto new_directory = line.substr(CdCommand.length());
                this->change_directory(new_directory);

                return;
            }

            /* We have a file. */
            const auto end_of_num = line.find_first_of(' ');
            [[assume(end_of_num != std::string_view::npos)]];

            const auto file_size = advent::to_integral<std::size_t>(line.substr(0, end_of_num));
            this->current_directory->add_file_with_size(file_size);
        }
};

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t sum_of_directory_sizes_with_limit(const std::size_t limit, Rng &&terminal_lines) {
    Terminal terminal;

    for (const std::string_view line : std::forward<Rng>(terminal_lines)) {
        if (line.empty()) {
            continue;
        }

        terminal.consume_line(line);
    }

    std::size_t limited_size = 0;
    terminal.fs.for_each_directory([&](const FileSystem::Directory &directory) {
        if (directory.size > limit) {
            return;
        }

        limited_size += directory.size;
    });

    return limited_size;
}

template<std::ranges::input_range Rng>
requires (std::convertible_to<std::ranges::range_reference_t<Rng>, std::string_view>)
constexpr std::size_t find_directory_to_delete(Rng &&terminal_lines) {
    Terminal terminal;

    for (const std::string_view line : std::forward<Rng>(terminal_lines)) {
        if (line.empty()) {
            continue;
        }

        terminal.consume_line(line);
    }

    const auto free_space = FileSystem::TotalSpace - terminal.fs.root.size;

    std::size_t minimum_sufficient_space = -1;
    terminal.fs.for_each_directory([&](const FileSystem::Directory &directory) {
        if (directory.size + free_space >= FileSystem::NeededSpace && directory.size < minimum_sufficient_space) {
            minimum_sufficient_space = directory.size;
        }
    });

    return minimum_sufficient_space;
}

constexpr std::size_t sum_of_directory_sizes_with_limit_from_string_data(const std::string_view data) {
    return sum_of_directory_sizes_with_limit(10'0000, data | advent::views::split_lines);
}

constexpr std::size_t find_directory_to_delete_from_string_data(const std::string_view data) {
    return find_directory_to_delete(data | advent::views::split_lines);
}

constexpr inline std::string_view example_data = (
    "$ cd /\n"
    "$ ls\n"
    "dir a\n"
    "14848514 b.txt\n"
    "8504156 c.dat\n"
    "dir d\n"
    "$ cd a\n"
    "$ ls\n"
    "dir e\n"
    "29116 f\n"
    "2557 g\n"
    "62596 h.lst\n"
    "$ cd e\n"
    "$ ls\n"
    "584 i\n"
    "$ cd ..\n"
    "$ cd ..\n"
    "$ cd d\n"
    "$ ls\n"
    "4060174 j\n"
    "8033020 d.log\n"
    "5626152 d.ext\n"
    "7214296 k\n"
);

static_assert(sum_of_directory_sizes_with_limit_from_string_data(example_data) == 95437);
static_assert(find_directory_to_delete_from_string_data(example_data) == 24933642);

int main(int argc, char **argv) {
    return advent::solve_puzzles(
        argc, argv,

        sum_of_directory_sizes_with_limit_from_string_data,
        find_directory_to_delete_from_string_data
    );
}
