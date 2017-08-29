#ifndef _MODE_
#define _MODE_

#include <block_data.pb.h>

#include "read_write_execute.hpp"

namespace pingfs {

class Mode {
 public:
    Mode(const ReadWriteExecute& user_mode,
        const ReadWriteExecute& group_mode,
        const ReadWriteExecute& other_mode);
    explicit Mode(const proto::ModeProto& proto);

    ~Mode();

    bool operator==(const Mode &other) const;
    bool operator!=(const Mode &other) const;

    void gen_proto(proto::ModeProto* proto) const;
    const ReadWriteExecute& get_user_mode() const;
    const ReadWriteExecute& get_group_mode() const;
    const ReadWriteExecute& get_other_mode() const;

 private:
    const ReadWriteExecute user_mode_;
    const ReadWriteExecute group_mode_;
    const ReadWriteExecute other_mode_;
};

}  // namespace pingfs

#endif
