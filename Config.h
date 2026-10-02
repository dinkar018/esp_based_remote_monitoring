#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// SYSTEM CONFIGURATION & PIN DEFINITIONS
// =============================================================================

// --- Wi-Fi & Telegram Credentials ---
// Replace with your actual network credentials and Telegram Bot Token
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"
#define BOT_TOKEN           "YOUR_TELEGRAM_BOT_TOKEN"
// Optional: restrict access to your Telegram Chat ID (set to "" to allow any authorized chat)
#define AUTHORIZED_CHAT_ID  ""

// --- Pin Allocations ---
// I2C Pins (Hardware Default on ESP32)
#define PIN_I2C_SDA         21
#define PIN_I2C_SCL         22

// DHT11 Sensor
#define PIN_DHT             4
#define DHT_TYPE            DHT11

// HC-SR04 Ultrasonic Sensor
#define PIN_TRIG            5
#define PIN_ECHO            18

// Actuator / Control Device (Relay / LED)
#define PIN_ACTUATOR        23

// Onboard / Status Indicator LED
#define PIN_STATUS_LED      2

// --- Sensor Select ---
// Set to 1 if using BMP280, set to 0 if using BMP180 (BMP085)
#define USE_BMP280          1
#define BMP_I2C_ADDRESS     0x76  // Default for many BMP280 modules; change to 0x77 if needed

// OLED Configuration
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define OLED_RESET          -1
#define OLED_I2C_ADDRESS    0x3C

// --- Scheduling Intervals (in Milliseconds) ---
#define INTERVAL_SENSOR_READ    2000   // Read sensors every 2 seconds
#define INTERVAL_DISPLAY_UPDATE 500    // Refresh OLED twice per second
#define INTERVAL_TELEGRAM_POLL  1000   // Check Telegram messages every 1 second
#define INTERVAL_WIFI_CHECK     10000  // Verify Wi-Fi link health every 10 seconds

#endif // CONFIG_H
