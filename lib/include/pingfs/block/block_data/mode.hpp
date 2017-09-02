#ifndef _MODE_
#define _MODE_

#include <block_data.pb.h>
#include <sys/stat.h>

#include "file_type.hpp"
#include "read_write_execute.hpp"

namespace pingfs {

class Mode {
 public:
    Mode(const ReadWriteExecute& user_mode,
        const ReadWriteExecute& group_mode,
        const ReadWriteExecute& other_mode,
        const FileType& file_type);
    explicit Mode(const proto::ModeProto& proto);
    explicit Mode(const mode_t mode);

    ~Mode();

    bool operator==(const Mode &other) const;
    bool operator!=(const Mode &other) const;

    void gen_proto(proto::ModeProto* proto) const;
    const ReadWriteExecute& get_user_mode() const;
    const ReadWriteExecute& get_group_mode() const;
    const ReadWriteExecute& get_other_mode() const;
    const FileType& get_file_type() const;

    mode_t to_mode_t() const;

 private:
    const ReadWriteExecute user_mode_;
    const ReadWriteExecute group_mode_;
    const ReadWriteExecute other_mode_;
    const FileType file_type_;
};

}  // namespace pingfs

#endif
