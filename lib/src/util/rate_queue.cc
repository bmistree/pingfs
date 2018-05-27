
#include <pingfs/util/rate_queue.hpp>

#include <functional>
#include <list>
#include <thread>


namespace pingfs {

std::shared_ptr<RateQueue> RateQueue::start_queue(
    const std::chrono::milliseconds& delay) {
    std::shared_ptr<RateQueue> rate_queue(
        new RateQueue(delay));
    rate_queue->start();
    return rate_queue;
}

RateQueue::RateQueue(const std::chrono::milliseconds& delay)
 : delay_(delay),
   queue_(),
   queue_mutex_(),
   stopped_(false) {
}

RateQueue::~RateQueue() {
    stop();
}

void RateQueue::start() {
    thread_ = std::thread(&RateQueue::service, this);
}

void RateQueue::add(const std::function<void()>& function) {
    boost::mutex::scoped_lock locker(queue_mutex_);
    if (stopped_) {
        throw "Attempting to add work to stopped service";
    }
    queue_.push(function);
}

void RateQueue::stop() {
    bool should_join = true;
    {
        boost::mutex::scoped_lock locker(queue_mutex_);
        should_join = !stopped_;
        stopped_ = true;
        while(!queue_.empty()) {
            queue_.pop();
        }
    }
    // Join when not holding mutex to ensure that
    // service terminates.
    if (should_join) {
        thread_.join();
    }
}

void RateQueue::service() {
    while (!stopped_) {
        queue_mutex_.lock();
        if (!queue_.empty()) {
            std::function<void()> to_exec = queue_.front();
            queue_.pop();
            queue_mutex_.unlock();
            to_exec();
        } else {
            queue_mutex_.unlock();
        }
        std::this_thread::sleep_for(delay_);
    }
}

}  // namespace pingfs
