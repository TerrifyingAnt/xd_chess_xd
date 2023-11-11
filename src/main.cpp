#include "logger.h"
#include "ChessBoard.h"
#include "ChessMove.h"
#include <Arduino.h>
#include <GyverMAX7219.h>
#include <WiFiManager.h> 
#include <HTTPClient.h>
#include <LiChessApi.h>
#include <AlfaZeroApi.h>
#include <Thread.h>


ChessBoard board; // игровая доска
bool whitePlays; // играем за белых

LiChessApi liChessApi;
AlfaZeroApi alfaZeroApi;

String alfaZeroLastMove = "";

ChessMove lastMove; // последний совершенный ход в игре

String stringMove = "";

String uid = "xd";

ChessMove chessMoveFrom; // ход, который совершает пользователь

std::vector<unsigned char> figurePossibleMoves; // вектор возможных ходов

bool gerkonActualFieldData[64]; // хранится актуальное поле с герконов, то есть сюда записывается в реальном времени инфа
bool lastStateGerkonFieldData[64]; // последнее состояние доски, то есть сюда записываются прям ходы с доски

// инфа с регистров
bool gerkonFromShiftRegisters1[16];
bool gerkonFromShiftRegisters2[16];
bool gerkonFromShiftRegisters3[16];
bool gerkonFromShiftRegisters4[16];

bool oldGerkonFromShiftRegisters1[16];
bool oldGerkonFromShiftRegisters2[16];
bool oldGerkonFromShiftRegisters3[16];
bool oldGerkonFromShiftRegisters4[16];

MAX7219 <2, 2, 12, 14, 13> mtrx;   // 4 матрицы (2х2 (Width, Height)) на 74HC595, пины: CS, DATA, CLK

const int numBits = 16; // количество битов в регистре

int GAME_TYPE = 0; // 1 - альфа зиро, 0 - личесс

// Q7 пины
int dataPin1 = 34;
int dataPin2 = 35;
int dataPin3 = 25;
int dataPin4 = 33;

// CE пин 15
int clockEnablePin = 32;

// CP пин 2
int clockPin = 26;

// PL пин 1
int load = 27;

bool boolPlay = true;

void startGamePlayerComputer(int depth, int maxSteps); // не используется
int userMove(ChessBoard& board); // функция хода пользователя
void computerMove(ChessBoard& board, int depth, int maxSteps); // функция в котором происходит связь с апи
void ledForFigures(ChessBoard& board); // подсветка доски исходя из фигур, которые записаны в lastStateGerkonFieldData
void playGame(); // функция по началу игры
void generateActualFieldFromGerkons(); // создание актуального поля игры
void ledBoard(); // подсветка доски исходя из текущего состояния доски
void shiftRegisters(); // функция получения информации с герконов
void showPossibleMoves(ChessMove chessMoveFrom); // функция показания возможных ходов (не работает пока, там ошибка по индексу (index out of bound которая))
void startDemoGame(ChessMove* chessMovesList, int size); // функция, которая проигрывает игру по массиву ходов
void calibrate(); // функция калибровки доски

