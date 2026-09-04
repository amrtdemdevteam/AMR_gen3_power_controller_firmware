// digital_output_pin.hpp
#pragma once
#include <Arduino.h>

class DigitalOutputPin {
public:
	enum class OutputMode : uint8_t {
		ACTIVE_LOW = 0,
		ACTIVE_HIGH
		};
    struct Config {
        uint8_t pin = 0;
        OutputMode output_mode = OutputMode::ACTIVE_HIGH;
        bool initial_on = false;     // สถานะเริ่มต้นหลัง begin()
    };

    explicit DigitalOutputPin(const Config& config);
    DigitalOutputPin(uint8_t pin, OutputMode output_mode = OutputMode::ACTIVE_HIGH, bool initial_on = false);

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