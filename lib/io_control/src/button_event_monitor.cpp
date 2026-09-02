#include "button_event_monitor.hpp"

ButtonEventMonitor::ButtonEventMonitor(const Config& config)
	: config_(config), input_pin_(config.pin, config.active_low) {
}

bool ButtonEventMonitor::begin() {
	if ((config_.long_hold_ms != 0) &&
		(config_.short_hold_ms != 0) &&
		(config_.long_hold_ms <= config_.short_hold_ms)) {
		return false;
	}

	if (!input_pin_.begin()) {
		return false;
	}

	raw_pressed_ = input_pin_.state();
	stable_pressed_ = raw_pressed_;
	initialized_ = true;
	last_raw_change_ms_ = millis();
	press_start_ms_ = raw_pressed_ ? last_raw_change_ms_ : 0;
	short_hold_reported_ = false;
	long_hold_reported_ = false;
	input_pin_.setChangeCallback(&ButtonEventMonitor::onInputChanged);

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

void ButtonEventMonitor::setEventCallback(EventCallback callback) {
	event_callback_ = callback;
}

void ButtonEventMonitor::update() {
	if (!initialized_) {
		return;
	}

	input_pin_.update();


	const unsigned long now = millis();

	if (raw_pressed_ != stable_pressed_) {
		if ((now - last_raw_change_ms_) >= DEBOUNCE_MS) {
			stable_pressed_ = raw_pressed_;

			if (stable_pressed_) {
				press_start_ms_ = now;
				short_hold_reported_ = false;
				long_hold_reported_ = false;
				emitEvent(Event::PRESSED);
			} else {
				short_hold_reported_ = false;
				long_hold_reported_ = false;
				emitEvent(Event::RELEASED);
			}
		}
		return;
	}

	processHoldEvents(now);
}

void ButtonEventMonitor::onInputChanged(bool current_state, bool previous_state) {
	(void)previous_state;

	// Assuming a single instance or using a singleton pattern for the monitor
	// Replace 'instance' with the actual instance of ButtonEventMonitor
	extern ButtonEventMonitor* instance;
	if (instance == nullptr) {
		return;
	}

	instance->handleInputChanged(current_state);
}

void ButtonEventMonitor::handleInputChanged(bool current_state) {
	raw_pressed_ = current_state;
	last_raw_change_ms_ = millis();
}

void ButtonEventMonitor::emitEvent(Event event) {
	if (event_callback_ != nullptr) {
		event_callback_(event);
	}
}

void ButtonEventMonitor::processHoldEvents(unsigned long now) {
	if (!stable_pressed_) {
		return;
	}

	const unsigned long held_ms = now - press_start_ms_;

	if (!long_hold_reported_ && (config_.long_hold_ms != 0) && (held_ms >= config_.long_hold_ms)) {
		long_hold_reported_ = true;
		short_hold_reported_ = true;
		emitEvent(Event::LONG_HOLD);
		return;
	}

	if (!short_hold_reported_ && (config_.short_hold_ms != 0) && (held_ms >= config_.short_hold_ms)) {
		short_hold_reported_ = true;
		emitEvent(Event::SHORT_HOLD);
	}
}
