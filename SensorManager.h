#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include "Config.h"
#include "SystemState.h"

#if USE_BMP280
#include <Adafruit_BMP280.h>
#else
#include <Adafruit_BMP085.h>
#endif

class SensorManager {
private:
    DHT dht;
#if USE_BMP280
    Adafruit_BMP280 bmp;
#else
    Adafruit_BMP085 bmp;
#endif
    bool bmpInitialized;

    float readUltrasonicDistance() {
        // Clear trigger pin
        digitalWrite(PIN_TRIG, LOW);
        delayMicroseconds(2);
        
        // Emit 10 microsecond HIGH pulse
        digitalWrite(PIN_TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_TRIG, LOW);
        
        // Measure echo duration with a 30ms timeout (max distance ~5 meters)
        // 30,000us timeout prevents blocking if sensor is disconnected
        unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000);
        
        if (duration == 0) {
            return -1.0f; // Timeout or sensor disconnected
        }
        
        // Speed of sound: 343 m/s = 0.0343 cm/us -> Distance = (duration * 0.0343) / 2
        float distance = (duration * 0.0343f) / 2.0f;
        
        // Valid distance bounds for HC-SR04: 2 cm to 400 cm
        if (distance < 2.0f || distance > 400.0f) {
            return -1.0f; // Out of measurable range
        }
        return distance;
    }

public:
    SensorManager() : dht(PIN_DHT, DHT_TYPE), bmpInitialized(false) {}

    void begin() {
        // Initialize DHT
        dht.begin();
        
        // Configure Ultrasonic GPIOs
        pinMode(PIN_TRIG, OUTPUT);
        pinMode(PIN_ECHO, INPUT);
        digitalWrite(PIN_TRIG, LOW);
        
        // Initialize BMP280 / BMP180 on I2C
#if USE_BMP280
        if (bmp.begin(BMP_I2C_ADDRESS)) {
            bmpInitialized = true;
            // Set up optimal sampling for weather / indoor monitoring
            bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                             Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
                             Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                             Adafruit_BMP280::FILTER_X16,      // Filtering
                             Adafruit_BMP280::STANDBY_MS_500); // Standby time
            Serial.println(F("[SensorManager] BMP280 initialized successfully."));
        } else {
            Serial.println(F("[SensorManager] WARNING: BMP280 not found at configured I2C address!"));
        }
#else
        if (bmp.begin()) {
            bmpInitialized = true;
            Serial.println(F("[SensorManager] BMP180 initialized successfully."));
        } else {
            Serial.println(F("[SensorManager] WARNING: BMP180 not found!"));
        }
#endif
    }

    void update(SystemState &state) {
        // 1. Read DHT11
        float tempDHT = dht.readTemperature();
        float humDHT = dht.readHumidity();
        
        if (isnan(tempDHT) || isnan(humDHT)) {
            state.dhtStatus = SENSOR_FAILED;
        } else {
            state.dhtStatus = SENSOR_OK;
            state.temperature = tempDHT;
            state.humidity = humDHT;
        }

        // 2. Read BMP Sensor
        if (bmpInitialized) {
            float pressureHPa = bmp.readPressure() / 100.0F; // Convert Pa to hPa
            float tempBMP = bmp.readTemperature();
            
            if (pressureHPa <= 300.0f || pressureHPa >= 1100.0f) {
                state.bmpStatus = SENSOR_DEGRADED;
            } else {
                state.bmpStatus = SENSOR_OK;
                state.pressure = pressureHPa;
                state.altitude = bmp.readAltitude(1013.25); // Standard sea-level reference
                // Fallback to BMP temperature if DHT failed
                if (state.dhtStatus != SENSOR_OK) {
                    state.temperature = tempBMP;
                }
            }
        } else {
            state.bmpStatus = SENSOR_FAILED;
        }

        // 3. Read Ultrasonic Sensor
        float dist = readUltrasonicDistance();
        if (dist < 0.0f) {
            state.ultrasonicStatus = SENSOR_FAILED;
        } else {
            state.ultrasonicStatus = SENSOR_OK;
            state.distanceCm = dist;
        }

        state.lastSampleTime = millis();
    }
};

#endif // SENSOR_MANAGER_H
