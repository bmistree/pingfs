#ifndef _BLOCK_
#define _BLOCK_

#include <memory>
#include <string>

namespace pingfs {

typedef uint64_t BlockId;

class Block {
 public:
    Block(BlockId block_id, const std::string& data);
    Block(BlockId block_id, std::shared_ptr<const std::string> data);
    ~Block();

    BlockId get_block_id() const;

    std::shared_ptr<const std::string> get_data() const;

    bool operator==(const Block &other) const;

    bool operator!=(const Block &other) const;

 private:
    const BlockId block_id_;
    const std::shared_ptr<const std::string> data_;
};

}  // namespace pingfs

#endif
