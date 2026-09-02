#include <Arduino.h>
#include <array>
#include "state_callback.hpp"
#include "led_controller.hpp"
#include "state_event_timer_manager.hpp"

constexpr size_t LED_ROLE_COUNT = static_cast<size_t>(LedRole::COUNT);
std::array<LedController*, LED_ROLE_COUNT> g_led_controllers = {nullptr};
StateEventTimerManager* g_state_event_timer_manager = nullptr;

bool isValidLedRole(LedRole led_role) {
    return static_cast<size_t>(led_role) < LED_ROLE_COUNT;
}

LedController* getLedController(LedRole led_role) {
    if (!isValidLedRole(led_role)) {
        return nullptr;
    }

    LedController* led_controller = g_led_controllers[static_cast<size_t>(led_role)];
    if (nullptr != led_controller) {
        return led_controller;
    }
    return nullptr;
}

    

bool registerLedController(LedRole led_role, LedController* led_controller) {
    if (!isValidLedRole(led_role)) {
        return false;
    }

    g_led_controllers[static_cast<size_t>(led_role)] = led_controller;
    return true;
}

void clearLedControllers() {
    g_led_controllers.fill(nullptr);
}

void setStateEventTimerManager(StateEventTimerManager* timer_manager) {
    g_state_event_timer_manager = timer_manager;
}

bool registerStateEventTimer(StateEventTimerId timer_id, StateEventTimer* timer) {
    if (nullptr == g_state_event_timer_manager) {
        return false;
    }

    return g_state_event_timer_manager->register_timer(static_cast<uint8_t>(timer_id), timer);
}

bool startStateEventTimer(StateEventTimerId timer_id) {
    if (nullptr == g_state_event_timer_manager) {
        return false;
    }

    return g_state_event_timer_manager->start_timer(static_cast<uint8_t>(timer_id));
}

bool stopStateEventTimer(StateEventTimerId timer_id) {
    if (nullptr == g_state_event_timer_manager) {
        return false;
    }

    return g_state_event_timer_manager->stop_timer(static_cast<uint8_t>(timer_id));
}

StateEventTimer* getStateEventTimer(StateEventTimerId timer_id) {
    if (nullptr == g_state_event_timer_manager) {
        return nullptr;
    }

    return g_state_event_timer_manager->get_timer(static_cast<uint8_t>(timer_id));
}


static void printVariantParams(const hsmcpp::VariantVector_t& params) {
    for (const auto& param : params) {
        if (param.getType() == hsmcpp::Variant::Type::STRING) {
            Serial.print("Param: ");
            Serial.println(param.toString().c_str());
        }
    }
}

void onStateChanged(const hsmcpp::VariantVector_t& params) {
    Serial.print("State changed -> ");
    printVariantParams(params);
}

bool onEnterShutdown(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SHUTDOWN");
    printVariantParams(params);

    // Mock implementation for entering shutdown state
    Serial.println("Shutdown IPC");
    Serial.println("Shutdown motor driver");
    Serial.println("Shutdown other peripherals");
    Serial.println("Shutdown complete");

    if (LedController* led = getLedController(LedRole::POWER)) {
        led->setStateOff();
    } // TODO: Handle LedController null

    // Actually turn off final power gate here
    return true;
}

bool onExitShutdown() {
    Serial.println("Exiting SHUTDOWN");

    if (LedController* led = getLedController(LedRole::POWER)) {
        led->setStateBlinkSlow();
    } // TODO: Handle LedController null

    

    return true;
}

bool onEnterInit(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering INIT");
    printVariantParams(params);

    if (startStateEventTimer(StateEventTimerId::INIT_DONE)) {
        Serial.println("INIT_DONE timer started");
    }

    Serial.println("Powering on");
    Serial.println("Initializing subsystems");


    return true;
}

bool onExitInit() {
    Serial.println("Exiting INIT");

    if (stopStateEventTimer(StateEventTimerId::INIT_DONE)) {
        Serial.println("INIT_DONE timer stopped");
    }

    Serial.println("Subsystem initialization completed");
    return true;
}

bool onEnterServiceLayer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SERVICE_LAYER");
    printVariantParams(params);
    return true;
}

bool onExitServiceLayer() {
    Serial.println("Exiting SERVICE_LAYER");
    return true;
}

bool onEnterSleep(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SLEEP");
    printVariantParams(params);
    return true;
}

bool onExitSleep() {
    Serial.println("Exiting SLEEP");
    return true;
}

bool onEnterCharged(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering CHARGED");
    printVariantParams(params);
    return true;
}

bool onExitCharged() {
    Serial.println("Exiting CHARGED");
    return true;
}

bool onEnterServiceMode(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SERVICE_MODE");
    printVariantParams(params);

    if (LedController* led = getLedController(LedRole::POWER)) {
        led->setStateSolid();
    } // TODO: Handle LedController null

    return true;
}

bool onExitServiceMode() {
    Serial.println("Exiting SERVICE_MODE");
    return true;
}

bool onEnterServiceEmer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SERVICE_EMER");
    printVariantParams(params);
    return true;
}

bool onExitServiceEmer() {
    Serial.println("Exiting SERVICE_EMER");
    return true;
}

bool onEnterCharging(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering CHARGING");
    printVariantParams(params);
    return true;
}

bool onExitCharging() {
    Serial.println("Exiting CHARGING");
    return true;
}

bool onEnterGoToCharger(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering GO_TO_CHARGER");
    printVariantParams(params);
    return true;
}

bool onExitGoToCharger() {
    Serial.println("Exiting GO_TO_CHARGER");
    return true;
}

bool onEnterWaitCharger(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering WAIT_CHARGER");
    printVariantParams(params);
    return true;
}

bool onExitWaitCharger() {
    Serial.println("Exiting WAIT_CHARGER");
    return true;
}

bool onEnterChargingInProgress(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering CHARGING_IN_PROGRESS");
    printVariantParams(params);
    return true;
}

bool onExitChargingInProgress() {
    Serial.println("Exiting CHARGING_IN_PROGRESS");
    return true;
}

bool onEnterApplicationLayer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering APPLICATION_LAYER");
    printVariantParams(params);
    return true;
}

bool onExitApplicationLayer() {
    Serial.println("Exiting APPLICATION_LAYER");
    return true;
}

bool onEnterOperation(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering OPERATION");
    printVariantParams(params);
    if (LedController* led = getLedController(LedRole::POWER)) {
        led->setState(LedController::State::SOLID);
    }
    return true;
}

bool onExitOperation() {
    Serial.println("Exiting OPERATION");
    return true;
}

bool onEnterAppEmer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering APP_EMER");
    printVariantParams(params);
    return true;
}

bool onExitAppEmer() {
    Serial.println("Exiting APP_EMER");
    return true;
}

bool onEnterManual(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering MANUAL");
    printVariantParams(params);
    return true;
}

bool onExitManual() {
    Serial.println("Exiting MANUAL");
    return true;
}

bool onEnterStandby(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering STANDBY");
    printVariantParams(params);
    return true;
}

bool onExitStandby() {
    Serial.println("Exiting STANDBY");
    return true;
}

bool onEnterAuto(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering AUTO");
    printVariantParams(params);
    return true;
}

bool onExitAuto() {
    Serial.println("Exiting AUTO");
    return true;
}

void onTransitionFailed(std::string message) {
    Serial.print("Transition failed: ");
    Serial.println(message.c_str());
}