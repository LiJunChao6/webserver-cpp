#ifndef TIMERID_H
#define TIMERID_H
#include <boost/operators.hpp>
class Timer;

class TimerId {
public:
    TimerId() :timer_(NULL), sequence_(0) {}
    TimerId(Timer* timer, int64_t seq)
        : timer_(timer),
        sequence_(seq) {}

    friend class TimerQueue;

private:
    Timer* timer_;
    int64_t sequence_;
};

#endif