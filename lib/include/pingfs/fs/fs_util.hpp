#ifndef _FS_UTIL_
#define _FS_UTIL_

#include <string>
#include <vector>

namespace pingfs {

class FsUtil {
 public:
    /**
     * Splits a fully-qualified path into its parts.
     * Note that the 0-index of the returned object
     * will always be "/". All other elements will not
     * have path separators in them. For instance, for
     * the path, "/a/b/c.txt", this function will return
     * the vector {"/", "a", "b", "c.txt"}.
     */
    static std::vector<std::string> separate_path(
        const std::string& abs_path);

    static const std::string& get_separator();

    static const std::string join(const std::vector<std::string>& paths);

    ~FsUtil();
 private:
    static const std::string PATH_SEPARATOR;
    FsUtil();
};


}  // namespace pingfs

#endif
