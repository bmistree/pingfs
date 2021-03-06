#include <pingfs/block/block_data/mode.hpp>
#include <pingfs/block/block_data/stat.hpp>

#include <time.h>
#include <block_data.pb.h>

namespace pingfs {

Stat::Stat(const Mode& mode, uint32_t uid, uint32_t gid, uint64_t size,
    time_t access_time, time_t mod_time, time_t status_change_time)
  : mode_(mode),
    uid_(uid),
    gid_(gid),
    size_(size),
    access_time_(access_time),
    mod_time_(mod_time),
    status_change_time_(status_change_time) {
}

Stat::Stat(const proto::StatProto& proto)
  : Stat(Mode(proto.mode()), proto.uid(), proto.gid(),
      proto.size(), proto.access_time(), proto.mod_time(),
      proto.status_change_time()) {
}

Stat::Stat(const Stat& stat, uint64_t size, time_t access_time,
    time_t mod_time, time_t status_change_time)
  : mode_(stat.mode_),
    uid_(stat.uid_),
    gid_(stat.gid_),
    size_(size),
    access_time_(access_time),
    mod_time_(mod_time),
    status_change_time_(status_change_time) {
}

Stat::~Stat() {
}

void Stat::update_stat(dev_t dev, ino_t ino, struct stat* stbuf) const {
    stbuf->st_ino = ino;
    stbuf->st_dev = dev;
    stbuf->st_mode = get_mode().to_mode_t();
    stbuf->st_uid = uid_;
    stbuf->st_gid = gid_;
    stbuf->st_size = size_;
    stbuf->st_atime = access_time_;
    stbuf->st_mtime = mod_time_;
    stbuf->st_ctime = status_change_time_;
    stbuf->st_nlink = 1;
}

bool Stat::operator==(const Stat& other) const {
    return ((mode_ == other.mode_) &&
        (uid_ == other.uid_) &&
        (gid_ == other.gid_) &&
        (size_ == other.size_) &&
        (access_time_ == other.access_time_) &&
        (mod_time_ == other.mod_time_) &&
        (status_change_time_ == other.status_change_time_));
}

bool Stat::operator!=(const Stat& other) const {
    return !(*this == other);
}

void Stat::gen_proto(proto::StatProto* proto) const {
    mode_.gen_proto(proto->mutable_mode());
    proto->set_uid(uid_);
    proto->set_gid(gid_);
    proto->set_size(size_);
    proto->set_access_time(access_time_);
    proto->set_mod_time(mod_time_);
    proto->set_status_change_time(status_change_time_);
}

const Mode& Stat::get_mode() const {
    return mode_;
}

mode_t Stat::get_mode_t() const {
    return mode_.to_mode_t();
}

uint32_t Stat::get_uid() const {
    return uid_;
}

uint32_t Stat::get_gid() const {
    return gid_;
}

uint64_t Stat::get_size() const {
    return size_;
}

time_t Stat::get_access_time() const {
    return access_time_;
}

time_t Stat::get_mod_time() const {
    return mod_time_;
}

time_t Stat::get_status_change_time() const {
    return status_change_time_;
}

bool Stat::is_dir() const {
    return mode_.get_file_type() == FileType::DIR;
}

}  // namespace pingfs
