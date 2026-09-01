#include <Arduino.h>
#include <memory>
#include "power_control_state_machine.hpp"
#include "ipc_status_checker.hpp"
#include "state_callback.hpp"
#include "button_event_monitor.hpp"
#include "led_controller.hpp"

PowerControlStateMachine powerControlHsm;


void powerControlHsmRegisterCallbacks();

void printHelp();

std::unique_ptr<IPCStatusChecker> ipcStatusChecker;

ButtonEventMonitor::Config power_button_config = {
    .pin = 12,
    .active_low = false,
    .short_hold_ms = 3000,
    .long_hold_ms = 5000
};
ButtonEventMonitor powerButtonMonitor(power_button_config);

LedController::Config power_led_config = {
    .pin = 13,
    .active_low = false,
    .blink_slow_hz = 1.0f,
    .blink_fast_hz = 5.0f
};
LedController powerLedController(power_led_config);

void setup() {

    powerLedController.begin();

    //Register led controller to be used in the state callback functions
    registerLedController(LedRole::POWER, &powerLedController);

    Serial.begin(115200);

    powerControlHsmRegisterCallbacks();

    powerControlHsm.begin();

    ipcStatusChecker.reset(new IPCStatusChecker());

    if (!powerButtonMonitor.begin()) {
        Serial.println("Failed to initialize power button monitor");
    }

}

void loop() {

    powerControlHsm.run();
    bool ipcIsOk = false;
    if (ipcStatusChecker->pollStatusChange(ipcIsOk)) {
        powerControlHsm.postEvent(
            ipcIsOk ? PowerControlStateMachine::Event::IPC_OK
                    : PowerControlStateMachine::Event::IPC_FAIL);
    }

    ButtonEventMonitor::Event powerButtonEvent;
    switch(powerButtonMonitor.pollEvent(powerButtonEvent)) {
        case true:
            switch (powerButtonEvent) {
                case ButtonEventMonitor::Event::PRESSED:
                    Serial.println("Power button pressed");
                    break;
                case ButtonEventMonitor::Event::RELEASED:
                    Serial.println("Power button released");
                    break;
                case ButtonEventMonitor::Event::SHORT_HOLD:
                    Serial.println("Power button short hold");
                    powerControlHsm.postEvent(PowerControlStateMachine::Event::POWER_ON);
                    break;
                case ButtonEventMonitor::Event::LONG_HOLD:
                    Serial.println("Power button long hold");
                    powerControlHsm.postEvent(PowerControlStateMachine::Event::POWER_OFF);
                    break;
                default:
                    break;
            }
            break;
        case false:
            // No event
            break;
    }

    powerLedController.run();

    while (Serial.available() > 0) {
        const char cmd = static_cast<char>(Serial.read());

        switch (cmd) {
            case 'p': powerControlHsm.postEvent(PowerControlStateMachine::Event::POWER_ON); break;
            case 'o': powerControlHsm.postEvent(PowerControlStateMachine::Event::POWER_OFF); break;
            case 'd': powerControlHsm.postEvent(PowerControlStateMachine::Event::INIT_DONE); break;
            case 'w': powerControlHsm.postEvent(PowerControlStateMachine::Event::FMS_WAKE_UP); break;
            case 'i': ipcStatusChecker->setIpcStatus(true); break;
            case 'x': ipcStatusChecker->setIpcStatus(false); break;
            case 'l': powerControlHsm.postEvent(PowerControlStateMachine::Event::BATTERY_LOW); break;
            case 'h': powerControlHsm.postEvent(PowerControlStateMachine::Event::BATTERY_HIGH); break;
            case 'a': powerControlHsm.postEvent(PowerControlStateMachine::Event::ARRIVED_AT_CHARGER); break;
            case 'c': powerControlHsm.postEvent(PowerControlStateMachine::Event::CHARGER_PLUGGED); break;
            case 'r': powerControlHsm.postEvent(PowerControlStateMachine::Event::CHARGER_REMOVED); break;
            case 'e': powerControlHsm.postEvent(PowerControlStateMachine::Event::EMER_LOW); break;
            case 'E': powerControlHsm.postEvent(PowerControlStateMachine::Event::EMER_HIGH); break;
            case 'm': powerControlHsm.postEvent(PowerControlStateMachine::Event::MODE_MANUAL); break;
            case 'A': powerControlHsm.postEvent(PowerControlStateMachine::Event::MODE_AUTO); break;
            case 'n': powerControlHsm.postEvent(PowerControlStateMachine::Event::FMS_NO_TASK); break;
            case 't': powerControlHsm.postEvent(PowerControlStateMachine::Event::FMS_TASK_ASSIGNED); break;
            case 'H': printHelp(); break;
            default: break;
        }
    }
}

void powerControlHsmRegisterCallbacks() {
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SHUTDOWN, nullptr, onEnterShutdown, onExitShutdown);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::INIT, nullptr, onEnterInit, onExitInit);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SERVICE_LAYER, nullptr, onEnterServiceLayer, onExitServiceLayer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SLEEP, nullptr, onEnterSleep, onExitSleep);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::CHARGED, nullptr, onEnterCharged, onExitCharged);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SERVICE_MODE, nullptr, onEnterServiceMode, onExitServiceMode);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SERVICE_EMER, nullptr, onEnterServiceEmer, onExitServiceEmer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::CHARGING, nullptr, onEnterCharging, onExitCharging);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::GO_TO_CHARGER, nullptr, onEnterGoToCharger, onExitGoToCharger);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::WAIT_CHARGER, nullptr, onEnterWaitCharger, onExitWaitCharger);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::CHARGING_IN_PROGRESS, nullptr, onEnterChargingInProgress, onExitChargingInProgress);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::APPLICATION_LAYER, nullptr, onEnterApplicationLayer, onExitApplicationLayer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::OPERATION, nullptr, onEnterOperation, onExitOperation);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::APP_EMER, nullptr, onEnterAppEmer, onExitAppEmer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::MANUAL, nullptr, onEnterManual, onExitManual);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::STANDBY, nullptr, onEnterStandby, onExitStandby);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::AUTO, nullptr, onEnterAuto, onExitAuto);

    powerControlHsm.setTransitionFailedCallback(onTransitionFailed);
}

void printHelp() {
    Serial.println("Commands:");
    Serial.println("  p: POWER_ON");
    Serial.println("  o: POWER_OFF");
    Serial.println("  d: INIT_DONE");
    Serial.println("  w: FMS_WAKE_UP");
    Serial.println("  i: IPC_OK");
    Serial.println("  x: IPC_FAIL");
    Serial.println("  l: BATTERY_LOW (<70%)");
    Serial.println("  h: BATTERY_HIGH (>80%)");
    Serial.println("  a: ARRIVED_AT_CHARGER");
    Serial.println("  c: CHARGER_PLUGGED");
    Serial.println("  r: CHARGER_REMOVED");
    Serial.println("  e: EMER_LOW");
    Serial.println("  E: EMER_HIGH");
    Serial.println("  m: MODE_MANUAL");
    Serial.println("  A: MODE_AUTO");
    Serial.println("  n: FMS_NO_TASK");
    Serial.println("  t: FMS_TASK_ASSIGNED");
    Serial.println("  H: help");
}


