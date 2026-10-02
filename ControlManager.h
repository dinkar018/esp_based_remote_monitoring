#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "SystemState.h"

class ControlManager {
public:
    void begin(SystemState &state) {
        pinMode(PIN_ACTUATOR, OUTPUT);
        pinMode(PIN_STATUS_LED, OUTPUT);

        // Fail-safe initialization: Default state MUST be OFF (LOW)
        setActuator(false, state);
        digitalWrite(PIN_STATUS_LED, LOW);

        Serial.println(F("[ControlManager] Actuator initialized to safe state: OFF"));
    }

    void setActuator(bool turnOn, SystemState &state) {
        state.actuatorState = turnOn;
        digitalWrite(PIN_ACTUATOR, turnOn ? HIGH : LOW);
        digitalWrite(PIN_STATUS_LED, turnOn ? HIGH : LOW);
        
        Serial.print(F("[ControlManager] Actuator state transitioned to: "));
        Serial.println(turnOn ? F("ON") : F("OFF"));
    }

    bool toggleActuator(SystemState &state) {
        setActuator(!state.actuatorState, state);
        return state.actuatorState;
    }
};

#endif // CONTROL_MANAGER_H
