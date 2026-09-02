#pragma once

#include <Arduino.h>
#include <arduino-timer.h>

#include <functional>


class StateEventTimer {
    /**
     * StateEventTimer is a wrapper around the arduino-timer library that provides a convenient interface for single-shot or repeating timers.
     */
public:

    /**
     * Timer engine type will be whatever is returned by timer_create_default() in arduino-timer.
     */
    using TimerEngine = decltype(timer_create_default());

    /**
     * Callback type for the timer event.
     */
    using Callback = std::function<void()>;

    StateEventTimer(TimerEngine& timer_engine, Callback callback, unsigned long timeout_ms, bool single_shot = true);


    void set_callback(Callback callback);

    void start();

    void stop();

    bool is_active() const;

private:
    static bool timer_thunk(void* timer_context);

    bool on_timer();

    TimerEngine& timer_engine_;
    Callback callback_;
    unsigned long timeout_ms_ = 0;
    bool single_shot_ = true;
    typename TimerEngine::Task task_{};
    bool active_ = false;
};