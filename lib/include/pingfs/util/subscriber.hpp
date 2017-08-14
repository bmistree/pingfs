#ifndef _SUBSCRIBER_
#define _SUBSCRIBER_

namespace pingfs {

template <typename T>
class Subscriber {
    
public:
    virtual void process(const T& notice) = 0;
    virtual ~Subscriber() {
    }
};

}  // namespace pingfs

#endif
