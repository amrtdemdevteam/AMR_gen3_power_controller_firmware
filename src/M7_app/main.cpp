#include <Arduino.h>
#include <cstdio>
#include <memory>
#include <arduino-timer.h>
#include "RPC.h"
#include "power_control_state_machine.hpp"
#include "ipc_status_checker.hpp"
#include "state_callback.hpp"
#include "button_event_monitor.hpp"
#include "led_controller.hpp"
#include "state_event_timer.hpp"
#include "state_event_timer_manager.hpp"
#include "electrical_measurement.hpp"
#include "digital_output_pin.hpp"
#include "digital_input_pin.hpp"
#include <ArduinoJson.h>

//Output pins
#define PIN_BATTERY_RELAY         9
#define PIN_CHARGER_RELAY         10
#define PIN_MOT_DRV_RELAY         11
#define PIN_CONTROL_RELAY         12 // Control ตัวไหน?
#define PIN_AUX_DEV_RELAY         13
#define PIN_UNLOCK_MOTOR          49 // ใช่ brake release มั๊ย
#define PIN_POWER_ON              51 // Power on signal for this controller
#define PIN_SERIAL_DR             33 // เอาไว้ทำอะไร
#define PIN_LED_BUTTON1           35 //What is this button?
#define PIN_LED_BUTTON2           37 //What is this button?
#define PIN_EMERGENCY_OUT         47

//Input pins
#define PIN_SW_OFF                41
#define PIN_SW_REQ_UNLOCK         31
#define PIN_EMERGENCY_IN          45

PowerControlStateMachine powerControlHsm;
auto app_timer = timer_create_default();
StateEventTimerManager state_event_timer_manager;

// Post event INIT_DONE after a timeout
constexpr unsigned long INIT_DONE_TIMEOUT_MS = 5000;
StateEventTimer init_done_timer(
    app_timer,
    []() {
        powerControlHsm.postEvent(PowerControlStateMachine::Event::INIT_DONE);
    },
    INIT_DONE_TIMEOUT_MS,
    true
);

// Auto power on timer
constexpr unsigned long AUTO_POWER_ON_TIMEOUT_MS = 3000;
StateEventTimer auto_power_on_timer(
    app_timer,
    []() {
        powerControlHsm.postEvent(PowerControlStateMachine::Event::POWER_ON);
    },
    AUTO_POWER_ON_TIMEOUT_MS,
    true
);


ElectricalMeasurement electricalMeasurement;
constexpr unsigned long ELECTRICAL_MEASUREMENT_SAMPLE_INTERVAL_MS = 1000;
StateEventTimer electrical_measurement_sample_timer(
    app_timer,
    []() {
        //electricalMeasurement.sampleAll();
        // เก็บค่าวัดเอาไว้ในตัวแปรแล้วค่อยส่งเมื่อได้รับ request
    },
    ELECTRICAL_MEASUREMENT_SAMPLE_INTERVAL_MS,
    false
);

std::string getElectricalMeasurementString(uint8_t channel);


void printElectricalData();


void powerControlHsmRegisterCallbacks();

void printHelp();

void initDigitalOutputPins();
void initDigitalInputPins();

void updateDigitalInputPins();


//Define output control pins
DigitalOutputPin batteryRelay(PIN_BATTERY_RELAY, DigitalOutputPin::OutputMode::ACTIVE_HIGH, false);
DigitalOutputPin chargerRelay(PIN_CHARGER_RELAY, DigitalOutputPin::OutputMode::ACTIVE_HIGH, false);
DigitalOutputPin motDrvRelay(PIN_MOT_DRV_RELAY, DigitalOutputPin::OutputMode::ACTIVE_HIGH, false);
DigitalOutputPin controlRelay(PIN_CONTROL_RELAY, DigitalOutputPin::OutputMode::ACTIVE_HIGH, false);
DigitalOutputPin auxDevRelay(PIN_AUX_DEV_RELAY, DigitalOutputPin::OutputMode::ACTIVE_HIGH, false);
DigitalOutputPin unlockMotorOut(PIN_UNLOCK_MOTOR, DigitalOutputPin::OutputMode::ACTIVE_HIGH, false);
DigitalOutputPin powerOn(PIN_POWER_ON, DigitalOutputPin::OutputMode::ACTIVE_HIGH, false);
DigitalOutputPin emergencyOut(PIN_EMERGENCY_OUT, DigitalOutputPin::OutputMode::ACTIVE_LOW, false);

