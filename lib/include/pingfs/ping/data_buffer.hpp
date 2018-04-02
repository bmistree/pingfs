#ifndef _DATA_BUFFER_
#define _DATA_BUFFER_

#include <boost/thread/mutex.hpp>

namespace pingfs {

class DataBuffer {
 public:
    DataBuffer();
    ~DataBuffer();

    void discard(std::size_t bytes_to_remove);
    void append(const std::string to_append);
    std::string peek();
    std::size_t size();

 private:
    std::string data_;
    boost::mutex data_mutex_;

};

}  // namespace pingfs


#endif
