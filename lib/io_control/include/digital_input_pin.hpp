// digital_input_pin.hpp
#pragma once

#include <Arduino.h>

class DigitalInputPin {


	public:
		enum class InputMode : uint8_t {
		ACTIVE_LOW = 0,
		ACTIVE_HIGH
		};
		struct Config {
			uint8_t pin = 0;
			InputMode input_mode = InputMode::ACTIVE_LOW;
		};

		using ChangeCallback = void (*)(bool current_state, bool previous_state);

		explicit DigitalInputPin(const Config& config);
		DigitalInputPin(uint8_t pin, InputMode input_mode = InputMode::ACTIVE_LOW);

		bool begin();

		// Read pin and trigger callback if value changed since the previous update.
		bool update();

		bool state() const;

		uint8_t pin() const;

		void setChangeCallback(ChangeCallback callback);

	private:
		Config config_;
		bool state_ = false;
		bool last_raw_state_ = false;
		bool initialized_ = false;
		unsigned long last_raw_change_time_ms_ = 0;

		ChangeCallback change_callback_ = nullptr;
		void* callback_user_data_ = nullptr;

		static constexpr unsigned int DEBOUNCE_DELAY_MS = 30;

		bool readHardwareState() const;
};
