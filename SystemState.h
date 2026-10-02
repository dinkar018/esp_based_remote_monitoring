#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

// Health and availability states for attached sensors
enum SensorStatus {
    SENSOR_OK = 0,
    SENSOR_DEGRADED,
    SENSOR_FAILED
};

// Unified System Telemetry & Control Data Model
struct SystemState {
    // Environmental telemetry
    float temperature;       // In Celsius (from DHT11 or BMP280 fallback)
    float humidity;          // In % (from DHT11)
    float pressure;          // In hPa (from BMP280 / BMP180)
    float altitude;          // In meters (calculated approximate)
    
    // Distance / Proximity telemetry
    float distanceCm;        // In centimeters (from HC-SR04)
    
    // Sensor diagnostic health flags
    SensorStatus dhtStatus;
    SensorStatus bmpStatus;
    SensorStatus ultrasonicStatus;
    
    // Actuator & device state
    bool actuatorState;      // true = ON / active, false = OFF / safe
    
    // Network & system telemetry
    bool wifiConnected;
    int wifiRSSI;            // Signal strength in dBm
    uint32_t uptimeSeconds;  // System uptime counter
    uint32_t lastSampleTime; // Timestamp of latest sensor sample (millis)
};

#endif // SYSTEM_STATE_H
