#pragma once

#include <Arduino.h>

class LedController {
public:
    enum class State {
        OFF = 0,
        SOLID,
        BLINK_SLOW,
        BLINK_FAST
    };

    struct Config {
        uint8_t pin = 13;
        bool active_low = false;
        float blink_slow_hz = 1.0f;
        float blink_fast_hz = 4.0f;
    };

    explicit LedController(const Config& config);

    bool begin();
    void run();

    void setState(State state);

    void setStateOff();

    void setStateSolid();

    void setStateBlinkSlow();
    
    void setStateBlinkFast();

    State state() const;

private:
    Config config_;
    State state_ = State::OFF;
    bool initialized_ = false;
    bool ledOn_ = false;

    unsigned long last_toggle_ms_ = 0;
    unsigned long half_period_ms_ = 500;

    static constexpr float MIN_BLINK_HZ = 1.0f;
    static constexpr float MAX_BLINK_HZ = 10.0f;

    void writeLed(bool on);
    bool computeHalfPeriodMs(float hz, unsigned long& outHalfPeriodMs) const;
};