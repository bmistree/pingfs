#ifndef _RATE_QUEUE_
#define _RATE_QUEUE_

#include <boost/thread/mutex.hpp>

#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <thread>


namespace pingfs {

class RateQueue {

public:
    static std::shared_ptr<RateQueue> start_queue(
        const std::chrono::milliseconds& delay);
    ~RateQueue();
    void add(const std::function<void()>& function);
    void stop();

private:
    RateQueue(const std::chrono::milliseconds& delay);
    /**
     * Should only be called once when creating and starting the queue.
     */
    void service();

    /**
     * Should only be called once.
     */
    void start();

private:
    const std::chrono::milliseconds delay_;
    std::queue<std::function<void()>> queue_;
    boost::mutex queue_mutex_;
    bool stopped_;
    std::thread thread_;
};

}  // namespace pingfs

#endif