// функция хода пользователя
int userMove(ChessBoard& tempBoard)
{
  while(true) {
    shiftRegisters(); // получем информацию с герконов
    ledBoard();
    bool from = false;
    int fromInt = -1;
    int toInt = -1;
    // в цикле сравниваем, если последнее состояние отличается от текущего с герконов (пользователь поднял фигуру), то смотрим
    for(int i = 0; i < 8; i++) {
      for(int j = 0; j < 8; j++) {
        // первое изменение и есть место, откуда пользователь пошел
        if(gerkonActualFieldData[i * 8 + j] != lastStateGerkonFieldData[i * 8 + j] && from == false && lastStateGerkonFieldData[i * 8 + j] == true) {
          from = true;
          fromInt = i * 8 + j;
          Serial.print(" from: ");
          Serial.print(fromInt);
          Serial.println();
        }
        else {
          // второе изменение - это куда он сходил
          if(gerkonActualFieldData[i * 8 + j] != lastStateGerkonFieldData[i * 8 + j] && from == true && i * 8 + j != from) {
            toInt = i * 8 + j;
            break;
          }
        }
      }
    }
    
    // если пользователь поднял фигуру и не поставил ее
    if(toInt == -1) {
      char x1 = static_cast<char>('a' + (fromInt%8));
      char y1 = static_cast<char>('1' + (fromInt/8));
      char buffer[] = {x1, y1, '\0'};
      ChessMove bufferChessmove = ChessMove(buffer);
      chessMoveFrom = ChessMove(buffer);

      showPossibleMoves(chessMoveFrom);
      
    }
    else {
      // если все-таки поставил, то смотрим, куда поставил
      char new_x1 = static_cast<char>('a' + (fromInt%8));
      char new_y1 = static_cast<char>('1' + (fromInt/8));

      char new_x2 = static_cast<char>('a' + (toInt%8));
      char new_y2 = static_cast<char>('1' + (toInt/8));

      char buffer[] = {new_x1, new_y1, ' ', new_x2, new_y2, '\0'};
      ChessMove move(buffer);

      // проверка хода на корректность
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
          if(GAME_TYPE == 0)  {
            liChessApi.makeMove(uid, stringMove);
          }
          else {
            alfaZeroLastMove = alfaZeroApi.makeMove(uid, stringMove);
          }
          board.performMove(move);

          // запись последнего хода
          lastMove = move;
        
          for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 8; j++)
            if(gerkonActualFieldData[i * 8 + j] == false){
              lastStateGerkonFieldData[i * 8 + j] = false;
            }
            else {
              lastStateGerkonFieldData[i * 8 + j] = true;
            }
          }

          if(tempBoard.board[move.to].key == 'k') {
            if(move.to == 2 && move.from == 4) {
              gerkonActualFieldData[3] = true;
              gerkonActualFieldData[0] = false;
              lastStateGerkonFieldData[3] = true;
              lastStateGerkonFieldData[0] = false;
            }
            if(move.to == 6 && move.from == 4) {
              gerkonActualFieldData[5] = true;
              gerkonActualFieldData[7] = false;
              lastStateGerkonFieldData[5] = true;
              lastStateGerkonFieldData[7] = false;
            }
          }

          Println(stringMove);
          stringMove = "";
          return 1;
      } 
    }
  }
  return 1;
}

// функция хода личеса
void computerMove(ChessBoard& tempBoard)
{
  char buffer[5];
  bool moveWasMade = false;
  while(!moveWasMade){
    if(GAME_TYPE == 0) {
      String response = liChessApi.getCurrentGameState(uid);
      int index = response.lastIndexOf("\"moves\":") + 8;
      String moves = response.substring(response.indexOf("\"", index + 4) - 4, response.indexOf("\"", index + 4));
      buffer[0] = moves[0];
      buffer[1] = moves[1];
      buffer[2] = ' ';
      buffer[3] = moves[2];
      buffer[4] = moves[3];
      buffer[5] = '\0';
    }
    else {
      buffer[0] = alfaZeroLastMove[0];
      buffer[1] = alfaZeroLastMove[1];
      buffer[2] = ' ';
      buffer[3] = alfaZeroLastMove[2];
      buffer[4] = alfaZeroLastMove[3];
      buffer[5] = '\0';
    }
    ChessMove lichessMove = ChessMove(buffer);
    String move1 = lichessMove.getMove();
    String move2 = lastMove.getMove();
    if(!move1.equals(move2)) {
      tempBoard.performMove(lichessMove);
      moveWasMade = true;
      lastMove = lichessMove;
      ledForFigures(board);
      lastStateGerkonFieldData[lichessMove.from] = false;
      lastStateGerkonFieldData[lichessMove.to] = true;

      if(lichessMove.to == 30 && lichessMove.from == 28 && tempBoard.board[lichessMove.to].key == 'k') {
        gerkonActualFieldData[31] = true;
        gerkonActualFieldData[28] = false;
        lastStateGerkonFieldData[31] = true;
        lastStateGerkonFieldData[28] = false;
      }
      else {
        if(lichessMove.to == 26 && lichessMove.from == 28 && tempBoard.board[lichessMove.to].key == 'k') {
          gerkonActualFieldData[27] = true;
          gerkonActualFieldData[24] = false;
          lastStateGerkonFieldData[27] = true;
          lastStateGerkonFieldData[24] = false;
        }
      }
      break;
    }
  }
  //liChessApi.cancelGameWithBot(uid);
}

