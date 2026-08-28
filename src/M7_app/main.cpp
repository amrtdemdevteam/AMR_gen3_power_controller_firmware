#include <Arduino.h>

#include "Fsm.h"
void on_red_enter();
void on_red_state();
void on_red_exit();
void on_green_enter();
void on_green_state();
void on_green_exit();
void on_blue_enter();
void on_blue_state();
void on_blue_exit();
void on_red_to_green_transition();
void on_green_to_blue_transition();
void on_blue_to_red_transition();

State state_red(&on_red_enter, &on_red_state, &on_red_exit);
State state_green(&on_green_enter, &on_green_state, &on_green_exit);
State state_blue(&on_blue_enter, &on_blue_state, &on_blue_exit);

//create a state machine with the initial state set to state_red
Fsm fsm(&state_red);

enum Events {
    EVENT_TIMEOUT
};


void setup() {
    Serial.begin(115200);

    fsm.add_transition(&state_red, &state_green, EVENT_TIMEOUT, &on_red_to_green_transition);
    fsm.add_transition(&state_green, &state_blue, EVENT_TIMEOUT, &on_green_to_blue_transition);
    fsm.add_transition(&state_blue, &state_red, EVENT_TIMEOUT, nullptr); // No transition action for blue to red

}

void loop() {
    fsm.run_machine();

    // Simulate a timeout event every 5 seconds
    static unsigned long last_event_time = 0;
    if (millis() - last_event_time >= 5000) {
        fsm.trigger(EVENT_TIMEOUT);
        last_event_time = millis();
    }
}

void on_red_enter() {
    Serial.println("Red state entered");

}

void on_red_state() {
    Serial.println("Red state active");
}

void on_red_exit() {
    Serial.println("Red state exited");
}

void on_green_enter() {
    Serial.println("Green state entered");
}

void on_green_state() {
    Serial.println("Green state active");
}
void on_green_exit() {
    Serial.println("Green state exited");
}
void on_blue_enter() {
    Serial.println("Blue state entered");
}
void on_blue_state() {
    Serial.println("Blue state active");
}
void on_blue_exit() {
    Serial.println("Blue state exited");
}


void on_red_to_green_transition() {
    Serial.println("Transitioning from Red to Green");
}

void on_green_to_blue_transition() {
    Serial.println("Transitioning from Green to Blue");
}

void on_blue_to_red_transition() {
    Serial.println("Transitioning from Blue to Red");
}