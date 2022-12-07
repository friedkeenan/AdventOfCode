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
                class SizeCache {
                    public:
                        std::size_t _limit;
                        std::optional<std::size_t> _storage;

                        constexpr void clear() {
                            this->_storage = std::nullopt;
                        }

                        constexpr void store(const std::size_t limit, const std::size_t size) {
                            this->_limit   = limit;
                            this->_storage = size;
                        }

                        constexpr bool is_cached_for_limit(const std::size_t limit) const {
                            return this->_storage.has_value() && this->_limit == limit;
                        }

                        constexpr std::size_t size() const {
                            return *this->_storage;
                        }
                };

                Directory &parent;

                std::string_view _name;

                std::vector<Directory> _directories;

                std::size_t _total_file_size = 0;

                SizeCache _size_cache;

                /* Default construction creates a root directory. */
                constexpr Directory() : parent(*this) { }

                constexpr Directory(Directory &parent, const std::string_view name) : parent(parent), _name(name) { }

                constexpr bool is_root() const {
                    return &this->parent == this;
                }

                /* NOTE: If we had an implementation of explicit object parameters, we could do this better. */
                template<typename Self>
                static constexpr auto &_directory_with_name(Self &self, const std::string_view name) {
                    for (auto &child : self._directories) {
                        if (child._name == name) {
                            return child;
                        }
                    }

                    /* There must be a child with the requested name. */
                    std::unreachable();
                }

                constexpr Directory &directory_with_name(const std::string_view name) {
                    return _directory_with_name(*this, name);
                }

                constexpr const Directory &directory_with_name(const std::string_view name) const {
                    return _directory_with_name(*this, name);
                }

                constexpr void add_directory(const std::string_view name) {
                    this->_directories.emplace_back(*this, name);
                }

                constexpr void add_file_with_size(const std::size_t file_size) {
                    this->_total_file_size += file_size;
                }

                /* NOTE: This cannot be const because it calls 'cached_size_with_limit' on child directories. */
                constexpr std::size_t uncached_size_with_limit(const std::size_t limit) {
                    std::size_t size = 0;

                    for (auto &child : this->_directories) {
                        const auto child_size = child.cached_size_with_limit(limit);
                        if (child_size == InvalidSize) {
                            return InvalidSize;
                        }

                        size += child_size;
                        if (size > limit) {
                            return InvalidSize;
                        }
                    }

                    size += this->_total_file_size;
                    if (size > limit) {
                        return InvalidSize;
                    }

                    return size;
                }

                /* NOTE: This cannot be const because it stores the cache. */
                constexpr std::size_t cached_size_with_limit(const std::size_t limit) {
                    if (this->_size_cache.is_cached_for_limit(limit)) {
                        return this->_size_cache.size();
                    }

                    const auto size = this->uncached_size_with_limit(limit);

                    this->_size_cache.store(limit, size);

                    return size;
                }

                /* NOTE: This cannot be const because it calls 'cached_size' on child directories. */
                constexpr std::size_t uncached_size() {
                    std::size_t size = 0;

                    for (auto &child : this->_directories) {
                        const auto child_size = child.cached_size();

                        size += child_size;
                    }

                    size += this->_total_file_size;

                    return size;
                }

                /* NOTE: This cannot be const because it stores the cache. */
                constexpr std::size_t cached_size() {
                    if (this->_size_cache.is_cached_for_limit(InvalidSize)) {
                        return this->_size_cache.size();
                    }

                    const auto size = this->uncached_size();

                    /* The total size with no limit is the same as having a limit of 'InvalidSize'. */
                    this->_size_cache.store(InvalidSize, size);

                    return size;
                }
        };

        Directory root;

        template<typename Consumer>
        requires (std::invocable<const Consumer &, Directory &>)
        static constexpr void _for_each_directory(const Consumer &consumer, Directory &directory) {
            for (auto &child : directory._directories) {
                _for_each_directory(consumer, child);
            }

            std::invoke(consumer, directory);
        }

        template<typename Consumer>
        requires (std::invocable<const Consumer &, Directory &>)
        constexpr void for_each_directory(const Consumer consumer) {
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
            if (new_directory == "/") {
                this->current_directory = &this->fs.root;

                return;
            }

            if (new_directory == "..") {
                this->current_directory = &this->current_directory->parent;

                return;
            }

            this->current_directory = &this->current_directory->directory_with_name(new_directory);
        }

        constexpr void consume_line(const std::string_view line) {
            /* Thank you Eric Wastl for only having each directory entered and listed once. */

            if (line == LsCommand) {
                /* We don't need to handle 'ls'. */
                return;
            }

            if (line.starts_with(CdCommand)) {
                const auto new_directory = line.substr(CdCommand.length());
                this->change_directory(new_directory);

                return;
            }

            if (line.starts_with(DirPrefix)) {
                const auto child_directory_name = line.substr(DirPrefix.length());
                this->current_directory->add_directory(child_directory_name);

                return;
            }

            /* We have a file. */
            const auto end_of_num = line.find_first_of(' ');
            advent::assume(end_of_num != std::string_view::npos);

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
    terminal.fs.for_each_directory([&](FileSystem::Directory &directory) {
        const auto size = directory.cached_size_with_limit(limit);
        if (size == FileSystem::InvalidSize) {
            return;
        }

        limited_size += size;
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

    const auto free_space = FileSystem::TotalSpace - terminal.fs.root.cached_size();

    std::size_t minimum_sufficient_space = -1;
    terminal.fs.for_each_directory([&](FileSystem::Directory &directory) {
        const auto size = directory.cached_size();

        if (size + free_space >= FileSystem::NeededSpace && size < minimum_sufficient_space) {
            minimum_sufficient_space = size;
        }
    });

    return minimum_sufficient_space;
}

constexpr std::size_t sum_of_directory_sizes_with_limit_from_string_data(const std::string_view data) {
    return sum_of_directory_sizes_with_limit(10'0000, advent::views::split_lines(data));
}

constexpr std::size_t find_directory_to_delete_from_string_data(const std::string_view data) {
    return find_directory_to_delete(advent::views::split_lines(data));
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
