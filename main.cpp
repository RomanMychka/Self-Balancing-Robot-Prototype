#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>


#include "ControlPage_WebPage.h"
#include "ControlPage_Routes.h"
#include "NetworkConnection_Manager.h"


#include "SteperMotor_Controller.h"

#include "BalancePID_Manager.h"
#include "RobotConrtroller_Controller.h"

// =========================================================
//  ПІНИ
// =========================================================

#define LEFT_STEP_PIN    33
#define LEFT_DIR_PIN     14
#define LEFT_ENABLE_PIN  26

#define RIGHT_STEP_PIN   32
#define RIGHT_DIR_PIN    25
#define RIGHT_ENABLE_PIN 27

#define SDA_PIN 21
#define SCL_PIN 22

// =========================================================
//  WiFi налаштування
// =========================================================

#define WIFI_STA_SSID  "Oa_Inet_Free"
#define WIFI_STA_PASS  "iloveoa!"
#define WIFI_AP_SSID   "BalanceBot"
#define WIFI_AP_PASS   "12345678"

// =========================================================
//  Об'єкти
// =========================================================

AsyncWebServer server(80);

MPU6050                    mpu6050(Wire);
StepperMotor_Controller    leftMotor(LEFT_STEP_PIN,  LEFT_DIR_PIN,  LEFT_ENABLE_PIN,  0);
StepperMotor_Controller    rightMotor(RIGHT_STEP_PIN, RIGHT_DIR_PIN, RIGHT_ENABLE_PIN, 1);
BalanceController          balance;
NetworkConnection_Manager  network(WIFI_STA_SSID, WIFI_STA_PASS, WIFI_AP_SSID, WIFI_AP_PASS);
ControlPage_Router         router(&server);

RobotController robot(
    mpu6050,
    leftMotor,
    rightMotor,
    balance,
    router,
    network,
    controlPageHTML
);


// =========================================================
//  SETUP
// =========================================================

void setup() {
    Serial.begin(115200);
    
    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("\n=== BALANCE BOT STARTUP (FIXED) ===");

    // 1. MPU6050
    Serial.println("📡 Ініціалізація MPU6050...");
    mpu6050.begin();
    mpu6050.setGyroOffsets(1.0f, 0.0f, -1.0f);
    
    
    balance.setInnerPID(600.0f, 5000.0f, 15.0f); 
    balance.setOuterPID(3.0f);
    
    balance.setBalanceOffset(0.0f);

    // 2. WiFi
    network.setupLocalWiFi();
    network.printStatus();

    // 3. Решта
    robot.begin();
    
}

// =========================================================
//  LOOP
// =========================================================

void loop() {
    robot.run();
}