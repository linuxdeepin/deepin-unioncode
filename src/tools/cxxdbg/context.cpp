#include "context.h"

namespace  cxxdbg {

// blocks until the event is fired.
void Context::wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&] { return fired;} );
}

// sets signals the event, and unblocks any calls to wait().
void Context::fire()
{
    std::unique_lock<std::mutex> lock(mutex);
    fired = true;
    cv.notify_all();
}

}
