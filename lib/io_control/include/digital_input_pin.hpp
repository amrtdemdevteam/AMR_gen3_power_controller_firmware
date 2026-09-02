// digital_input_pin.hpp
#pragma once

#include <Arduino.h>

class DigitalInputPin {
public:
	struct Config {
		uint8_t pin = 0;
		bool active_low = false;
	};

	using ChangeCallback = void (*)(bool current_state, bool previous_state);

	explicit DigitalInputPin(const Config& config);
	DigitalInputPin(uint8_t pin, bool active_low = false);

	bool begin();

	// Read pin and trigger callback if value changed since the previous update.
	bool update();

	bool state() const;

	uint8_t pin() const;

	void setChangeCallback(ChangeCallback callback);

private:
	Config config_;
	bool state_ = false;
	bool initialized_ = false;

	ChangeCallback change_callback_ = nullptr;
	void* callback_user_data_ = nullptr;

	bool readHardwareState() const;
};
