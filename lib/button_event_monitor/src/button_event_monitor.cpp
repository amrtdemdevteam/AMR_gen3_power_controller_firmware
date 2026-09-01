#include "button_event_monitor.hpp"

ButtonEventMonitor::ButtonEventMonitor(const Config& config)
	: config_(config) {
}

bool ButtonEventMonitor::begin() {
	if ((config_.long_hold_ms != 0) &&
		(config_.short_hold_ms != 0) &&
		(config_.long_hold_ms <= config_.short_hold_ms)) {
		return false;
	}

	pinMode(config_.pin, config_.active_low ? INPUT_PULLUP : INPUT);

	raw_pressed_ = readPressed();
	stable_pressed_ = raw_pressed_;
	initialized_ = true;
	last_raw_change_ms_ = millis();
	press_start_ms_ = raw_pressed_ ? last_raw_change_ms_ : 0;
	short_hold_reported_ = false;
	long_hold_reported_ = false;
	pending_event_ = Event::NONE;

	return true;
}

void ButtonEventMonitor::setDurations(unsigned long short_hold_ms, unsigned long long_hold_ms) {
	if ((long_hold_ms != 0) && (short_hold_ms != 0) && (long_hold_ms <= short_hold_ms)) {
		return;
	}

	config_.short_hold_ms = short_hold_ms;
	config_.long_hold_ms = long_hold_ms;
}

bool ButtonEventMonitor::isPressed() const {
	return stable_pressed_;
}

bool ButtonEventMonitor::pollEvent(Event& event) {
	if (!initialized_) {
		event = Event::NONE;
		return false;
	}

	const unsigned long now = millis();
	const bool current_raw_pressed = readPressed();

	if (current_raw_pressed != raw_pressed_) {
		raw_pressed_ = current_raw_pressed;
		last_raw_change_ms_ = now;
	}

	if (raw_pressed_ != stable_pressed_) {
		if ((now - last_raw_change_ms_) >= DEBOUNCE_MS) {
			stable_pressed_ = raw_pressed_;

			if (stable_pressed_) {
				press_start_ms_ = now;
				short_hold_reported_ = false;
				long_hold_reported_ = false;
				queueEvent(Event::PRESSED);
			} else {
				short_hold_reported_ = false;
				long_hold_reported_ = false;
				queueEvent(Event::RELEASED);
			}
		}
	} else if (stable_pressed_) {
		const unsigned long held_ms = now - press_start_ms_;

		if (!short_hold_reported_ && (config_.short_hold_ms != 0) && (held_ms >= config_.short_hold_ms)) {
			short_hold_reported_ = true;
			queueEvent(Event::SHORT_HOLD);
		}

		if (!long_hold_reported_ && (config_.long_hold_ms != 0) && (held_ms >= config_.long_hold_ms)) {
			long_hold_reported_ = true;
			queueEvent(Event::LONG_HOLD);
		}
	}

	if (pending_event_ == Event::NONE) {
		event = Event::NONE;
		return false;
	}

	event = pending_event_;
	pending_event_ = Event::NONE;
	return true;
}

bool ButtonEventMonitor::readPressed() const {
	const bool pinIsHigh = (digitalRead(config_.pin) == HIGH);
	return config_.active_low ? !pinIsHigh : pinIsHigh;
}

void ButtonEventMonitor::queueEvent(Event event) {
	if ((pending_event_ == Event::NONE) || (event == Event::LONG_HOLD)) {
		pending_event_ = event;
	}
}
