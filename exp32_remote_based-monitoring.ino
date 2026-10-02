/**
 * =========================================================================================
 * Project: ESP32-Based Remote Monitoring & Control System
 * Architecture: Modular, Event-Driven, Non-Blocking Cooperative Multitasking
 * Target MCU: ESP32 Dev Module / NodeMCU-32S
 * =========================================================================================
 */

#include <Arduino.h>
#include <Wire.h>

#include "Config.h"
#include "SystemState.h"
#include "ControlManager.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "CommunicationManager.h"

// --- Global Subsystem Instances ---
SystemState          sysState;
ControlManager       controlManager;
SensorManager        sensorManager;
DisplayManager       displayManager;
CommunicationManager commManager;

// --- Task Schedulers ---
unsigned long previousSensorMillis  = 0;
unsigned long previousDisplayMillis = 0;

void setup() {
    // 1. Initialize Serial Diagnostics
    Serial.begin(115200);
    delay(250);
    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F(" ESP32 Industrial Remote Monitoring & Control     "));
    Serial.println(F(" Initializing System Modules...                   "));
    Serial.println(F("=================================================="));

    // 2. Initialize Shared Hardware I2C Bus
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    // 3. Initialize Control Manager (Fail-Safe Actuator Default)
    controlManager.begin(sysState);

    // 4. Initialize OLED Local Display
    displayManager.begin();

    // 5. Initialize Sensor Subsystems (DHT11, BMP280/180, Ultrasonic)
    sensorManager.begin();

    // 6. Initialize Wireless Network & Telegram Service
    commManager.begin();

    // 7. Perform Initial Telemetry Sample & Display Refresh
    sensorManager.update(sysState);
    sysState.uptimeSeconds = millis() / 1000;
    displayManager.render(sysState);

    Serial.println(F("[System] All modules initialized. Entering non-blocking main loop."));
}

void loop() {
    unsigned long currentMillis = millis();

    // --- Task 1: Wi-Fi Link Watchdog & Reconnection ---
    commManager.maintainConnection(sysState);

    // --- Task 2: Remote Telegram Message Dispatcher ---
    commManager.processTelegram(sysState, controlManager);

    // --- Task 3: Sensor Data Acquisition Task (Every 2000 ms) ---
    if (currentMillis - previousSensorMillis >= INTERVAL_SENSOR_READ) {
        previousSensorMillis = currentMillis;
        
        sensorManager.update(sysState);
        sysState.uptimeSeconds = currentMillis / 1000;
        
        // Debug telemetry printout to Serial Monitor
        Serial.printf("[Telemetry] T: %.1f C | H: %.1f %% | P: %.1f hPa | D: %.1f cm | Actuator: %s\n",
                      sysState.temperature,
                      sysState.humidity,
                      sysState.pressure,
                      sysState.distanceCm,
                      sysState.actuatorState ? "ON" : "OFF");
    }

    // --- Task 4: Local OLED Display Refresh Task (Every 500 ms) ---
    if (currentMillis - previousDisplayMillis >= INTERVAL_DISPLAY_UPDATE) {
        previousDisplayMillis = currentMillis;
        
        displayManager.render(sysState);
    }

    // Yield to FreeRTOS background tasks (WiFi, TCP/IP stack)
    yield();
}
