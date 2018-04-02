#include <pingfs/ping/data_buffer.hpp>


namespace pingfs {

DataBuffer::DataBuffer()
 : data_(),
   data_mutex_() {
}

DataBuffer::~DataBuffer() {
}

void DataBuffer::discard(std::size_t bytes_to_remove) {
    boost::mutex::scoped_lock locker(data_mutex_);
    data_.erase(0, bytes_to_remove);
}

void DataBuffer::append(const std::string to_append) {
    boost::mutex::scoped_lock locker(data_mutex_);
    data_ += to_append;
}

std::string DataBuffer::peek() {
   boost::mutex::scoped_lock locker(data_mutex_);
   return data_;
}

std::size_t DataBuffer::size() {
    boost::mutex::scoped_lock locker(data_mutex_);
    return data_.size();
}

}  // namespace pingfs
