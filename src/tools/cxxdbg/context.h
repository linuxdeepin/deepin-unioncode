#ifndef CONTEXT_H
#define CONTEXT_H

#include <mutex>
#include <condition_variable>

namespace cxxdbg {

//provides a basic wait and signal synchronization primitive.
class Context
{
public:
    // wait() blocks until the event is fired.
    void wait();

    // fire() sets signals the event, and unblocks any calls to wait().
    void fire();

private:
    std::mutex mutex;
    std::condition_variable cv;
    bool fired = false;
};

}
;

#endif // CONTEXT_H