// функция чтения информации с герконов
void shiftRegisters() {

  digitalWrite(load, LOW);
  digitalWrite(load, HIGH);

  
  for (int i = 0; i < 16; i++) {
    oldGerkonFromShiftRegisters1[i] = gerkonFromShiftRegisters1[i];
    oldGerkonFromShiftRegisters2[i] = gerkonFromShiftRegisters2[i];
    oldGerkonFromShiftRegisters3[i] = gerkonFromShiftRegisters3[i];
    oldGerkonFromShiftRegisters4[i] = gerkonFromShiftRegisters4[i];
  }

  Serial.print("\n\nData from 1: ");

  for (int i = 0; i < numBits; i++) {
    int bit = digitalRead(dataPin1);
    if (bit == HIGH) {
      gerkonFromShiftRegisters1[i] = true;
      Serial.print("1");
    } else {
      Serial.print("0");
      gerkonFromShiftRegisters1[i] = false;
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
      gerkonFromShiftRegisters2[i] = true;

    } else {
      Serial.print("0");
      gerkonFromShiftRegisters2[i] = false;
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
      gerkonFromShiftRegisters3[i] = true;
    } else {
      Serial.print("0");
      gerkonFromShiftRegisters3[i] = false;
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
      gerkonFromShiftRegisters4[i] = true;
    } else {
      Serial.print("0");
      gerkonFromShiftRegisters4[i] = false;
    }
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }

  generateActualFieldFromGerkons();   
  delay(100);
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

  mtrx.clear();

  // Setup 74HC165 connections
  pinMode(dataPin1, INPUT);
  pinMode(dataPin2, INPUT);
  pinMode(dataPin3, INPUT);
  pinMode(dataPin4, INPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(load, OUTPUT);

  calibrate();
  //shiftRegisters();
  for(int i = 0; i < 64; i++) {
    lastStateGerkonFieldData[i] = gerkonActualFieldData[i];
  }


  WiFi.begin("JG", "J7abcak47");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  if(GAME_TYPE == 0){
    String gameBotPostResponse = liChessApi.createGameWithBot();
    uid = gameBotPostResponse.substring(gameBotPostResponse.indexOf("id") + 5, gameBotPostResponse.indexOf("id") + 13);
  }
  else {
    String gameBotPostResponse = alfaZeroApi.makeMove(uid, "0000");
  }
  Serial.print("GAME_ID: " + uid);

  ledForFigures(board);
}


void loop() {
  playGame();
}

void ledForFigures(ChessBoard& tempBoard) {
  mtrx.clear();
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 8; j++) {
      if(!tempBoard.board[i * 8 + j].empty()) {
        mtrx.rect(i * 2, j * 2, i * 2 + 1, j * 2 + 1);
      }
    }
  }
  mtrx.update();
}

