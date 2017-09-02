
#include <pingfs/block/block_data/read_write_execute.hpp>
#include <pingfs/block/block_data/mode.hpp>

namespace pingfs {

Mode::Mode(const ReadWriteExecute& user_mode,
    const ReadWriteExecute& group_mode,
    const ReadWriteExecute& other_mode,
    const FileType& file_type)
  : user_mode_(user_mode),
    group_mode_(group_mode),
    other_mode_(other_mode),
    file_type_(file_type) {
}

Mode::Mode(const proto::ModeProto& proto)
  : Mode(ReadWriteExecuteFactory::from_proto(proto.user_mode()),
      ReadWriteExecuteFactory::from_proto(proto.group_mode()),
      ReadWriteExecuteFactory::from_proto(proto.other_mode()),
      FileTypeFactory::from_proto(proto.file_type())) {
}

Mode::Mode(const mode_t mode) {
    lkjs;
}

Mode::~Mode() {
}

bool Mode::operator==(const Mode &other) const {
    return ((user_mode_ == other.user_mode_) &&
        (group_mode_ == other.group_mode_) &&
        (other_mode_ == other.other_mode_) &&
        (file_type_ == other.file_type_));
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

    proto::FileTypeProto file_type_proto;
    FileTypeFactory::gen_proto(&file_type_proto, file_type_);
    proto->set_file_type(file_type_proto);
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

const FileType& Mode::get_file_type() const {
    return file_type_;
}

static mode_t get_user_mode(const ReadWriteExecute& read_write_execute) {
    switch (read_write_execute_) {
      case ReadWriteExectue::NONE:
        return 0;
      case ReadWriteExecute::READ:
        return S_IRUSR;
      case ReadWriteExecute::READ_WRITE:
        return S_IRUSR | S_IWUSR;
      case ReadWriteExecute::READ_EXECUTE:
        return S_IRUSR | S_XUSR;
      case ReadWriteExecute::READ_WRITE_EXECUTE:
        return S_IRUSR | S_IWUSR | S_XUSR;
      case ReadWriteExecute::WRITE:
        return S_IWUSR;
      case ReadWriteExecute::WRITE_EXECUTE:
        return S_IWUSR | S_XUSR;
      case ReadWriteExecute::EXECUTE:
        return S_XUSR;
      default:
        throw "Unknown mode";
    }
}

static mode_t get_group_mode(const ReadWriteExecute& read_write_execute) {
    switch (read_write_execute) {
      case ReadWriteExectue::NONE:
        return 0;
      case ReadWriteExecute::READ:
        return S_IRGRP;
      case ReadWriteExecute::READ_WRITE:
        return S_IRGRP | S_IWGRP;
      case ReadWriteExecute::READ_EXECUTE:
        return S_IRGRP | S_XGRP;
      case ReadWriteExecute::READ_WRITE_EXECUTE:
        return S_IRGRP | S_IWGRP | S_XGRP;
      case ReadWriteExecute::WRITE:
        return S_IWGRP;
      case ReadWriteExecute::WRITE_EXECUTE:
        return S_IWGRP | S_XGRP;
      case ReadWriteExecute::EXECUTE:
        return S_XGRP;
      default:
        throw "Unknown mode";
    }
}

static mode_t get_other_mode(const ReadWriteExecute& read_write_execute) {
    switch (read_write_execute) {
      case ReadWriteExectue::NONE:
        return 0;
      case ReadWriteExecute::READ:
        return S_IROTH;
      case ReadWriteExecute::READ_WRITE:
        return S_IROTH | S_IWOTH;
      case ReadWriteExecute::READ_EXECUTE:
        return S_IROTH | S_XOTH;
      case ReadWriteExecute::READ_WRITE_EXECUTE:
        return S_IROTH | S_IWOTH | S_XOTH;
      case ReadWriteExecute::WRITE:
        return S_IWOTH;
      case ReadWriteExecute::WRITE_EXECUTE:
        return S_IWOTH | S_XOTH;
      case ReadWriteExecute::EXECUTE:
        return S_XOTH;
        break;
      default:
        throw "Unknown mode";
    }
}

mode_t Mode::to_mode() const {
    mode_t m = 0;
    m |= FileTypeFactory::to_mode(file_type_);
    m |= get_user_mode(user_mode_);
    m |= get_group_mode(group_mode_);
    m |= get_other_mode(other_mode_);
    return m;
}

}  // namespace pingfs
