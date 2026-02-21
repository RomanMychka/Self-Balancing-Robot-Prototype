#pragma once

#include <Arduino.h>

enum Direction {
    ROTATE_FORWARD = HIGH,   
    ROTATE_BACKWARD = LOW    
};

class StepperMotor_Controller {

private:

    uint8_t stepPin;        
    uint8_t directionPin;  
    uint8_t enablePin;      
    uint8_t ledcChannel;

    bool isMotorEnabled;       
    Direction currentDirection; 
    float currentSpeed;
    
    volatile long currentPosition;

    unsigned long lastStepTimeMicros;  
    uint32_t stepIntervalMicros;
    
    bool pulseState;
    unsigned long pulseStartMicros;
    static constexpr unsigned long PULSE_WIDTH_MICROS = 2;  
    
    static constexpr float MIN_SPEED = 200.0f;  
    
    void generateStep() {
        if (stepIntervalMicros == 0 || !isMotorEnabled) {
            pulseState = false;
            return;
        }

        unsigned long currentTime = micros();
        
        if (pulseState) {
            if (currentTime - pulseStartMicros >= PULSE_WIDTH_MICROS) {
                digitalWrite(stepPin, LOW);
                pulseState = false;

                if (currentDirection == ROTATE_FORWARD) {
                    currentPosition++;
                } else {
                    currentPosition--;
                }
            }
            return;
        }
        
        // Перевірка overflow
        unsigned long elapsed;
        if (currentTime >= lastStepTimeMicros) {
            elapsed = currentTime - lastStepTimeMicros;
        } else {
            elapsed = (0xFFFFFFFF - lastStepTimeMicros) + currentTime + 1;
        }


        if (elapsed >= stepIntervalMicros) {
            digitalWrite(stepPin, HIGH);
            pulseState = true;
            pulseStartMicros = currentTime;
            lastStepTimeMicros = currentTime;
        }
    }

    void updateStepInterval() {
        if (currentSpeed > 0) {
            stepIntervalMicros = (uint32_t)(1000000.0 / currentSpeed);
        } else {
            stepIntervalMicros = 0;
        }
    }

public:

    StepperMotor_Controller(
        uint8_t step_pin,
        uint8_t direction_pin,
        uint8_t enable_pin,
        uint8_t ledc_channel
    ) :
        stepPin(step_pin),
        directionPin(direction_pin),
        enablePin(enable_pin),
        ledcChannel(ledc_channel),
        isMotorEnabled(false),
        currentDirection(ROTATE_FORWARD),
        currentSpeed(0),
        currentPosition(0),
        lastStepTimeMicros(0),
        stepIntervalMicros(0),
        pulseState(false),
        pulseStartMicros(0)
    {}

    void begin() {
        pinMode(stepPin, OUTPUT);
        pinMode(directionPin, OUTPUT);
        pinMode(enablePin, OUTPUT);

        digitalWrite(stepPin, LOW);
        digitalWrite(directionPin, ROTATE_FORWARD);
        digitalWrite(enablePin, HIGH);
        
        isMotorEnabled = false;
    }

    void setMotorEnable(bool enable) {
        digitalWrite(enablePin, enable ? LOW : HIGH);
        isMotorEnabled = enable;
    }

    bool isEnabled() const {
        return isMotorEnabled;
    }

    void setSpeed(float speed) {
        currentSpeed = abs(speed);
        
        // Застосовуємо мертву зону
        if (currentSpeed > 0 && currentSpeed < MIN_SPEED) {
            currentSpeed = MIN_SPEED;
        }
        
        if (currentSpeed > 50000) {
            currentSpeed = 50000;
        }
        
        updateStepInterval();
    }

    void setDirection(Direction direction) {
        currentDirection = direction;
        digitalWrite(directionPin, direction);
    }

    void run() {
        generateStep();
    }

    float getSpeed() const {
        return currentSpeed;
    }

    Direction getDirection() const {
        return currentDirection;
    }

    long getPosition() const {
        return currentPosition;
    }

    void resetPosition() {
        currentPosition = 0;
    }

    bool isRunning() const {
        return currentSpeed > 0;
    }

};