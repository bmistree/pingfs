#ifndef _STAT_
#define _STAT_

#include <time.h>
#include <unistd.h>
#include <block_data.pb.h>

#include "mode.hpp"

namespace pingfs {

class Stat {
 public:
    explicit Stat(const Mode& mode, uint32_t uid, uint32_t gid,
        uint64_t size, time_t access_time, time_t mod_time,
        time_t status_change_time);

    explicit Stat(const proto::StatProto& proto);
    ~Stat();

    bool operator==(const Stat& other) const;
    bool operator!=(const Stat& other) const;

    void gen_proto(proto::StatProto* proto) const;

    const Mode& get_mode() const;
    uint32_t get_uid() const;
    uint32_t get_gid() const;
    uint64_t get_size() const;
    time_t get_access_time() const;
    time_t get_mod_time() const;
    time_t get_status_change_time() const;
    mode_t get_mode_t() const;
    void update_stat(dev_t dev, ino_t ino, struct stat* stbuf) const;

 private:
    const Mode mode_;
    const uint32_t uid_;
    const uint32_t gid_;
    const uint64_t size_;
    const time_t access_time_;
    const time_t mod_time_;
    const time_t status_change_time_;
};

}  // namespace pingfs

#endif
