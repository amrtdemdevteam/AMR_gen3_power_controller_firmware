#include <Arduino.h>

#include "../include/digital_input_pin.hpp"

DigitalInputPin::DigitalInputPin(const Config& config)
	: config_(config) {
}

DigitalInputPin::DigitalInputPin(uint8_t pin, bool active_low)
	: config_{pin, active_low} {
}

bool DigitalInputPin::begin() {
	if (config_.pin == 0) {
		return false;
	}

	pinMode(config_.pin, config_.active_low ? INPUT_PULLUP : INPUT);
	state_ = readHardwareState();
	initialized_ = true;
	return true;
}

bool DigitalInputPin::update() {
	if (!initialized_) {
		return false;
	}

	const bool current_state = readHardwareState();
	if (current_state == state_) {
		return false;
	}

	const bool previous_state = state_;
	state_ = current_state;

	if (change_callback_ != nullptr) {
		change_callback_(current_state, previous_state);
	}

	return true;
}

bool DigitalInputPin::state() const {
	return state_;
}

uint8_t DigitalInputPin::pin() const {
	return config_.pin;
}

void DigitalInputPin::setChangeCallback(ChangeCallback callback) {
	change_callback_ = callback;
}

bool DigitalInputPin::readHardwareState() const {
	const bool pin_is_high = (digitalRead(config_.pin) == HIGH);
	return config_.active_low ? !pin_is_high : pin_is_high;
}
