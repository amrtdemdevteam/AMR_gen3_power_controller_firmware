#include <Arduino.h>
#include <memory>
#include "power_control_state_machine.hpp"
#include "state_callback.hpp"

PowerControlStateMachine powerControlHsm;

void powerControlHsmRegisterCallbacks();

void printHelp();





void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

    powerControlHsmRegisterCallbacks();

    powerControlHsm.begin();

}

void loop() {

    powerControlHsm.run();


    while (Serial.available() > 0) {
        const char cmd = static_cast<char>(Serial.read());

        switch (cmd) {
            case 'p': powerControlHsm.postEvent(PowerControlStateMachine::Event::POWER_ON); break;
            case 'o': powerControlHsm.postEvent(PowerControlStateMachine::Event::POWER_OFF); break;
            case 'w': powerControlHsm.postEvent(PowerControlStateMachine::Event::FMS_WAKE_UP); break;
            case 'i': powerControlHsm.postEvent(PowerControlStateMachine::Event::IPC_OK); break;
            case 'x': powerControlHsm.postEvent(PowerControlStateMachine::Event::IPC_FAIL); break;
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
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SHUTDOWN, nullptr, on_enter_shutdown, on_exit_shutdown);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SERVICE_LAYER, nullptr, on_enter_service_layer, on_exit_service_layer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SLEEP, nullptr, on_enter_sleep, on_exit_sleep);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::CHARGED, nullptr, on_enter_charged, on_exit_charged);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SERVICE_MODE, nullptr, on_enter_service_mode, on_exit_service_mode);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::SERVICE_EMER, nullptr, on_enter_service_emer, on_exit_service_emer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::CHARGING, nullptr, on_enter_charging, on_exit_charging);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::GO_TO_CHARGER, nullptr, on_enter_go_to_charger, on_exit_go_to_charger);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::WAIT_CHARGER, nullptr, on_enter_wait_charger, on_exit_wait_charger);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::CHARGING_IN_PROGRESS, nullptr, on_enter_charging_in_progress, on_exit_charging_in_progress);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::APPLICATION_LAYER, nullptr, on_enter_application_layer, on_exit_application_layer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::OPERATION, nullptr, on_enter_operation, on_exit_operation);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::APP_EMER, nullptr, on_enter_app_emer, on_exit_app_emer);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::MANUAL, nullptr, on_enter_manual, on_exit_manual);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::STANDBY, nullptr, on_enter_standby, on_exit_standby);
    powerControlHsm.setStateCallbacks(PowerControlStateMachine::State::AUTO, nullptr, on_enter_auto, on_exit_auto);

    powerControlHsm.setTransitionFailedCallback(on_transition_failed);
}

void printHelp() {
    Serial.println("Commands:");
    Serial.println("  p: POWER_ON");
    Serial.println("  o: POWER_OFF");
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


