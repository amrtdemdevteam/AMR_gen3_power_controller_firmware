// digital_output_pin.hpp
#pragma once
#include <Arduino.h>

class DigitalOutputPin {
public:
    struct Config {
        uint8_t pin = 0;
        bool active_low = false;     // true = ON จะเขียน LOW
        bool initial_on = false;     // สถานะเริ่มต้นหลัง begin()
    };

    explicit DigitalOutputPin(const Config& config);
    DigitalOutputPin(uint8_t pin, bool active_low = false, bool initial_on = false);

    bool begin();

    void on();

    void off();

    void set(bool on);

    void toggle();

    bool isOn() const;

    uint8_t pin() const;

private:
    Config config_;
    bool state_on_ = false;
    bool initialized_ = false;

    void writeHardware(bool on);
};