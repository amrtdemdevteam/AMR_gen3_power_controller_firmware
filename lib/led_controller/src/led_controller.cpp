#include <Arduino.h>

#include "../include/led_controller.hpp"

LedController::LedController(const Config& config)
	: config_(config) {
}

bool LedController::begin() {
	unsigned long lowHalfPeriodMs = 0;
	unsigned long highHalfPeriodMs = 0;
	if (!computeHalfPeriodMs(config_.blink_slow_hz, lowHalfPeriodMs) ||
		!computeHalfPeriodMs(config_.blink_fast_hz, highHalfPeriodMs)) {
		return false;
	}

	pinMode(config_.pin, OUTPUT);
	initialized_ = true;
	last_toggle_ms_ = millis();
	setStateOff();

	return true;
}

void LedController::run() {
	if (!initialized_) {
		return;
	}

	if ((state_ != State::BLINK_SLOW) && (state_ != State::BLINK_FAST)) {
		return;
	}

	const unsigned long now = millis();
	if ((now - last_toggle_ms_) >= half_period_ms_) {
		last_toggle_ms_ = now;
		ledOn_ = !ledOn_;
		writeLed(ledOn_);
	}
}

void LedController::setState(State state) {
	if (!initialized_) {
		state_ = state;
		return;
	}

	state_ = state;

	switch (state_) {
		case State::OFF:
			ledOn_ = false;
			writeLed(false);
			break;

		case State::SOLID:
			ledOn_ = true;
			writeLed(true);
			break;

		case State::BLINK_SLOW: {
			unsigned long half_period_ms = 0;
			if (!computeHalfPeriodMs(config_.blink_slow_hz, half_period_ms)) {
				setStateOff();
				break;
			}
			half_period_ms_ = half_period_ms;
			last_toggle_ms_ = millis();
			ledOn_ = true;
			writeLed(true);
			break;
		}

		case State::BLINK_FAST: {
			unsigned long half_period_ms = 0;
			if (!computeHalfPeriodMs(config_.blink_fast_hz, half_period_ms)) {
				setStateOff();
				break;
			}
			half_period_ms_ = half_period_ms;
			last_toggle_ms_ = millis();
			ledOn_ = true;
			writeLed(true);
			break;
		}
	}
}

void LedController::setStateOff() {
	setState(State::OFF);
}

void LedController::setStateSolid() {
	setState(State::SOLID);
}

void LedController::setStateBlinkSlow() {
	setState(State::BLINK_SLOW);
}

void LedController::setStateBlinkFast() {
	setState(State::BLINK_FAST);
}

LedController::State LedController::state() const {
	return state_;
}

void LedController::writeLed(bool on) {
	const bool pinHigh = config_.active_low ? !on : on;
	digitalWrite(config_.pin, pinHigh ? HIGH : LOW);
}

bool LedController::computeHalfPeriodMs(float hz, unsigned long& outHalfPeriodMs) const {
	if ((hz < MIN_BLINK_HZ) || (hz > MAX_BLINK_HZ)) {
		return false;
	}

	const float half_period = 500.0f / hz;
	if (!(half_period >= 1.0f)) {
		outHalfPeriodMs = 1;
		return true;
	}

	outHalfPeriodMs = static_cast<unsigned long>(half_period);
	if (outHalfPeriodMs == 0) {
		outHalfPeriodMs = 1;
	}

	return true;
}
