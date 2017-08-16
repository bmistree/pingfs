#ifndef _PUBLISHER_
#define _PUBLISHER_

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/locks.hpp>

#include <vector>

#include "subscriber.hpp"

namespace pingfs {

template <typename T>
class Publisher {
 public:
    /**
     * @param subscriber Caller is responsible for freeing this memory (and
     * calling unsubscribe before freeing it).
     */
    void subscribe(Subscriber<T>* subscriber) {
        boost::recursive_mutex::scoped_lock locker(subscriber_mutex_);
        subscribers_.push_back(subscriber);
    }

    void unsubscribe(Subscriber<T>* subscriber) {
        boost::recursive_mutex::scoped_lock locker(subscriber_mutex_);
        subscribers_.erase(
            std::remove(subscribers_.begin(), subscribers_.end(), subscriber),
            subscribers_.end());
    }

    virtual ~Publisher() {}

 protected:
    Publisher() {}

    void notify(const T& notice) {
        boost::recursive_mutex::scoped_lock locker(subscriber_mutex_);
        // Making a copy here so that a subscriber can call unsubscribe
        // from within its notification method.
        std::vector<Subscriber<T>*> vec_copy = subscribers_;
        for (auto iter = vec_copy.cbegin(); iter != vec_copy.end();
             ++iter) {
            (*iter)->process(notice);
        }
    }

 private:
    boost::recursive_mutex subscriber_mutex_;
    /**
     * Each element is loaned from caller.
     */
    std::vector<Subscriber<T>*> subscribers_;
};

}  // namespace pingfs

#endif
