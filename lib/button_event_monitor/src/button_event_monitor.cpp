#include "button_event_monitor.hpp"

ButtonEventMonitor::ButtonEventMonitor(const Config& config)
	: config_(config) {
}

bool ButtonEventMonitor::begin() {
	if ((config_.longHoldMs != 0) &&
		(config_.shortHoldMs != 0) &&
		(config_.longHoldMs <= config_.shortHoldMs)) {
		return false;
	}

	pinMode(config_.pin, config_.activeLow ? INPUT_PULLUP : INPUT);

	rawPressed_ = readPressed();
	stablePressed_ = rawPressed_;
	initialized_ = true;
	lastRawChangeMs_ = millis();
	pressStartMs_ = rawPressed_ ? lastRawChangeMs_ : 0;
	shortHoldReported_ = false;
	longHoldReported_ = false;
	pendingEvent_ = Event::NONE;

	return true;
}

void ButtonEventMonitor::setDurations(unsigned long shortHoldMs, unsigned long longHoldMs) {
	if ((longHoldMs != 0) && (shortHoldMs != 0) && (longHoldMs <= shortHoldMs)) {
		return;
	}

	config_.shortHoldMs = shortHoldMs;
	config_.longHoldMs = longHoldMs;
}

bool ButtonEventMonitor::isPressed() const {
	return stablePressed_;
}

bool ButtonEventMonitor::pollEvent(Event& event) {
	if (!initialized_) {
		event = Event::NONE;
		return false;
	}

	const unsigned long now = millis();
	const bool currentRawPressed = readPressed();

	if (currentRawPressed != rawPressed_) {
		rawPressed_ = currentRawPressed;
		lastRawChangeMs_ = now;
	}

	if (rawPressed_ != stablePressed_) {
		if ((now - lastRawChangeMs_) >= config_.debounceMs) {
			stablePressed_ = rawPressed_;

			if (stablePressed_) {
				pressStartMs_ = now;
				shortHoldReported_ = false;
				longHoldReported_ = false;
				queueEvent(Event::PRESSED);
			} else {
				shortHoldReported_ = false;
				longHoldReported_ = false;
				queueEvent(Event::RELEASED);
			}
		}
	} else if (stablePressed_) {
		const unsigned long heldMs = now - pressStartMs_;

		if (!shortHoldReported_ && (config_.shortHoldMs != 0) && (heldMs >= config_.shortHoldMs)) {
			shortHoldReported_ = true;
			queueEvent(Event::SHORT_HOLD);
		}

		if (!longHoldReported_ && (config_.longHoldMs != 0) && (heldMs >= config_.longHoldMs)) {
			longHoldReported_ = true;
			queueEvent(Event::LONG_HOLD);
		}
	}

	if (pendingEvent_ == Event::NONE) {
		event = Event::NONE;
		return false;
	}

	event = pendingEvent_;
	pendingEvent_ = Event::NONE;
	return true;
}

bool ButtonEventMonitor::readPressed() const {
	const bool pinIsHigh = (digitalRead(config_.pin) == HIGH);
	return config_.activeLow ? !pinIsHigh : pinIsHigh;
}

void ButtonEventMonitor::queueEvent(Event event) {
	if ((pendingEvent_ == Event::NONE) || (event == Event::LONG_HOLD)) {
		pendingEvent_ = event;
	}
}
