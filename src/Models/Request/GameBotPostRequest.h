#include <Arduino.h>

#include <WiFiManager.h> 
#include <HTTPClient.h>

class GameBotPostRequest {
    int level;
    int limit;
    int increment;
    int days;
    String color; 

    public: GameBotPostRequest(int level, int limit, int increment, int days) {
        this->level = level;
        this->limit = limit;
        this->increment = increment;
        this->days = days;
    }

    public: String toString() {
        Serial.print("level:" + String(level) + "\nclock.limit:" + String(limit) + " clock.increment:" + String(increment) + " color:white");
        return "level=" + String(level) + "&clock.limit=" + String(limit) + "&clock.increment=" + String(increment) + "&color=white";
    }
};