//Define input pins
DigitalInputPin swOff(PIN_SW_OFF, DigitalInputPin::InputMode::ACTIVE_LOW); //active low
void swOffChanged(bool current_state, bool previous_state);

DigitalInputPin swReqUnlock(PIN_SW_REQ_UNLOCK, DigitalInputPin::InputMode::ACTIVE_LOW);//active low
void swReqUnlockChanged(bool current_state, bool previous_state);

DigitalInputPin emergencyIn(PIN_EMERGENCY_IN, DigitalInputPin::InputMode::ACTIVE_LOW);//active low: true
void emergencyInChanged(bool current_state, bool previous_state);
void printEmerInState();

std::unique_ptr<IPCStatusChecker> ipcStatusChecker;

// Just for demo
ButtonEventMonitor::Config power_button_config = {
    .pin = 6, // เปลี่ยนกลับเป็น 6 เพื่อใช้ปุ่มทดสอบ
    .input_mode = DigitalInputPin::InputMode::ACTIVE_HIGH,
    .short_hold_ms = 3000,
    .long_hold_ms = 5000
};
ButtonEventMonitor powerButton(power_button_config);
void onPowerButtonEvent(ButtonEventMonitor::Event event);


// Just for demo
LedController::Config power_led_config = {
    .pin = 5,
    .output_mode = DigitalOutputPin::OutputMode::ACTIVE_HIGH,
    .blink_slow_hz = 2.0f,
    .blink_fast_hz = 5.0f
};
LedController powerLedController(power_led_config);


/*int add(int a, int b) {
    return a + b;
}*/




void setup() {
    Serial.begin(115200);
    RPC.begin();
    //RPC.bind("add", add);
    RPC.bind("getElectricalMeasurement", getElectricalMeasurementString);
    initDigitalOutputPins();
    initDigitalInputPins();

    powerLedController.begin();

    electricalMeasurement.begin();
    electrical_measurement_sample_timer.start();

    //Register these components to be used in state callback functions
    registerLedController(LedRole::POWER, &powerLedController);
    registerDigitalOutputPin(DigitalOutputRole::BATTERY_RELAY, &batteryRelay);
    //registerDigitalOutputPin(DigitalOutputRole::CHARGER_RELAY, &chargerRelay);
    registerDigitalOutputPin(DigitalOutputRole::MOT_DRV_RELAY, &motDrvRelay);
    registerDigitalOutputPin(DigitalOutputRole::CONTROL_RELAY, &controlRelay);
    registerDigitalOutputPin(DigitalOutputRole::AUX_DEV_RELAY, &auxDevRelay);
    registerDigitalOutputPin(DigitalOutputRole::UNLOCK_MOTOR, &unlockMotorOut);
    registerDigitalOutputPin(DigitalOutputRole::POWER_ON, &powerOn);
    registerDigitalOutputPin(DigitalOutputRole::EMERGENCY_OUT, &emergencyOut);
    powerControlHsmRegisterCallbacks();
    setStateEventTimerManager(&state_event_timer_manager);
    registerStateEventTimer(StateEventTimerId::INIT_DONE, &init_done_timer);
    registerStateEventTimer(StateEventTimerId::AUTO_POWER_ON, &auto_power_on_timer);

    powerControlHsm.begin(); // Start with the INIT state just for testing หลังจากอ่านค่า power button ได้ค่อยเริ่มจาก SHUTDOWN

    ipcStatusChecker.reset(new IPCStatusChecker());

    



}

void initDigitalOutputPins() {
    batteryRelay.begin();
    chargerRelay.begin();
    motDrvRelay.begin();
    controlRelay.begin();
    auxDevRelay.begin();
    unlockMotorOut.begin();
    powerOn.begin();
}

void initDigitalInputPins() {

    powerButton.setEventCallback(onPowerButtonEvent);
    powerButton.begin();

    emergencyIn.setChangeCallback(emergencyInChanged);
    emergencyIn.begin();
    swOff.setChangeCallback(swOffChanged);
    swOff.begin();
    swReqUnlock.setChangeCallback(swReqUnlockChanged);
    swReqUnlock.begin();
}



void updateDigitalInputPins() {
    powerButton.update();
    emergencyIn.update();
    swOff.update();
    swReqUnlock.update();
}

