#pragma once

#include <cstdint>
#include <string>
#include <hsmcpp.hpp>

#include "led_controller.hpp"

class StateEventTimer;
class StateEventTimerManager;
class DigitalOutputPin;

enum class StateEventTimerId : uint8_t {
	INIT_DONE = 0,
	CHARGED_IDLE_TIMEOUT,
	AUTO_POWER_ON,
	COUNT
};


enum class LedRole : uint8_t {
	POWER = 0,
	MASTER,
	RUN,
	COUNT // Keep this as the last element to represent the number of LED roles
};

enum class DigitalOutputRole : uint8_t {
	BATTERY_RELAY = 0,
	CHARGER_RELAY,
	MOT_DRV_RELAY,
	CONTROL_RELAY,
	AUX_DEV_RELAY,
	UNLOCK_MOTOR,
	POWER_ON,
	EMERGENCY_OUT,
	COUNT
};

/**
 * @brief Registers an LED controller for a specific LED role.
 *
 * @param led_role The role of the LED to register.
 * @param led_controller Pointer to the LED controller instance.
 * @return true if the registration was successful, false otherwise.
 **/
bool registerLedController(LedRole led_role, LedController* led_controller);

bool getLedController(LedRole led_role, LedController** led_controller);

bool isValidLedRole(LedRole led_role);

void clearLedControllers();

bool registerDigitalOutputPin(DigitalOutputRole output_role, DigitalOutputPin* output_pin);

DigitalOutputPin* getDigitalOutputPin(DigitalOutputRole output_role);

bool isValidDigitalOutputRole(DigitalOutputRole output_role);

void clearDigitalOutputPins();

void setStateEventTimerManager(StateEventTimerManager* timer_manager);

bool registerStateEventTimer(StateEventTimerId timer_id, StateEventTimer* timer);

bool startStateEventTimer(StateEventTimerId timer_id);

bool stopStateEventTimer(StateEventTimerId timer_id);

StateEventTimer* getStateEventTimer(StateEventTimerId timer_id);

void onStateChanged(const hsmcpp::VariantVector_t& params);

bool onEnterShutdown(const hsmcpp::VariantVector_t& params);
bool onExitShutdown();

bool onEnterInit(const hsmcpp::VariantVector_t& params);
bool onExitInit();

bool onEnterServiceLayer(const hsmcpp::VariantVector_t& params);
bool onExitServiceLayer();

bool onEnterSleep(const hsmcpp::VariantVector_t& params);
bool onExitSleep();

bool onEnterCharged(const hsmcpp::VariantVector_t& params);
bool onExitCharged();

bool onEnterServiceMode(const hsmcpp::VariantVector_t& params);
bool onExitServiceMode();

bool onEnterServiceEmer(const hsmcpp::VariantVector_t& params);
bool onExitServiceEmer();

bool onEnterCharging(const hsmcpp::VariantVector_t& params);
bool onExitCharging();

bool onEnterGoToCharger(const hsmcpp::VariantVector_t& params);
bool onExitGoToCharger();

bool onEnterWaitCharger(const hsmcpp::VariantVector_t& params);
bool onExitWaitCharger();

bool onEnterChargingInProgress(const hsmcpp::VariantVector_t& params);
bool onExitChargingInProgress();

bool onEnterApplicationLayer(const hsmcpp::VariantVector_t& params);
bool onExitApplicationLayer();

bool onEnterOperation(const hsmcpp::VariantVector_t& params);
bool onExitOperation();

bool onEnterAppEmer(const hsmcpp::VariantVector_t& params);
bool onExitAppEmer();

bool onEnterManual(const hsmcpp::VariantVector_t& params);
bool onExitManual();

bool onEnterStandby(const hsmcpp::VariantVector_t& params);
bool onExitStandby();

bool onEnterAuto(const hsmcpp::VariantVector_t& params);
bool onExitAuto();

void onTransitionFailed(std::string message);