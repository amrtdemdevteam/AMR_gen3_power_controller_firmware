#include "state_event_timer_manager.hpp"

bool StateEventTimerManager::register_timer(uint8_t timer_id, StateEventTimer* timer) {
	if (nullptr == timer) {
		return false;
	}

	if (Slot* slot = find_slot(timer_id)) {
		slot->timer = timer;
		slot->in_use = true;
		return true;
	}

	for (auto& slot : slots_) {
		if (!slot.in_use) {
			slot.in_use = true;
			slot.timer_id = timer_id;
			slot.timer = timer;
			return true;
		}
	}

	return false;
}

StateEventTimer* StateEventTimerManager::get_timer(uint8_t timer_id) const {
	if (const Slot* slot = find_slot(timer_id)) {
		return slot->timer;
	}

	return nullptr;
}

bool StateEventTimerManager::start_timer(uint8_t timer_id) {
	if (StateEventTimer* timer = get_timer(timer_id)) {
		timer->start();
		return true;
	}

	return false;
}

bool StateEventTimerManager::stop_timer(uint8_t timer_id) {
	if (StateEventTimer* timer = get_timer(timer_id)) {
		timer->stop();
		return true;
	}

	return false;
}

void StateEventTimerManager::clear() {
	for (auto& slot : slots_) {
		slot = {};
	}
}

StateEventTimerManager::Slot* StateEventTimerManager::find_slot(uint8_t timer_id) {
	for (auto& slot : slots_) {
		if (slot.in_use && slot.timer_id == timer_id) {
			return &slot;
		}
	}

	return nullptr;
}

const StateEventTimerManager::Slot* StateEventTimerManager::find_slot(uint8_t timer_id) const {
	for (const auto& slot : slots_) {
		if (slot.in_use && slot.timer_id == timer_id) {
			return &slot;
		}
	}

	return nullptr;
}