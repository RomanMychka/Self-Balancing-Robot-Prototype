#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

enum DirectionVector {
    STOP = 0,
    FORWARD = 1,   
    BACKWARD = 2,  
    LEFT = 3,      
    RIGHT = 4,   
    FORWARD_LEFT = 5,
    FORWARD_RIGHT = 6,
    BACKWARD_LEFT = 7,
    BACKWARD_RIGHT = 8
};

struct ControlCommand {
    DirectionVector direction;
    uint8_t speed;
    uint8_t steer;
};

class ControlPage_Router {
private:
    ControlCommand command;
    AsyncWebServer* server;

public:
    ControlPage_Router(AsyncWebServer* server) : server(server) {
        command.direction = STOP;
        command.speed = 150;
        command.steer = 50;
    }

    void setupRoutes(const char *HTML) {  
        // Головна сторінка
        server->on("/", HTTP_GET, [HTML](AsyncWebServerRequest *req) {
            req->send(200, "text/html; charset=utf-8", HTML);
        });

        // Ping
        server->on("/ping", HTTP_GET, [](AsyncWebServerRequest *req) {
            String msg = "PONG, ip = ";
            msg += (WiFi.getMode() == WIFI_STA) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
            req->send(200, "text/plain", msg);
        });

        // ═══════════════════════════════════════════════════════
        //  НОВИЙ МАРШРУТ /move ДЛЯ ГЕЙМПАДУ
        //  move?v=-1..1&h=-1..1&s=0..255
        //  v:  1 = вперед, -1 = назад, 0 = стоп по вертикалі
        //  h:  1 = вправо, -1 = вліво, 0 = прямо
        //  s:  швидкість 0..255
        // ═══════════════════════════════════════════════════════
        server->on("/move", HTTP_GET, [this](AsyncWebServerRequest *req) {
            int v = 0;
            int h = 0;
            int s = command.speed;

            if (req->hasParam("v")) {
                v = req->getParam("v")->value().toInt();
            }
            if (req->hasParam("h")) {
                h = req->getParam("h")->value().toInt();
            }
            if (req->hasParam("s")) {
                s = constrain(req->getParam("s")->value().toInt(), 0, 255);
            }

            // Нормалізуємо v та h до діапазону [-1; 1]
            v = constrain(v, -1, 1);
            h = constrain(h, -1, 1);

            // Мапінг v/h → DirectionVector
            if (v == 0 && h == 0) {
                command.direction = STOP;
            } else if (v == 1 && h == 0) {
                command.direction = FORWARD;
            } else if (v == -1 && h == 0) {
                command.direction = BACKWARD;
            } else if (v == 0 && h == -1) {
                command.direction = LEFT;
            } else if (v == 0 && h == 1) {
                command.direction = RIGHT;
            } else if (v == 1 && h == -1) {
                command.direction = FORWARD_LEFT;
            } else if (v == 1 && h == 1) {
                command.direction = FORWARD_RIGHT;
            } else if (v == -1 && h == -1) {
                command.direction = BACKWARD_LEFT;
            } else if (v == -1 && h == 1) {
                command.direction = BACKWARD_RIGHT;
            } else {
                command.direction = STOP;
            }

            // Швидкість прямо з слайдера
            command.speed = s;

            // Steer: грубо мапимо h на 0..100 (0=макс. вліво, 50=прямо, 100=макс. вправо)
            uint8_t steer = 50;
            if (h < 0)      steer = 0;
            else if (h > 0) steer = 100;
            command.steer = steer;

            // Для налагодження — виводимо поточну команду в Serial
            this->printCommand();

            req->send(200, "text/plain", "OK");
        });

        // ═══════════════════════════════════════════════════════
        //  КОМБІНАЦІЇ НАПРЯМКІВ
        // ═══════════════════════════════════════════════════════
        server->on("/direction", HTTP_GET, [this](AsyncWebServerRequest *req) {
            if (req->hasParam("val")) {
                String dir = req->getParam("val")->value();
                
                // Парсинг рядка в DirectionVector
                if      (dir == "forward")        command.direction = FORWARD;
                else if (dir == "backward")       command.direction = BACKWARD;
                else if (dir == "left")           command.direction = LEFT;
                else if (dir == "right")          command.direction = RIGHT;
                else if (dir == "forward_left")   command.direction = FORWARD_LEFT;
                else if (dir == "forward_right")  command.direction = FORWARD_RIGHT;
                else if (dir == "backward_left")  command.direction = BACKWARD_LEFT;
                else if (dir == "backward_right") command.direction = BACKWARD_RIGHT;
                else                              command.direction = STOP;
            }
            req->send(200, "text/plain", "OK");
        });

        // Speed slider
        server->on("/speed", HTTP_GET, [this](AsyncWebServerRequest *req) {
            if (req->hasParam("val")) {
                command.speed = constrain(req->getParam("val")->value().toInt(), 0, 255);
            }
            req->send(200, "text/plain", "OK");
        });

        // Steer slider
        server->on("/steer", HTTP_GET, [this](AsyncWebServerRequest *req) {
            if (req->hasParam("val")) {
                command.steer = constrain(req->getParam("val")->value().toInt(), 0, 100);
            }
            req->send(200, "text/plain", "OK");
        });

        server->begin();
        Serial.println("✓ Web server started");
    }

    ControlCommand getCommand() { return command; }
    
    void resetCommand() {
        command.direction = STOP;
        command.speed = 150;
        command.steer = 50;
    }

    void printCommand() const {
        const char* dirNames[] = {
            "STOP", "FORWARD", "BACKWARD", "LEFT", "RIGHT",
            "FORWARD_LEFT", "FORWARD_RIGHT", "BACKWARD_LEFT", "BACKWARD_RIGHT"
        };
        Serial.print("Direction: ");
        Serial.print(dirNames[command.direction]);
        Serial.print(" | Speed: ");
        Serial.print(command.speed);
        Serial.print(" | Steer: ");
        Serial.println(command.steer);
    }
};