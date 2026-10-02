#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "Config.h"
#include "SystemState.h"
#include "ControlManager.h"

class CommunicationManager {
private:
    WiFiClientSecure securedClient;
    UniversalTelegramBot bot;
    unsigned long lastTelegramPoll;
    unsigned long lastWifiCheck;

    void sendTelegramResponse(const String &chatId, const String &message) {
        bot.sendMessage(chatId, message, "Markdown");
    }

    String formatFullStatus(const SystemState &state) {
        String msg = "*ESP32 Telemetry & System Status*\n";
        msg += "━━━━━━━━━━━━━━━━━━━━\n";

        // Temperature
        msg += "*Temperature:* ";
        if (state.dhtStatus == SENSOR_OK || state.bmpStatus == SENSOR_OK) {
            msg += String(state.temperature, 1) + " °C";
            if (state.dhtStatus != SENSOR_OK) msg += " _(BMP Fallback)_";
        } else {
            msg += "⚠️ _Sensor Fault_";
        }
        msg += "\n";

        // Humidity
        msg += "*Humidity:* ";
        if (state.dhtStatus == SENSOR_OK) {
            msg += String(state.humidity, 1) + " %";
        } else {
            msg += "⚠️ _Sensor Fault_";
        }
        msg += "\n";

        // Pressure
        msg += "*Barometric Pressure:* ";
        if (state.bmpStatus == SENSOR_OK) {
            msg += String(state.pressure, 1) + " hPa\n";
            msg += "*Altitude (est.):* " + String(state.altitude, 1) + " m\n";
        } else {
            msg += "⚠️ _Sensor Fault_\n";
        }

        // Distance / Level
        msg += "*Distance / Level:* ";
        if (state.ultrasonicStatus == SENSOR_OK) {
            msg += String(state.distanceCm, 1) + " cm\n";
        } else {
            msg += "⚠️ _Out of Range / Disconnected_\n";
        }

        msg += "━━━━━━━━━━━━━━━━━━━━\n";
        msg += "*Actuator State:* " + String(state.actuatorState ? "🟢 *ACTIVE (ON)*" : "🔴 *INACTIVE (OFF)*") + "\n";
        msg += "*Wi-Fi RSSI:* " + String(state.wifiRSSI) + " dBm\n";
        msg += "*System Uptime:* " + String(state.uptimeSeconds) + " sec\n";

        return msg;
    }

    String getHelpMessage() {
        String help = "*Available Remote Commands:*\n";
        help += "━━━━━━━━━━━━━━━━━━━━\n";
        help += "📊 `/status` - Complete telemetry snapshot\n";
        help += "🌡 `/temperature` - Current temperature\n";
        help += "💧 `/humidity` - Current humidity\n";
        help += "🧭 `/pressure` - Barometric pressure & altitude\n";
        help += "📏 `/distance` - Measured distance / level\n";
        help += "⚡ `/on` - Turn actuator / LED ON\n";
        help += "⭕ `/off` - Turn actuator / LED OFF\n";
        help += "🔄 `/toggle` - Toggle actuator state\n";
        help += "ℹ️ `/help` - Show command manual\n";
        return help;
    }

