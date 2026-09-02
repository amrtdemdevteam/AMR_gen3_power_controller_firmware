#include "state_event_timer.hpp"

StateEventTimer::StateEventTimer(TimerEngine& timer_engine, Callback callback, unsigned long timeout_ms, bool single_shot)
	: timer_engine_(timer_engine), callback_(callback), timeout_ms_(timeout_ms), single_shot_(single_shot) {}



void StateEventTimer::set_callback(Callback callback) {
	callback_ = callback;
}

void StateEventTimer::start() {
	stop();

	if (!callback_ || timeout_ms_ == 0) {
		return;
	}

	if (single_shot_) {
		task_ = timer_engine_.in(timeout_ms_, &StateEventTimer::timer_thunk, this);
	} else {
		task_ = timer_engine_.every(timeout_ms_, &StateEventTimer::timer_thunk, this);
	}

	active_ = true;
}

void StateEventTimer::stop() {
	if (!active_) {
		return;
	}

	timer_engine_.cancel(task_);
	active_ = false;
}

bool StateEventTimer::is_active() const {
	return active_;
}

bool StateEventTimer::timer_thunk(void* timer_context) {
	if (nullptr == timer_context) {
		return false;
	}

	return static_cast<StateEventTimer*>(timer_context)->on_timer();
}

bool StateEventTimer::on_timer() {
	if (callback_) {
		callback_();
	}

	if (single_shot_) {
		active_ = false;
		return false;
	}

	return true;
}
