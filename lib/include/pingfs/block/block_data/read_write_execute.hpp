#ifndef _READ_WRITE_EXECUTE_
#define _READ_WRITE_EXECUTE_

#include <block_data.pb.h>

namespace pingfs {

enum class ReadWriteExecute {
    NONE,
    READ,
    READ_WRITE,
    READ_EXECUTE,
    READ_WRITE_EXECUTE,
    WRITE,
    WRITE_EXECUTE,
    EXECUTE,
};

class ReadWriteExecuteFactory {
 public:
    static ReadWriteExecute from_proto(
        const proto::ReadWriteExecuteProto& proto) {
        switch (proto) {
          case proto::NONE:
            return ReadWriteExecute::NONE;
          case proto::READ:
            return ReadWriteExecute::READ;
          case proto::READ_WRITE:
            return ReadWriteExecute::READ_WRITE;
          case proto::READ_EXECUTE:
            return ReadWriteExecute::READ_EXECUTE;
          case proto::READ_WRITE_EXECUTE:
            return ReadWriteExecute::READ_WRITE_EXECUTE;
          case proto::WRITE:
            return ReadWriteExecute::WRITE;
          case proto::WRITE_EXECUTE:
            return ReadWriteExecute::WRITE_EXECUTE;
          case proto::EXECUTE:
            return ReadWriteExecute::EXECUTE;
          default:
            throw "Unknown proto";
        }
    }

    static void gen_proto(proto::ReadWriteExecuteProto* proto,
        const ReadWriteExecute& val) {
        switch (val) {
          case ReadWriteExecute::NONE:
            *proto = proto::NONE;
            break;
          case ReadWriteExecute::READ:
            *proto = proto::READ;
            break;
          case ReadWriteExecute::READ_WRITE:
            *proto = proto::READ_WRITE;
            break;
          case ReadWriteExecute::READ_EXECUTE:
            *proto = proto::READ_EXECUTE;
            break;
          case ReadWriteExecute::READ_WRITE_EXECUTE:
            *proto = proto::READ_WRITE_EXECUTE;
            break;
          case ReadWriteExecute::WRITE:
            *proto = proto::WRITE;
            break;
          case ReadWriteExecute::WRITE_EXECUTE:
            *proto = proto::WRITE_EXECUTE;
            break;
          case ReadWriteExecute::EXECUTE:
            *proto = proto::EXECUTE;
            break;
          default:
            throw "Unknown value";
        }
    }

    ~ReadWriteExecuteFactory() {
    }

 private:
    ReadWriteExecuteFactory() {
    }
};

}  // namespace pingfs

#endif
