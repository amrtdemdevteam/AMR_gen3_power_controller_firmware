#pragma once

#include <string>
#include <hsmcpp.hpp>

void on_state_changed(const hsmcpp::VariantVector_t& params);

bool on_enter_shutdown(const hsmcpp::VariantVector_t& params);
bool on_exit_shutdown();

bool on_enter_service_layer(const hsmcpp::VariantVector_t& params);
bool on_exit_service_layer();

bool on_enter_sleep(const hsmcpp::VariantVector_t& params);
bool on_exit_sleep();

bool on_enter_charged(const hsmcpp::VariantVector_t& params);
bool on_exit_charged();

bool on_enter_service_mode(const hsmcpp::VariantVector_t& params);
bool on_exit_service_mode();

bool on_enter_service_emer(const hsmcpp::VariantVector_t& params);
bool on_exit_service_emer();

bool on_enter_charging(const hsmcpp::VariantVector_t& params);
bool on_exit_charging();

bool on_enter_go_to_charger(const hsmcpp::VariantVector_t& params);
bool on_exit_go_to_charger();

bool on_enter_wait_charger(const hsmcpp::VariantVector_t& params);
bool on_exit_wait_charger();

bool on_enter_charging_in_progress(const hsmcpp::VariantVector_t& params);
bool on_exit_charging_in_progress();

bool on_enter_application_layer(const hsmcpp::VariantVector_t& params);
bool on_exit_application_layer();

bool on_enter_operation(const hsmcpp::VariantVector_t& params);
bool on_exit_operation();

bool on_enter_app_emer(const hsmcpp::VariantVector_t& params);
bool on_exit_app_emer();

bool on_enter_manual(const hsmcpp::VariantVector_t& params);
bool on_exit_manual();

bool on_enter_standby(const hsmcpp::VariantVector_t& params);
bool on_exit_standby();

bool on_enter_auto(const hsmcpp::VariantVector_t& params);
bool on_exit_auto();

void on_transition_failed(std::string message);