    void handleNewMessages(int numNewMessages, SystemState &state, ControlManager &control) {
        for (int i = 0; i < numNewMessages; i++) {
            String chatId = String(bot.messages[i].chat_id);
            String text = bot.messages[i].text;
            String fromName = bot.messages[i].from_name;

            // Optional security filter
            if (strlen(AUTHORIZED_CHAT_ID) > 0 && chatId != AUTHORIZED_CHAT_ID) {
                bot.sendMessage(chatId, "⛔ Unauthorized user. Access denied.", "");
                continue;
            }

            Serial.printf("[CommunicationManager] Command received: '%s' from %s\n", text.c_str(), fromName.c_str());

            if (text == "/start" || text == "/help") {
                String reply = "Welcome, *" + fromName + "*!\n\n" + getHelpMessage();
                sendTelegramResponse(chatId, reply);
            }
            else if (text == "/status") {
                sendTelegramResponse(chatId, formatFullStatus(state));
            }
            else if (text == "/temperature") {
                if (state.dhtStatus == SENSOR_OK || state.bmpStatus == SENSOR_OK) {
                    sendTelegramResponse(chatId, "🌡 *Temperature:* " + String(state.temperature, 2) + " °C");
                } else {
                    sendTelegramResponse(chatId, "⚠️ *Temperature Sensor Error*: Reading unavailable.");
                }
            }
            else if (text == "/humidity") {
                if (state.dhtStatus == SENSOR_OK) {
                    sendTelegramResponse(chatId, "💧 *Relative Humidity:* " + String(state.humidity, 1) + " %");
                } else {
                    sendTelegramResponse(chatId, "⚠️ *Humidity Sensor Error*: DHT11 not responding.");
                }
            }
            else if (text == "/pressure") {
                if (state.bmpStatus == SENSOR_OK) {
                    sendTelegramResponse(chatId, "🧭 *Pressure:* " + String(state.pressure, 2) + " hPa\n" +
                                                 "🏔 *Estimated Altitude:* " + String(state.altitude, 1) + " m");
                } else {
                    sendTelegramResponse(chatId, "⚠️ *Pressure Sensor Error*: BMP module not responding.");
                }
            }
            else if (text == "/distance") {
                if (state.ultrasonicStatus == SENSOR_OK) {
                    sendTelegramResponse(chatId, "📏 *Distance:* " + String(state.distanceCm, 1) + " cm");
                } else {
                    sendTelegramResponse(chatId, "⚠️ *Ultrasonic Error*: Target out of range or echo lost.");
                }
            }
            else if (text == "/on") {
                control.setActuator(true, state);
                sendTelegramResponse(chatId, "✅ *Command Executed:* Actuator is now *ON*.");
            }
            else if (text == "/off") {
                control.setActuator(false, state);
                sendTelegramResponse(chatId, "✅ *Command Executed:* Actuator is now *OFF*.");
            }
            else if (text == "/toggle") {
                bool newState = control.toggleActuator(state);
                sendTelegramResponse(chatId, "🔄 *Actuator Toggled:* Currently " + String(newState ? "*ON*" : "*OFF*"));
            }
            else {
                sendTelegramResponse(chatId, "❓ *Unknown Command*: '" + text + "'\nSend `/help` for valid commands.");
            }
        }
    }

public:
    CommunicationManager() : bot(BOT_TOKEN, securedClient), lastTelegramPoll(0), lastWifiCheck(0) {}

    void begin() {
        Serial.print(F("[CommunicationManager] Connecting to Wi-Fi SSID: "));
        Serial.println(WIFI_SSID);

        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        // Allow up to 10 seconds synchronous wait on first boot
        unsigned long startAttempt = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
            delay(500);
            Serial.print(F("."));
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.print(F("[CommunicationManager] Wi-Fi Connected! IP Address: "));
            Serial.println(WiFi.localIP());
        } else {
            Serial.println(F("[CommunicationManager] Wi-Fi initial connection failed. Will retry in background."));
        }

        // Configure TLS client: Set insecure to bypass self-signed / expired root CA cert sync issues
        securedClient.setInsecure();
    }

    void maintainConnection(SystemState &state) {
        unsigned long currentMillis = millis();

        if (currentMillis - lastWifiCheck >= INTERVAL_WIFI_CHECK) {
            lastWifiCheck = currentMillis;

            if (WiFi.status() == WL_CONNECTED) {
                state.wifiConnected = true;
                state.wifiRSSI = WiFi.RSSI();
            } else {
                state.wifiConnected = false;
                state.wifiRSSI = 0;
                Serial.println(F("[CommunicationManager] Wi-Fi connection lost. Reconnecting..."));
                WiFi.disconnect();
                WiFi.reconnect();
            }
        }
    }

    void processTelegram(SystemState &state, ControlManager &control) {
        if (!state.wifiConnected) return;

        unsigned long currentMillis = millis();
        if (currentMillis - lastTelegramPoll >= INTERVAL_TELEGRAM_POLL) {
            lastTelegramPoll = currentMillis;

            int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            if (numNewMessages > 0) {
                handleNewMessages(numNewMessages, state, control);
            }
        }
    }
};

#endif // COMMUNICATION_MANAGER_H
