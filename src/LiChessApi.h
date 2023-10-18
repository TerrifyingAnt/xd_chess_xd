// class that implements the Lichess API

#include <Arduino.h>
#include <WiFiManager.h> 
#include <HTTPClient.h>
#include "Models/Request/GameBotPostRequest.h"
#include "Models/Response/GameBotPostResponse.h"
#include "Models/Response/CancelGameResponse.h"

#ifndef LichessApi_h
#define LichessApi_h

class LiChessApi {

    private: 
        String CREATE_GAME_WITH_BOT_LINK = "https://lichess.org/api/challenge/ai";
        String CANCEL_GAME = "https://lichess.org/api/challenge/{challengeId}/cancel";
        String ACCOUNT_LINK = "https://lichess.org/api/account"; 
        String MAKE_A_MOVE = "https://lichess.org/api/bot/game/{gameId}/move/{move}";
        String GET_CURRENT_GAME_STATE = "https://lichess.org/api/bot/game/stream/{gameId}";

        HTTPClient http;

        String token = /*"lip_IidE4DDeLZ84Oiv4DKyd"*/ "lip_24oJJxlBlYlACB4y21Zc";

    // TODO
        String CREATE_GAME_WITH_PLAYER_LINK = "https://lichess.org/api/account";

    public:
        void getAccount(); // метод для получения информации об аккаунте

        String createGameWithBot(); // метод дял создания игры с ботом

        String cancelGameWithBot(String uid); // метод для отмены игры с ботом

        String makeMove(String uid, String move); // метод для совершения хода

        String getCurrentGameState(String uid); // метод для получения хода


};

#endif