#pragma once

#include <Arduino.h>

// === LIBRARIES FOR WIFI SERVER ===
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


class NetworkConnection_Manager {

private:

    String STA_SSID;
    String STA_PASS;
    String AP_SSID;
    String AP_PASS;

    uint16_t timeout;
    

public:

    NetworkConnection_Manager(
        String sta_ssid, String sta_pass, 
        String ap_ssid, String ap_pass, 
        uint16_t timeout = 10000
    ) :
        STA_SSID(sta_ssid), 
        STA_PASS(sta_pass), 
        AP_SSID(ap_ssid), 
        AP_PASS(ap_pass), 
        timeout(timeout)
    {}

    bool connectToExternalWiFi()
    {
        Serial.println("\n--- Attempting to connect to external WiFi ---");
        Serial.print("SSID: ");
        Serial.println(STA_SSID);
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(STA_SSID, STA_PASS);

        uint32_t startTime = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout)
        {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\n✓ WiFi connected successfully!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.print("Signal strength: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
            return true;
        }
        
        Serial.println("\n✗ WiFi connection failed (timeout)");
        return false;
    }

    bool setupLocalWiFi()
    {
        Serial.println("\n--- Setting up Access Point ---");
        Serial.print("AP SSID: ");
        Serial.println(AP_SSID);
        
        WiFi.mode(WIFI_AP);
        bool success = WiFi.softAP(AP_SSID, AP_PASS);
        
        if (success) {
            Serial.println("✓ Access Point started successfully!");
            Serial.print("AP IP address: ");
            Serial.println(WiFi.softAPIP());
        } else {
            Serial.println("✗ Access Point failed to start!");
        }
        
        return success;
    }

 

    String getIP() const
    {
        if (WiFi.getMode() == WIFI_STA) {
            return WiFi.localIP().toString();
        } else {
            return WiFi.softAPIP().toString();
        }
    }

    bool isConnected() const
    {
        return WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP;
    }

    String getMode() const
    {
        wifi_mode_t mode = WiFi.getMode();
        if (mode == WIFI_STA) return "STA";
        if (mode == WIFI_AP) return "AP";
        return "OFF";
    }

 
    void printStatus() const
    {
        Serial.println("\n=== WiFi Status ===");
        Serial.print("Mode: ");
        Serial.println(getMode());
        Serial.print("IP: ");
        Serial.println(getIP());
        
        if (WiFi.getMode() == WIFI_STA) {
            Serial.print("RSSI: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
        }
        
        Serial.println("===================\n");
    }
};