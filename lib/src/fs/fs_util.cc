#include <pingfs/fs/fs_util.hpp>

#include <string>
#include <vector>

namespace pingfs {

const std::string FsUtil::PATH_SEPARATOR = "/";

FsUtil::FsUtil() {
}

FsUtil::~FsUtil() {
}

const std::string& FsUtil::get_separator() {
    return PATH_SEPARATOR;
}

std::vector<std::string> FsUtil::separate_path(
    const std::string& abs_path) {
    std::vector<std::string> split_path({ get_separator() });
    // Handle case that this is just the root directory
    if (abs_path == get_separator()) {
        return split_path;
    }

    // Start at the second position to skip "/".
    std::size_t from_index = 1;
    std::size_t index = abs_path.find(get_separator(), from_index);
    while (index != std::string::npos) {
        split_path.push_back(
            abs_path.substr(from_index, index - from_index));
        from_index = index + 1;
        index = abs_path.find(get_separator(), from_index);
    }
    // Add final suffix. E.g., for /a/b.txt, the above would generate
    // {/, a}, this line adds on b.txt.
    split_path.push_back(abs_path.substr(from_index));
    return split_path;
}


const std::string FsUtil::join(const std::vector<std::string>& paths) {
    std::string joined;
    for (std::size_t i = 0; i < paths.size() - 1; ++i) {
        joined += paths[i];
        if (paths[i] == get_separator()) {
            // handle the case of a root directory
            continue;
        }
        joined += get_separator();
    }
    if (!paths.empty()) {
        joined += paths[paths.size() - 1];
    }

    return joined;
}

}  // namespace pingfs
