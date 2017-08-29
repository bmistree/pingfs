
#include <pingfs/block/block_data/read_write_execute.hpp>
#include <pingfs/block/block_data/mode.hpp>

namespace pingfs {

Mode::Mode(const ReadWriteExecute& user_mode,
    const ReadWriteExecute& group_mode,
    const ReadWriteExecute& other_mode)
  : user_mode_(user_mode),
    group_mode_(group_mode),
    other_mode_(other_mode) {
}

Mode::Mode(const proto::ModeProto& proto)
  : Mode(ReadWriteExecuteFactory::from_proto(proto.user_mode()),
      ReadWriteExecuteFactory::from_proto(proto.group_mode()),
      ReadWriteExecuteFactory::from_proto(proto.other_mode())) {
}

Mode::~Mode() {
}

bool Mode::operator==(const Mode &other) const {
    return ((user_mode_ == other.user_mode_) &&
        (group_mode_ == other.group_mode_) &&
        (other_mode_ == other.other_mode_));
}

bool Mode::operator!=(const Mode &other) const {
    return !(*this == other);
}

void Mode::gen_proto(proto::ModeProto* proto) const {
    proto::ReadWriteExecuteProto rwe_proto;
    ReadWriteExecuteFactory::gen_proto(&rwe_proto, user_mode_);
    proto->set_user_mode(rwe_proto);

    ReadWriteExecuteFactory::gen_proto(&rwe_proto, group_mode_);
    proto->set_group_mode(rwe_proto);

    ReadWriteExecuteFactory::gen_proto(&rwe_proto, other_mode_);
    proto->set_other_mode(rwe_proto);
}

const ReadWriteExecute& Mode::get_user_mode() const {
    return user_mode_;
}

const ReadWriteExecute& Mode::get_group_mode() const {
    return group_mode_;
}

const ReadWriteExecute& Mode::get_other_mode() const {
    return other_mode_;
}

}  // namespace pingfs
