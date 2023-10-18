// class that implements the Lichess API

#include <Arduino.h>
#include <WiFiManager.h> 
#include <HTTPClient.h>

#ifndef AlfaZeroApi_h
#define AlfaZeroApi_h

class AlfaZeroApi {

    private: 
        String MAKE_A_MOVE = "https://levandrovskiy.ru/api/bot/game/{gameId}/move/{move}";

        HTTPClient http;


    public:
        String makeMove(String uid, String move); // метод для совершения хода



};

#endif