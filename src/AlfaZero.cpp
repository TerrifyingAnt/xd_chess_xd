#include "AlfaZeroApi.h"

// method that sends a players move to lichess server
String AlfaZeroApi::makeMove(String uid, String move) {
    String MAKE_A_MOVE_COPY = MAKE_A_MOVE;

    MAKE_A_MOVE_COPY.replace("{gameId}", uid);
    MAKE_A_MOVE_COPY.replace("{move}", move);
    http.begin(MAKE_A_MOVE_COPY);
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
