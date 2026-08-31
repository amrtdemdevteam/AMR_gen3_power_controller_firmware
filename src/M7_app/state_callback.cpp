#include <Arduino.h>
#include "state_callback.hpp"

static void printVariantParams(const hsmcpp::VariantVector_t& params) {
    for (const auto& param : params) {
        if (param.getType() == hsmcpp::Variant::Type::STRING) {
            Serial.print("Param: ");
            Serial.println(param.toString().c_str());
        }
    }
}

void on_state_changed(const hsmcpp::VariantVector_t& params) {
    Serial.print("State changed -> ");
    printVariantParams(params);
}

bool on_enter_shutdown(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SHUTDOWN");
    printVariantParams(params);
    return true;
}

bool on_exit_shutdown() {
    Serial.println("Exiting SHUTDOWN");
    return true;
}

bool on_enter_service_layer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SERVICE_LAYER");
    printVariantParams(params);
    return true;
}

bool on_exit_service_layer() {
    Serial.println("Exiting SERVICE_LAYER");
    return true;
}

bool on_enter_sleep(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SLEEP");
    printVariantParams(params);
    return true;
}

bool on_exit_sleep() {
    Serial.println("Exiting SLEEP");
    return true;
}

bool on_enter_charged(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering CHARGED");
    printVariantParams(params);
    return true;
}

bool on_exit_charged() {
    Serial.println("Exiting CHARGED");
    return true;
}

bool on_enter_service_mode(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SERVICE_MODE");
    printVariantParams(params);
    return true;
}

bool on_exit_service_mode() {
    Serial.println("Exiting SERVICE_MODE");
    return true;
}

bool on_enter_service_emer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering SERVICE_EMER");
    printVariantParams(params);
    return true;
}

bool on_exit_service_emer() {
    Serial.println("Exiting SERVICE_EMER");
    return true;
}

bool on_enter_charging(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering CHARGING");
    printVariantParams(params);
    return true;
}

bool on_exit_charging() {
    Serial.println("Exiting CHARGING");
    return true;
}

bool on_enter_go_to_charger(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering GO_TO_CHARGER");
    printVariantParams(params);
    return true;
}

bool on_exit_go_to_charger() {
    Serial.println("Exiting GO_TO_CHARGER");
    return true;
}

bool on_enter_wait_charger(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering WAIT_CHARGER");
    printVariantParams(params);
    return true;
}

bool on_exit_wait_charger() {
    Serial.println("Exiting WAIT_CHARGER");
    return true;
}

bool on_enter_charging_in_progress(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering CHARGING_IN_PROGRESS");
    printVariantParams(params);
    return true;
}

bool on_exit_charging_in_progress() {
    Serial.println("Exiting CHARGING_IN_PROGRESS");
    return true;
}

bool on_enter_application_layer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering APPLICATION_LAYER");
    printVariantParams(params);
    return true;
}

bool on_exit_application_layer() {
    Serial.println("Exiting APPLICATION_LAYER");
    return true;
}

bool on_enter_operation(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering OPERATION");
    printVariantParams(params);
    return true;
}

bool on_exit_operation() {
    Serial.println("Exiting OPERATION");
    return true;
}

bool on_enter_app_emer(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering APP_EMER");
    printVariantParams(params);
    return true;
}

bool on_exit_app_emer() {
    Serial.println("Exiting APP_EMER");
    return true;
}

bool on_enter_manual(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering MANUAL");
    printVariantParams(params);
    return true;
}

bool on_exit_manual() {
    Serial.println("Exiting MANUAL");
    return true;
}

bool on_enter_standby(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering STANDBY");
    printVariantParams(params);
    return true;
}

bool on_exit_standby() {
    Serial.println("Exiting STANDBY");
    return true;
}

bool on_enter_auto(const hsmcpp::VariantVector_t& params) {
    Serial.println("Entering AUTO");
    printVariantParams(params);
    return true;
}

bool on_exit_auto() {
    Serial.println("Exiting AUTO");
    return true;
}

void on_transition_failed(std::string message) {
    Serial.print("Transition failed: ");
    Serial.println(message.c_str());
}