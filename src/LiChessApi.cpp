#include "LiChessApi.h"

// метод создания игры 
String LiChessApi::createGameWithBot() {
    GameBotPostRequest gameBotPostRequest(5, 10800, 60, 1);
    String xd = gameBotPostRequest.toString();
    http.begin(CREATE_GAME_WITH_BOT_LINK); 
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("charset", "utf-8");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    http.addHeader("Content-Length", String(xd.length()));
    Serial.println(xd);
    int httpResponseCode = http.POST(xd); 


    String response = "";
    if(httpResponseCode>0){
        response = http.getString();  
        Serial.println(httpResponseCode);  
        Serial.println(response);         
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
        http.end();
    }

    return response;
}

String LiChessApi::cancelGameWithBot(String uid) {
    CANCEL_GAME.replace("{challengeId}", uid);
    http.begin(CANCEL_GAME); 
    Serial.println(CANCEL_GAME);
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("charset", "utf-8");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    int httpResponseCode = http.POST(""); 

    String response = "";
    if(httpResponseCode>0){
        response = http.getString();  
        Serial.println(httpResponseCode);  
        Serial.println(response);         
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
        http.end();
    }

    return response;
}

// method that sends a players move to lichess server
String LiChessApi::makeMove(String uid, String move) {
    MAKE_A_MOVE.replace("{gameId}", uid);
    MAKE_A_MOVE.replace("{move}", move);
    http.begin(MAKE_A_MOVE);
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("charset", "utf-8");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    int httpResponseCode = http.POST(""); 
    String response = "";
    if(httpResponseCode>0){
        response = http.getString();  
        Serial.println(httpResponseCode);  
        Serial.println(response);         
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
        http.end();
    }
    return response;
}

// method that gets the current state of the game
String LiChessApi::getCurrentGameState(String uid) {
    GET_CURRENT_GAME_STATE.replace("{gameId}", uid);
    http.begin(GET_CURRENT_GAME_STATE);
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("charset", "utf-8");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    int httpResponseCode = http.GET(); 
    String response = "";
    if(httpResponseCode>0){
        response = http.getString();  
        Serial.println(httpResponseCode);  
        Serial.println(response);         
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
        http.end();
    }
    return response;
}