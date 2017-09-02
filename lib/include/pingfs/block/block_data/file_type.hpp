#ifndef _FILE_TYPE_
#define _FILE_TYPE_

#include <block_data.pb.h>

namespace pingfs {

enum class FileType {
    DIR,
    REGULAR,
};

class FileTypeFactory {
 public:
    static FileType from_proto(
        const proto::FileTypeProto& proto) {
        switch (proto) {
          case proto::DIR:
            return FileType::DIR;
          case proto::REGULAR:
            return FileType::REGULAR;
          default:
            throw "Unknown proto";
        }
    }

    static void gen_proto(proto::FileTypeProto* proto,
        const FileType& val) {
        switch (val) {
          case FileType::DIR:
            *proto = proto::DIR;
            break;
          case FileType::REGULAR:
            *proto = proto::REGULAR;
            break;
          default:
            throw "Unknown value";
        }
    }

    ~FileTypeFactory() {
    }

 private:
    FileTypeFactory() {
    }
};

}  // namespace pingfs
#endif
