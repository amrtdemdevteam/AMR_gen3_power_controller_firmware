#include <Arduino.h>

#include "../include/digital_output_pin.hpp"

DigitalOutputPin::DigitalOutputPin(const Config& config)
	: config_(config), state_on_(config.initial_on) {
}

DigitalOutputPin::DigitalOutputPin(uint8_t pin, OutputMode output_mode, bool initial_on)
	: config_{pin, output_mode, initial_on}, state_on_(initial_on) {
}


bool DigitalOutputPin::begin() {
	if (config_.pin == 0) {
		return false;
	}

	pinMode(config_.pin, OUTPUT);
	initialized_ = true;
	writeHardware(state_on_);
	return true;
}

void DigitalOutputPin::on() {
	state_on_ = true;
	if (initialized_) {
		writeHardware(state_on_);
	}
}

void DigitalOutputPin::off() {
	state_on_ = false;
	if (initialized_) {
		writeHardware(state_on_);
	}
}

void DigitalOutputPin::set(bool on) {
	state_on_ = on;
	if (initialized_) {
		writeHardware(state_on_);
	}
}

void DigitalOutputPin::toggle() {
	state_on_ = !state_on_;
	if (initialized_) {
		writeHardware(state_on_);
	}
}

bool DigitalOutputPin::isOn() const {
	return state_on_;
}

uint8_t DigitalOutputPin::pin() const {
	return config_.pin;
}

void DigitalOutputPin::writeHardware(bool on) {
	const bool pin_high = config_.output_mode == OutputMode::ACTIVE_LOW ? !on : on;
	digitalWrite(config_.pin, pin_high ? HIGH : LOW);
}
