#pragma once

#include <Arduino.h>

class ButtonEventMonitor {
public:
    enum class Event {
        NONE = 0,
        PRESSED,
        RELEASED,
        SHORT_HOLD,
        LONG_HOLD
    };

    struct Config {
        uint8_t pin = 0;
        bool activeLow = false;
        unsigned long debounceMs = 30;
        unsigned long shortHoldMs = 3000;
        unsigned long longHoldMs = 5000;
    };

    explicit ButtonEventMonitor(const Config& config);

    bool begin();
    void setDurations(unsigned long shortHoldMs, unsigned long longHoldMs);
    bool isPressed() const;
    bool pollEvent(Event& event);

private:
    Config config_;

    bool rawPressed_ = false;
    bool stablePressed_ = false;
    bool initialized_ = false;

    unsigned long lastRawChangeMs_ = 0;
    unsigned long pressStartMs_ = 0;

    bool shortHoldReported_ = false;
    bool longHoldReported_ = false;

    Event pendingEvent_ = Event::NONE;

    bool readPressed() const;
    void queueEvent(Event event);
};