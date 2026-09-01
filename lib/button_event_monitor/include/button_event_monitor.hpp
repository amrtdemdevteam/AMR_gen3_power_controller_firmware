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
        uint8_t pin = 0; // Pin number for the button
        bool active_low = false; // true if the button is active low, false if active high
        unsigned long short_hold_ms = 3000;
        unsigned long long_hold_ms = 5000;
    };

    explicit ButtonEventMonitor(const Config& config);

    bool begin();
    void setDurations(unsigned long short_hold_ms, unsigned long long_hold_ms);
    bool isPressed() const;
    bool pollEvent(Event& event);

private:
    Config config_;

    bool raw_pressed_ = false;
    bool stable_pressed_ = false;
    bool initialized_ = false;

    unsigned long last_raw_change_ms_ = 0;
    unsigned long press_start_ms_ = 0;

    bool short_hold_reported_ = false;
    bool long_hold_reported_ = false;

    Event pending_event_ = Event::NONE;

    static constexpr unsigned long DEBOUNCE_MS = 30;

    bool readPressed() const;
    void queueEvent(Event event);
};