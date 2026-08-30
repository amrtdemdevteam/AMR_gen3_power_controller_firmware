#include <Arduino.h>
#include <memory>


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
    Serial.println("  ?: help");
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

    const uint32_t startupDeadline = millis() + 3000U;
    while (!Serial && (millis() < startupDeadline)) {
    }



    printHelp();
}

void loop() {


    while (Serial.available() > 0) {
        const char cmd = static_cast<char>(Serial.read());

        switch (cmd) {
            case '?': printHelp(); break;
            default: break;
        }
    }
}

