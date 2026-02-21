#pragma once
#include <Arduino.h>



class BalanceController {

private:

    // === Коефіцієнти внутрішнього PID ===
    float Kp_inner;  
    float Ki_inner;   
    float Kd_inner;   

    // === Змінні внутрішнього PID ===
    float lastAngleError;  
    float angleErrorSum;   
    float targetAngle;    

    // === Зовнішній контур ===
    float Kp_outer;        
    float targetSpeed;     
    float estimatedSpeed;  

    // === Вихід ===
    float baseSpeed;    

    // === Параметри ===
    float balanceOffset;   
    float maxSpeed;      

    // === Таймінги ===
    unsigned long lastInnerMs;
    unsigned long lastOuterMs;

    bool enabled;

    // === Зовнішній контур: targetSpeed -> targetAngle ===
    void runOuterLoop() {
        unsigned long now = millis();
        if (now - lastOuterMs < 100) return;  
        lastOuterMs = now;
        
        float speedError = (targetSpeed - estimatedSpeed);
        
        float normalizedError = speedError / maxSpeed;
        targetAngle = constrain(Kp_outer * normalizedError, -7.0f, 7.0f);
    }

    // === Внутрішній контур: currentAngle -> baseSpeed ===
    void runInnerLoop(float currentAngle) {
        unsigned long now = millis();
        float dt = (now - lastInnerMs) / 1000.0f;

        lastInnerMs = now;

        float adjustedAngle = currentAngle - balanceOffset;
        float error = targetAngle - adjustedAngle;

        float P = Kp_inner * error;

        angleErrorSum += error * dt;
        
        angleErrorSum = constrain(angleErrorSum, -500.0f, 500.0f);

        float I = Ki_inner * angleErrorSum;

        float D = 0.0f;
        if (dt > 0.0f) {
            D = Kd_inner * (error - lastAngleError) / dt;
        }
        lastAngleError = error;


        baseSpeed = constrain(P + I + D, -maxSpeed, maxSpeed);
    }

    // === Оцінка швидкості (без енкодерів) ===
    void updateSpeedEstimate(float dt) {
        
        float alpha = 0.1f;  
        
        estimatedSpeed = alpha * baseSpeed + (1.0f - alpha) * estimatedSpeed;
        estimatedSpeed = constrain(estimatedSpeed, -maxSpeed, maxSpeed);
    }

public:

    // === Конструктор ===
    BalanceController()
        : Kp_inner(200.0f), Ki_inner(5.0f), Kd_inner(8.0f)
        , Kp_outer(5.0f)
        , lastAngleError(0), angleErrorSum(0), targetAngle(0)
        , targetSpeed(0), estimatedSpeed(0)
        , baseSpeed(0)
        , balanceOffset(0), maxSpeed(15000.0f)
        , lastInnerMs(0), lastOuterMs(0)
        , enabled(false)
    {}

    // === Ініціалізація ===
    void begin() {
        lastInnerMs = millis();
        lastOuterMs = millis();
    }


    void update(float currentAngle) {
        if (!enabled) { 
            baseSpeed = 0;
            estimatedSpeed = 0;
            return;
        }

        unsigned long now = millis();
        float dt = (now - lastInnerMs) / 1000.0f;

        runOuterLoop();
        runInnerLoop(currentAngle);
        updateSpeedEstimate(dt);
    }

    void setTargetSpeed(float speed) {
        targetSpeed = constrain(speed, -maxSpeed, maxSpeed);
    }

    void setEnabled(bool en) {
        enabled = en;
        if (!en) {
            angleErrorSum  = 0;
            lastAngleError = 0;
            estimatedSpeed = 0;
            baseSpeed      = 0;
        }
    }

    void emergencyStop() {
        setEnabled(false);
        targetSpeed = 0;
    }

    void setInnerPID(float Kp, float Ki, float Kd) {
        Kp_inner = Kp; Ki_inner = Ki; Kd_inner = Kd;
        angleErrorSum = 0; 
    }
    void setOuterPID(float Kp)        { Kp_outer = Kp; }
    void setBalanceOffset(float off)  { balanceOffset = off; }
    void setMaxSpeed(float spd)       { maxSpeed = spd; }

    // === Вихідні дані для RobotController ===
    float getBaseSpeed()     const { return baseSpeed; }
    float getTargetAngle()   const { return targetAngle; }
    float getEstimatedSpeed()const { return estimatedSpeed; }
    bool  isEnabled()        const { return enabled; }

};