
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

static bool bits_set(mode_t target, mode_t bits_to_check) {
    return (target & bits_to_check) == bits_to_check;
}

static ReadWriteExecute user_mode_from_mode_t(mode_t mode) {
    if (bits_set(mode, S_IRUSR | S_IWUSR | S_IXUSR)) {
        return ReadWriteExecute::READ_WRITE_EXECUTE;
    }
    if (bits_set(mode, S_IRUSR | S_IWUSR)) {
        return ReadWriteExecute::READ_WRITE;
    }
    if (bits_set(mode, S_IRUSR | S_IXUSR)) {
        return ReadWriteExecute::READ_EXECUTE;
    }
    if (bits_set(mode, S_IRUSR)) {
        return ReadWriteExecute::READ;
    }
    if (bits_set(mode, S_IWUSR | S_IXUSR)) {
        return ReadWriteExecute::WRITE_EXECUTE;
    }
    if (bits_set(mode, S_IWUSR)) {
        return ReadWriteExecute::WRITE;
    }
    if (bits_set(mode, S_IXUSR)) {
        return ReadWriteExecute::EXECUTE;
    }
    return ReadWriteExecute::NONE;
}

static ReadWriteExecute group_mode_from_mode_t(mode_t mode) {
    if (bits_set(mode, S_IRGRP | S_IWGRP | S_IXGRP)) {
        return ReadWriteExecute::READ_WRITE_EXECUTE;
    }
    if (bits_set(mode, S_IRGRP | S_IWGRP)) {
        return ReadWriteExecute::READ_WRITE;
    }
    if (bits_set(mode, S_IRGRP | S_IXGRP)) {
        return ReadWriteExecute::READ_EXECUTE;
    }
    if (bits_set(mode, S_IRGRP)) {
        return ReadWriteExecute::READ;
    }
    if (bits_set(mode, S_IWGRP | S_IXGRP)) {
        return ReadWriteExecute::WRITE_EXECUTE;
    }
    if (bits_set(mode, S_IWGRP)) {
        return ReadWriteExecute::WRITE;
    }
    if (bits_set(mode, S_IXGRP)) {
        return ReadWriteExecute::EXECUTE;
    }
    return ReadWriteExecute::NONE;
}

static ReadWriteExecute other_mode_from_mode_t(mode_t mode) {
    if (bits_set(mode, S_IROTH | S_IWOTH | S_IXOTH)) {
        return ReadWriteExecute::READ_WRITE_EXECUTE;
    }
    if (bits_set(mode, S_IROTH | S_IWOTH)) {
        return ReadWriteExecute::READ_WRITE;
    }
    if (bits_set(mode, S_IROTH | S_IXOTH)) {
        return ReadWriteExecute::READ_EXECUTE;
    }
    if (bits_set(mode, S_IROTH)) {
        return ReadWriteExecute::READ;
    }
    if (bits_set(mode, S_IWOTH | S_IXOTH)) {
        return ReadWriteExecute::WRITE_EXECUTE;
    }
    if (bits_set(mode, S_IWOTH)) {
        return ReadWriteExecute::WRITE;
    }
    if (bits_set(mode, S_IXOTH)) {
        return ReadWriteExecute::EXECUTE;
    }
    return ReadWriteExecute::NONE;
}

Mode::Mode(const mode_t mode)
  : Mode(user_mode_from_mode_t(mode),
      group_mode_from_mode_t(mode),
      other_mode_from_mode_t(mode),
      FileTypeFactory::from_mode_t(mode)) {
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

static mode_t get_user_mode_t(const ReadWriteExecute& read_write_execute) {
    switch (read_write_execute) {
      case ReadWriteExecute::NONE:
        return 0;
      case ReadWriteExecute::READ:
        return S_IRUSR;
      case ReadWriteExecute::READ_WRITE:
        return S_IRUSR | S_IWUSR;
      case ReadWriteExecute::READ_EXECUTE:
        return S_IRUSR | S_IXUSR;
      case ReadWriteExecute::READ_WRITE_EXECUTE:
        return S_IRUSR | S_IWUSR | S_IXUSR;
      case ReadWriteExecute::WRITE:
        return S_IWUSR;
      case ReadWriteExecute::WRITE_EXECUTE:
        return S_IWUSR | S_IXUSR;
      case ReadWriteExecute::EXECUTE:
        return S_IXUSR;
      default:
        throw "Unknown mode";
    }
}

static mode_t get_group_mode_t(const ReadWriteExecute& read_write_execute) {
    switch (read_write_execute) {
      case ReadWriteExecute::NONE:
        return 0;
      case ReadWriteExecute::READ:
        return S_IRGRP;
      case ReadWriteExecute::READ_WRITE:
        return S_IRGRP | S_IWGRP;
      case ReadWriteExecute::READ_EXECUTE:
        return S_IRGRP | S_IXGRP;
      case ReadWriteExecute::READ_WRITE_EXECUTE:
        return S_IRGRP | S_IWGRP | S_IXGRP;
      case ReadWriteExecute::WRITE:
        return S_IWGRP;
      case ReadWriteExecute::WRITE_EXECUTE:
        return S_IWGRP | S_IXGRP;
      case ReadWriteExecute::EXECUTE:
        return S_IXGRP;
      default:
        throw "Unknown mode";
    }
}

static mode_t get_other_mode_t(const ReadWriteExecute& read_write_execute) {
    switch (read_write_execute) {
      case ReadWriteExecute::NONE:
        return 0;
      case ReadWriteExecute::READ:
        return S_IROTH;
      case ReadWriteExecute::READ_WRITE:
        return S_IROTH | S_IWOTH;
      case ReadWriteExecute::READ_EXECUTE:
        return S_IROTH | S_IXOTH;
      case ReadWriteExecute::READ_WRITE_EXECUTE:
        return S_IROTH | S_IWOTH | S_IXOTH;
      case ReadWriteExecute::WRITE:
        return S_IWOTH;
      case ReadWriteExecute::WRITE_EXECUTE:
        return S_IWOTH | S_IXOTH;
      case ReadWriteExecute::EXECUTE:
        return S_IXOTH;
      default:
        throw "Unknown mode";
    }
}

mode_t Mode::to_mode_t() const {
    mode_t m = FileTypeFactory::to_mode(file_type_);
    m |= get_user_mode_t(user_mode_);
    m |= get_group_mode_t(group_mode_);
    m |= get_other_mode_t(other_mode_);
    return m;
}

}  // namespace pingfs