void loop() {

    app_timer.tick();

    powerControlHsm.run();
    bool ipcIsOk = false;
    if (ipcStatusChecker->pollStatusChange(ipcIsOk)) {
        powerControlHsm.postEvent(
            ipcIsOk ? PowerControlStateMachine::Event::IPC_OK
                    : PowerControlStateMachine::Event::IPC_FAIL);
    }

    updateDigitalInputPins();

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
            case 'e': powerControlHsm.postEvent(PowerControlStateMachine::Event::EMER_OFF); break;
            case 'E': powerControlHsm.postEvent(PowerControlStateMachine::Event::EMER_ON); break;
            case 'm': powerControlHsm.postEvent(PowerControlStateMachine::Event::MODE_MANUAL); break;
            case 'A': powerControlHsm.postEvent(PowerControlStateMachine::Event::MODE_AUTO); break;
            case 'n': powerControlHsm.postEvent(PowerControlStateMachine::Event::FMS_NO_TASK); break;
            case 't': powerControlHsm.postEvent(PowerControlStateMachine::Event::FMS_TASK_ASSIGNED); break;
            case 'v': printElectricalData(); break;
            case 'z': printEmerInState(); break;
            case 'H': printHelp(); break;
            default: break;
        }
    }

    while(RPC.available()) {
        // To  see what happen inside M4
        String rpc_msg =RPC.readStringUntil('\n');
        Serial.println(rpc_msg);
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
    Serial.println("  v: PRINT_VOLTAGE");
    Serial.println("  z: PRINT_EMER_IN_STATE");
    Serial.println("  H: help");
}

void onPowerButtonEvent(ButtonEventMonitor::Event event) {

    switch (event) {
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
}

void emergencyInChanged(bool current_state, bool previous_state) {
    if (current_state) {
        Serial.println("Emergency input activated");
        //emergencyOut.on();
        powerControlHsm.postEvent(PowerControlStateMachine::Event::EMER_ON);
    } else {
        Serial.println("Emergency input deactivated");
        //emergencyOut.off();
        powerControlHsm.postEvent(PowerControlStateMachine::Event::EMER_OFF);
    }
}

void swReqUnlockChanged(bool current_state, bool previous_state) {
    if (current_state) {
        Serial.println("Request to unlock activated");
        if(DigitalOutputPin* unlockMotorPin = getDigitalOutputPin(DigitalOutputRole::UNLOCK_MOTOR)) {
            unlockMotorPin->on();
        }
    } else {
        Serial.println("Request to unlock deactivated");
        if(DigitalOutputPin* unlockMotorPin = getDigitalOutputPin(DigitalOutputRole::UNLOCK_MOTOR)) {
            unlockMotorPin->off();
        }
    }
}

void swOffChanged(bool current_state, bool previous_state) {
    if (current_state) {
        Serial.println("SW_OFF activated");
    } else {
        Serial.println("SW_OFF deactivated");
    }
}

std::string getElectricalMeasurementString(uint8_t channel) {
    JsonDocument document;

    ElectricalMeasurement::Channel::Value channel_value;
    if (!ElectricalMeasurement::Channel::FromUint8(channel, channel_value)) {
        document["error"] = "invalid_channel";
        document["channel"] = channel;

        std::string result;
        serializeJson(document, result);
        Serial.println(result.c_str());
        return result;
    }

    auto channel_data = electricalMeasurement.readChannelData(channel_value);
    if (channel_data == nullptr) {
        document["error"] = "channel_data_unavailable";
        document["channel"] = channel;

        std::string result;
        serializeJson(document, result);
        Serial.println(result.c_str());
        return result;
    }


    document["name"] = ElectricalMeasurement::Channel::ToString(channel_data->channel);
  
    document["value"] = channel_data->physical;
    document["unit"] = ElectricalMeasurement::Unit::ToString(channel_data->unit);

    std::string result;
    serializeJson(document, result);
    Serial.println(result.c_str());
    return result;
}

void printElectricalData(){
    auto channel_data_list = electricalMeasurement.readAll();
    for (const auto& channel_data : *channel_data_list) {
        Serial.print(ElectricalMeasurement::Channel::ToString(channel_data.channel).c_str());
        Serial.print(" : ");
        Serial.print(channel_data.physical);
        Serial.print("  ");
        Serial.print(ElectricalMeasurement::Unit::ToString(channel_data.unit).c_str());
        Serial.println(".");
    }
}

void printEmerInState() {
    Serial.print("Emergency input state: ");
    Serial.println(emergencyIn.state() ? "Activated" : "Deactivated");
}