// основная функция игры
void playGame() {
  ledBoard();
  while (boolPlay) {

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

}

// функция получает акутальные данные о положении фигур на доске
void generateActualFieldFromGerkons() {
  // a8 - h8
  gerkonActualFieldData[24] = gerkonFromShiftRegisters4[7];
  gerkonActualFieldData[25] = gerkonFromShiftRegisters4[3];
  gerkonActualFieldData[26] = gerkonFromShiftRegisters4[15];
  gerkonActualFieldData[27] = gerkonFromShiftRegisters4[11];

  gerkonActualFieldData[28] = gerkonFromShiftRegisters1[7];
  gerkonActualFieldData[29] = gerkonFromShiftRegisters1[3];
  gerkonActualFieldData[30] = gerkonFromShiftRegisters1[15];
  gerkonActualFieldData[31] = gerkonFromShiftRegisters1[11];

  // a7 - h7
  gerkonActualFieldData[16] = gerkonFromShiftRegisters4[6];
  gerkonActualFieldData[17] = gerkonFromShiftRegisters4[2];
  gerkonActualFieldData[18] = gerkonFromShiftRegisters4[14];
  gerkonActualFieldData[19] = gerkonFromShiftRegisters4[10];

  gerkonActualFieldData[20] = gerkonFromShiftRegisters1[6];
  gerkonActualFieldData[21] = gerkonFromShiftRegisters1[2];
  gerkonActualFieldData[22] = gerkonFromShiftRegisters1[14];
  gerkonActualFieldData[23] = gerkonFromShiftRegisters1[10];

  // a6 - h6
  gerkonActualFieldData[8] = gerkonFromShiftRegisters4[5];
  gerkonActualFieldData[9] = gerkonFromShiftRegisters4[1];
  gerkonActualFieldData[10] = gerkonFromShiftRegisters4[13];
  gerkonActualFieldData[11] = gerkonFromShiftRegisters4[9];

  gerkonActualFieldData[12] = gerkonFromShiftRegisters1[5];
  gerkonActualFieldData[13] = gerkonFromShiftRegisters1[1];
  gerkonActualFieldData[14] = gerkonFromShiftRegisters1[13];
  gerkonActualFieldData[15] = gerkonFromShiftRegisters1[9];

  // a5 - h5
  gerkonActualFieldData[0] = gerkonFromShiftRegisters4[4];
  gerkonActualFieldData[1] = gerkonFromShiftRegisters4[0];
  gerkonActualFieldData[2] = gerkonFromShiftRegisters4[12];
  gerkonActualFieldData[3] = gerkonFromShiftRegisters4[8];

  gerkonActualFieldData[4] = gerkonFromShiftRegisters1[4];
  gerkonActualFieldData[5] = gerkonFromShiftRegisters1[0];
  gerkonActualFieldData[6] = gerkonFromShiftRegisters1[12];
  gerkonActualFieldData[7] = gerkonFromShiftRegisters1[8];

  // a4 - h4
  gerkonActualFieldData[56] = gerkonFromShiftRegisters3[7];
  gerkonActualFieldData[57] = gerkonFromShiftRegisters3[3];
  gerkonActualFieldData[58] = gerkonFromShiftRegisters3[15];
  gerkonActualFieldData[59] = gerkonFromShiftRegisters3[11];

  gerkonActualFieldData[60] = gerkonFromShiftRegisters2[7];
  gerkonActualFieldData[61] = gerkonFromShiftRegisters2[3];
  gerkonActualFieldData[62] = gerkonFromShiftRegisters2[15];
  gerkonActualFieldData[63] = gerkonFromShiftRegisters2[11];

  // a3 - h3
  gerkonActualFieldData[48] = gerkonFromShiftRegisters3[6];
  gerkonActualFieldData[49] = gerkonFromShiftRegisters3[2];
  gerkonActualFieldData[50] = gerkonFromShiftRegisters3[14];
  gerkonActualFieldData[51] = gerkonFromShiftRegisters3[10];

  gerkonActualFieldData[52] = gerkonFromShiftRegisters2[6]; 
  gerkonActualFieldData[53] = gerkonFromShiftRegisters2[2];
  gerkonActualFieldData[54] = gerkonFromShiftRegisters2[14];
  gerkonActualFieldData[55] = gerkonFromShiftRegisters2[10];

  // a2 - h2
  gerkonActualFieldData[40] = gerkonFromShiftRegisters3[5];
  gerkonActualFieldData[41] = gerkonFromShiftRegisters3[1];
  gerkonActualFieldData[42] = gerkonFromShiftRegisters3[13];
  gerkonActualFieldData[43] = gerkonFromShiftRegisters3[9];

  gerkonActualFieldData[44] = gerkonFromShiftRegisters2[5];
  gerkonActualFieldData[45] = gerkonFromShiftRegisters2[1];
  gerkonActualFieldData[46] = gerkonFromShiftRegisters2[13];
  gerkonActualFieldData[47] = gerkonFromShiftRegisters2[9];

  // a1 - h1
  gerkonActualFieldData[32] = gerkonFromShiftRegisters3[4];
  gerkonActualFieldData[33] = gerkonFromShiftRegisters3[0];
  gerkonActualFieldData[34] = gerkonFromShiftRegisters3[12];
  gerkonActualFieldData[35] = gerkonFromShiftRegisters3[8];

  gerkonActualFieldData[36] = gerkonFromShiftRegisters2[4];
  gerkonActualFieldData[37] = gerkonFromShiftRegisters2[0];
  gerkonActualFieldData[38] = gerkonFromShiftRegisters2[12];
  gerkonActualFieldData[39] = gerkonFromShiftRegisters2[8];
}

// функция вывода подсветки доски
void ledBoard() {
  mtrx.clear();
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 8; j++) {
      if(gerkonActualFieldData[i * 8 + j] && board.board[i * 8 + j].whiteOwns()) {
        mtrx.rect(i * 2, j * 2, i * 2 + 1, j * 2 + 1);
      }
      else {
        if(board.board[i * 8 + j].whiteOwns()){
          mtrx.dot(i * 2 + 1, j * 2);
          mtrx.dot(i * 2, j * 2 + 1);
        }
      }
      if(!board.board[i * 8 + j].whiteOwns() && !board.board[i * 8 + j].empty() && gerkonActualFieldData[i * 8 + j]) {
        mtrx.rect(i * 2, j * 2, i * 2 + 1, j * 2 + 1);
      }
      if(!board.board[i * 8 + j].whiteOwns() && !board.board[i * 8 + j].empty()) {
        mtrx.dot(i * 2, j * 2);
        mtrx.dot(i * 2 + 1, j * 2 + 1);
      }
    }
    mtrx.update();
  }
}

