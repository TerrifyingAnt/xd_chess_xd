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

String uid = "xd";

int mode = 0;
int depth = 2;
int maxSteps = 10;

MAX7219 <2, 2, 12, 14, 13> mtrx;   // одна матрица (1х1), пин CS на D5

bool boolPlay = true;

void startGamePlayerComputer(int depth, int maxSteps);
int userMove(ChessBoard& board);
void computerMove(ChessBoard& board, int depth, int maxSteps);
void ledForFigures(ChessBoard& board);
void playGame();

int userMove(ChessBoard& board)
{
    if (board.whitePlays)
        Print("White");
    else
        Print("Black");

    Println(" goes next");

    Println("Perform move:");
    char buffer[12];
    Serial.readBytesUntil('\n', buffer, sizeof(buffer));

    if (strlen(buffer) == 3) {
        Print("Possible moves: ");
        ChessMove move = ChessMove(buffer);
        board.possibleMoves(move.from).printList();
        Println("");
        return 1;
    }

    if (strncmp(buffer, "exit\n", sizeof("exit\n")) == 0) return 2;

    ChessMove move(buffer);

    if (board.validMove(move)) {
        Print("Moving ");
        char name[16];
        board.board[move.from].name(name);
        Print(name);
        Print(" ");
        move.printMove();

        if (!board.board[move.to].empty()) {
          Print("Taking ");
          board.board[move.to].name(name);
          Println(name);
        }

        Println("");
        String stringMove = move.getMove();
        api.makeMove(uid, stringMove);
        board.performMove(move);

        //lastMove = move;
        //ledForFigures(board);

        Println(stringMove);
        ledForFigures(board);
    } else {
        Println("Invalid move try again");
        return 1;
    }

    return 0;
}

void computerMove()
{
  String response = api.getCurrentGameState(uid);
  int index = response.lastIndexOf("\"moves\":") + 8;
  String moves = response.substring(response.indexOf("\"", index + 4) - 4, response.indexOf("\"", index + 4));
  Println(moves);
  char* createMove = new char[5];
  for(int i = 0; i < moves.length(); i++) {
      if(i != 2) {
        createMove[i] = moves[i];
      }
    else {
      createMove[i] = ' ';
    }
  }
  ChessMove lichessMove = ChessMove(createMove);
  board.performMove(createMove);
  api.cancelGameWithBot(uid);
}

void setup() {
  mtrx.setRotation(1);
  mtrx.setType(GM_SERIES);
  delay(100);
  mtrx.setConnection(GM_LEFT_TOP_DOWN);
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

  WiFi.begin("IoT_Case", "qweqweqwe");
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

void ledForFigures(ChessBoard& board) {
    Serial.print("XDXD");
    for(int i = 0; i < 8; i++) {
      for(int j = 0; j < 8; j++) {
        if(!board.board[i * 8 + j].empty()) {
          mtrx.rect(i * 2, j * 2, i * 2 + 1, j * 2 + 1);
        }

      }
      mtrx.update();
    }
  }

void playGame() {
  while (boolPlay) {
      Print("Current board score: ");
      Println(ChessEngine::evaluateMoveScore(board));

      board.printBoard();

      if (!board.whitePlays) {
          computerMove();
          board.whitePlays = !board.whitePlays;
          //update = true;
          continue;
      }

      int status;
      status = userMove(board);
      if (status == 1) continue;
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
}

