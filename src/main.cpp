#include "logger.h"
#include "ChessBoard.h"
#include "ChessMove.h"
#include "chess-minimax.h"
#include <Arduino.h>
#include <GyverMAX7219.h>
#include <WiFiManager.h> 
#include <HTTPClient.h>
#include <LiChessApi.h>


ChessBoard board;
bool whitePlays;

LiChessApi api;
ChessMove lastMove;

String stringMove = "";

String uid = "xd";

MAX7219 <2, 2, 12, 14, 13> mtrx;   // одна матрица (1х1), пин CS на D5

// Define the number of bits and create an array to store button states
const int numBits = 16; // Change this to match your specific number of bits
int buttonStates[numBits] = {1}; // Initialize all button states as not pressed (0)
int lightningButtonStates[numBits] = {1};

// Q7 pins
int dataPin1 = 34;
int dataPin2 = 35;
int dataPin3 = 25;
int dataPin4 = 33;

// CE pin 15
int clockEnablePin = 32;

// CP pin 2
int clockPin = 26;

// PL pin 1
int load = 27;

bool boolPlay = true;

void startGamePlayerComputer(int depth, int maxSteps);
int userMove(ChessBoard& board);
void computerMove(ChessBoard& board, int depth, int maxSteps);
void ledForFigures(ChessBoard& board);
void playGame();

int userMove(ChessBoard& tempBoard)
{
    if (tempBoard.whitePlays)
        Print("White");
    else
        Print("Black");

    Println(" goes next");

    Println("Perform move:");
    char buffer[5];
    Serial.readBytesUntil('\n', buffer, sizeof(buffer));

    if (strlen(buffer) == 3) {
        Print("Possible moves: ");
        ChessMove move = ChessMove(buffer);
        tempBoard.possibleMoves(move.from).printList();
        Println("");
        return 1;
    }

    if (strncmp(buffer, "exit\n", sizeof("exit\n")) == 0) return 2;

    ChessMove move(buffer);

    if (tempBoard.validMove(move)) {
        Print("Moving ");
        char name[16];
        tempBoard.board[move.from].name(name);
        Print(name);
        Print(" ");
        move.printMove();

        if (!tempBoard.board[move.to].empty()) {
          Print("Taking ");
          tempBoard.board[move.to].name(name);
          Println(name);
        }

        Println("");
        stringMove = move.getMove();
        api.makeMove(uid, stringMove);
        board.performMove(move);

        lastMove = move;
        ledForFigures(board);

        Println(stringMove);
        stringMove = "";
    } else {
        Println("Invalid move try again");
        return 1;
    }

    return 0;
}

void computerMove(ChessBoard& tempBoard)
{
  bool moveWasMade = false;
  while(!moveWasMade){
    String response = api.getCurrentGameState(uid);
    int index = response.lastIndexOf("\"moves\":") + 8;
    String moves = response.substring(response.indexOf("\"", index + 4) - 4, response.indexOf("\"", index + 4));
    char buffer[] = {
            moves[0], moves[1], ' ', moves[2], moves[3], '\0'
    };
    ChessMove lichessMove = ChessMove(buffer);
    String move1 = lichessMove.getMove();
    String move2 = lastMove.getMove();
    if(!move1.equals(move2)) {
      tempBoard.performMove(lichessMove);
      moveWasMade = true;
      lastMove = lichessMove;
      ledForFigures(board);
      break;
    }
  }
  //api.cancelGameWithBot(uid);
}

void shiftRegisters() {
  digitalWrite(load, LOW);
  digitalWrite(load, HIGH);
  
  bool dataChanged = false; // Flag to indicate if there are any changes in data
  
  Serial.print("Data from 1: ");
  for (int i = 0; i < numBits; i++) {
    int bit = digitalRead(dataPin1);
    if (bit == HIGH) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }

  Serial.println("");
  Serial.print("Data from 2: ");

  digitalWrite(load, LOW);
  digitalWrite(load, HIGH);
  for (int i = 0; i < numBits; i++) {
    int bit = digitalRead(dataPin2);
    if (bit == HIGH) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
  Serial.println("");
  Serial.print("Data from 3: ");
    digitalWrite(load, LOW);
  digitalWrite(load, HIGH);
  for (int i = 0; i < numBits; i++) {
    int bit = digitalRead(dataPin3);
    if (bit == HIGH) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
  Serial.println("");

  Serial.print("Data from 4: ");
    digitalWrite(load, LOW);
  digitalWrite(load, HIGH);
  for (int i = 0; i < numBits; i++) {
    int bit = digitalRead(dataPin4);
    if (bit == HIGH) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
        digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
    Serial.println("");
    Serial.println("");
    digitalWrite(clockPin, HIGH);
  digitalWrite(clockPin, LOW);
  delay(1000);
}


void setup() {
  mtrx.setRotation(1);
  mtrx.setFlip(true, false);
  mtrx.setType(GM_SERIES);
  delay(100);
  mtrx.setConnection(GM_LEFT_BOTTOM_UP);
  delay(100);
  mtrx.begin();       // запускаем
  delay(100);
  mtrx.setBright(7);  // яркость 0..15
    delay(100);
  mtrx.clear();
    delay(100);
  mtrx.update();

  Serial.begin(9600);
  whitePlays = true;

  Println("Moves are written like 'e2 e4'");
    mtrx.clear();

  //shiftRegistersThread.onRun(shiftRegisters);
  //shiftRegistersThread.setInterval(5);

    // Setup 74HC165 connections
  pinMode(dataPin1, INPUT);
  pinMode(dataPin2, INPUT);
  pinMode(dataPin3, INPUT);
  pinMode(dataPin4, INPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(load, OUTPUT);

  WiFi.begin("TP-Link_10DC", "37163006");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  String gameBotPostResponse = api.createGameWithBot();
  uid = gameBotPostResponse.substring(gameBotPostResponse.indexOf("id") + 5, gameBotPostResponse.indexOf("id") + 13);
  Serial.print("GAME_ID: " + uid);

}


void loop() {
  playGame();

}

void ledForFigures(ChessBoard& tempBoard) {
    Serial.print("XDXD");
    mtrx.clear();
    for(int i = 0; i < 8; i++) {
      for(int j = 0; j < 8; j++) {
        if(!tempBoard.board[i * 8 + j].empty()) {
          mtrx.rect(i * 2, j * 2, i * 2 + 1, j * 2 + 1);
        }

      }
      mtrx.update();
    }
  }

  void gerkonsToLed() {}


void playGame() {
  ledForFigures(board);
  while (boolPlay) {
      Print("Current board score: ");
      Println(ChessEngine::evaluateMoveScore(board));

      board.printBoard();

      if (!board.whitePlays) {
          computerMove(board);
      }

      int status;
      status = userMove(board);
      if (status == 1) {          
        continue;
      };
      if (status == 2) boolPlay = false;

      int endState = board.gameEnded();
      if (endState != 0) {
          if (endState == 1)
              Print("Black");
          else
              Print("White");

          Println(" has won!");
          boolPlay = false;
      }
  }

  //shiftRegisters();

}

