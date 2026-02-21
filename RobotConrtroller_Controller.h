#pragma once

#include <Arduino.h>
#include <MPU6050_tockn.h>

#include "SteperMotor_Controller.h"
#include "BalancePID_Manager.h"
#include "ControlPage_Routes.h"
#include "ControlPage_WebPage.h"
#include "NetworkConnection_Manager.h"


class RobotController {

public:

    MPU6050& mpu6050;
    StepperMotor_Controller& leftMotor;
    StepperMotor_Controller& rightMotor;
    BalanceController& balanceController;
    ControlPage_Router& controlRouter;
    NetworkConnection_Manager& networkManager;

    const char* controlPage;

    static constexpr float         FALL_ANGLE      = 40.0f;
    static constexpr unsigned long PID_INTERVAL_MS = 10;  

    float maxSpeed = 50000.0f;
    float maxSteer = 2000.0f;

    unsigned long lastPidMs = 0;
    bool fallen = false;


public:

    RobotController(
        MPU6050& mpu6050,
        StepperMotor_Controller& leftMotor,
        StepperMotor_Controller& rightMotor,
        BalanceController& balanceController,
        ControlPage_Router& controlRouter,
        NetworkConnection_Manager& networkManager,
        const char* controlPage
    ) :
        mpu6050(mpu6050),
        leftMotor(leftMotor),
        rightMotor(rightMotor),
        balanceController(balanceController),
        controlRouter(controlRouter),
        networkManager(networkManager),
        controlPage(controlPage)
    {}


    void begin() {
        leftMotor.begin();
        rightMotor.begin();
        leftMotor.setMotorEnable(true);
        rightMotor.setMotorEnable(true);

        balanceController.begin();
        balanceController.setEnabled(true);

        controlRouter.setupRoutes(controlPage);

        lastPidMs = millis();
    }


    void run() {

        // Генерація кроків — ЗАВЖДИ, кожну ітерацію loop()
        leftMotor.run();
        rightMotor.run();

        if (fallen) return;

        unsigned long now = millis();
        if (now - lastPidMs < PID_INTERVAL_MS) return;
        lastPidMs = now;

        // --- MPU ---
        mpu6050.update();
        

        float pitch = mpu6050.getAngleY();

        // --- Аварійна зупинка ---
        if (abs(pitch) > FALL_ANGLE) {
            leftMotor.setSpeed(0);
            rightMotor.setSpeed(0);
            balanceController.emergencyStop();
            fallen = true;
            Serial.println("\n[!] СТОП: Робот впав!");
        }

        else {
            balanceController.setEnabled(true);
            fallen = false;
            lastPidMs = millis();
        }


        // --- Команда від веб-інтерфейсу ---
        ControlCommand cmd = controlRouter.getCommand();

        float targetSpeed = 0.0f;
        switch (cmd.direction) {
            case FORWARD:

            case FORWARD_LEFT:

            case FORWARD_RIGHT:  
                targetSpeed =  (cmd.speed / 255.0f) * maxSpeed; 
                break;

            case BACKWARD:

            case BACKWARD_LEFT:

            case BACKWARD_RIGHT: 
                targetSpeed = -(cmd.speed / 255.0f) * maxSpeed; 
                break;
            default:      

                targetSpeed =  0.0f; 
                break;
        }

        float steerOffset = map(cmd.steer, 0, 100, -maxSteer, maxSteer);

        // --- PID ---
        balanceController.setTargetSpeed(targetSpeed);
        balanceController.update(pitch);
        float baseSpeed = balanceController.getBaseSpeed();

        // --- Диференційний поворот ---
        float leftSpeed  = constrain(baseSpeed + steerOffset, -maxSpeed, maxSpeed);
        float rightSpeed = constrain(baseSpeed - steerOffset, -maxSpeed, maxSpeed);

        // --- Застосовуємо до моторів ---
        leftMotor.setDirection(leftSpeed >= 0 ? ROTATE_BACKWARD : ROTATE_FORWARD);
        leftMotor.setSpeed(abs(leftSpeed));

        rightMotor.setDirection(rightSpeed >= 0 ? ROTATE_FORWARD : ROTATE_BACKWARD);
        rightMotor.setSpeed(abs(rightSpeed));

        }

};