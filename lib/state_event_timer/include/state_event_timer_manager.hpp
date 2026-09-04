#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "state_event_timer.hpp"

class StateEventTimerManager {
public:
    static constexpr size_t MAX_TIMER_COUNT = 8;

    bool register_timer(uint8_t timer_id, StateEventTimer* timer);

    StateEventTimer* get_timer(uint8_t timer_id) const;

    bool start_timer(uint8_t timer_id);

    bool stop_timer(uint8_t timer_id);

    void clear();

private:
    struct Slot {
        bool in_use = false;
        uint8_t timer_id = 0;
        StateEventTimer* timer = nullptr;
    };

    Slot* find_slot(uint8_t timer_id);
    const Slot* find_slot(uint8_t timer_id) const;

    std::array<Slot, MAX_TIMER_COUNT> slots_{};
};