// функция для показа возможных ходов
void showPossibleMoves(ChessMove chessMoveFrom) {
  int state = 0;
  if(chessMoveFrom.from != 0){
    figurePossibleMoves = board.possibleMoves(chessMoveFrom.from).toList();
    int possibleMovesCount = figurePossibleMoves.size(); 
    if(possibleMovesCount > 0) {  
      for(int j = 0; j < 4; j++) {
        for (int i = 0; i < possibleMovesCount; i++){
          Serial.println(figurePossibleMoves[i]);
          for(int k = 0; k < 8; k++) {
            for(int l = 0; l < 8; l++) {
              if(figurePossibleMoves[i] == k * 8 + l) {
                mtrx.rect(k * 2, l * 2, k * 2 + 1, l * 2 + 1, 0);
                switch(j) {
                  case 0:
                    mtrx.dot(k * 2 + j, l * 2 + j);
                    break;
                  case 1:
                    mtrx.dot(k * 2 + j - 1, l * 2 + j);
                    break;
                  case 2:
                    mtrx.dot(k * 2 + j - 1, l * 2 + j - 1);
                    break;
                  case 3:
                    mtrx.dot(k * 2 + 1, l * 2 + j - 3);
                    break;
                  }
              }
            }
          }
        }
        mtrx.update();
        if(j == 3){
          delay(400); // задержка для считывания регистров == 100, таким образом не будет задержек при прокручивании анимации возможного хода
        }
        else {
          delay(500);
        }
      }
    }
    state = 0;
  }
}

// функция калибровки доски
void calibrate() {
  bool isCalibrated = false;
  while(!isCalibrated) {
    int count = 0;
    shiftRegisters();
    ledBoard();
    for(int i = 0; i < 64; i++) {
      if(gerkonActualFieldData[i] == true) {
        count++;
      }
    }
    // for(int i = 0; i < 16; i++) {
    //   if(gerkonActualFieldData[i] == false) {
    //     count++;
    //   }
    // }
    // for(int i = 48; i < 64; i++) {
    //   if(gerkonActualFieldData[i] == false) {
    //     count++;
    //   }
    // }
    
    if(count >= 30) {
      isCalibrated = true;
      break;
    }
    else {
      isCalibrated = false;
    }
  }
}








