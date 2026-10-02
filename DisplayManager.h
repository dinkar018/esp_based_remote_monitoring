#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "SystemState.h"

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    bool displayAvailable;

public:
    DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), displayAvailable(false) {}

    void begin() {
        if (display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
            displayAvailable = true;
            display.clearDisplay();
            display.setTextColor(SSD1306_WHITE);
            display.setTextSize(1);
            display.setCursor(10, 20);
            display.println(F("ESP32 MONITOR"));
            display.setCursor(10, 35);
            display.println(F("Booting System..."));
            display.display();
            delay(500);
            Serial.println(F("[DisplayManager] SSD1306 initialized successfully."));
        } else {
            Serial.println(F("[DisplayManager] WARNING: SSD1306 allocation failed!"));
        }
    }

    void render(const SystemState &state) {
        if (!displayAvailable) return;

        display.clearDisplay();

        // --- Status Header Bar ---
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.print(F("SYS:"));
        display.print(state.wifiConnected ? F("ONLINE") : F("OFFLINE"));

        // Actuator status pill on top right
        display.setCursor(85, 0);
        display.print(F("ACT:"));
        display.print(state.actuatorState ? F("ON") : F("OFF"));

        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        // --- Telemetry Grid (Row 1: Temp & Humidity) ---
        display.setCursor(0, 14);
        display.print(F("T: "));
        if (state.dhtStatus == SENSOR_OK || state.bmpStatus == SENSOR_OK) {
            display.print(state.temperature, 1);
            display.print((char)247); // Degree symbol
            display.print(F("C"));
        } else {
            display.print(F("ERR"));
        }

        display.setCursor(70, 14);
        display.print(F("H: "));
        if (state.dhtStatus == SENSOR_OK) {
            display.print(state.humidity, 0);
            display.print(F("%"));
        } else {
            display.print(F("ERR"));
        }

        // --- Telemetry Grid (Row 2: Pressure) ---
        display.setCursor(0, 28);
        display.print(F("P: "));
        if (state.bmpStatus == SENSOR_OK) {
            display.print(state.pressure, 1);
            display.print(F(" hPa"));
        } else {
            display.print(F("SENSOR ERR"));
        }

        // --- Telemetry Grid (Row 3: Ultrasonic Distance) ---
        display.setCursor(0, 42);
        display.print(F("Dist: "));
        if (state.ultrasonicStatus == SENSOR_OK) {
            display.print(state.distanceCm, 1);
            display.print(F(" cm"));
        } else {
            display.print(F("OUT OF RANGE"));
        }

        // --- Footer Line: RSSI and Uptime ---
        display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
        display.setCursor(0, 56);
        if (state.wifiConnected) {
            display.print(F("RSSI:"));
            display.print(state.wifiRSSI);
            display.print(F("dBm"));
        } else {
            display.print(F("Reconnecting..."));
        }

        display.setCursor(84, 56);
        display.print(F("Up:"));
        display.print(state.uptimeSeconds);
        display.print(F("s"));

        display.display();
    }
};

#endif // DISPLAY_MANAGER_H
