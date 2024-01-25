//
// Created by Viktor Hundahl Strate on 18/02/2019.
//


#pragma once

#include "compatibility.h"
#include <set>
#include <vector>
#include <algorithm>
#include "ChessPiece.h"
#include "ChessMove.h"
#include "LinkedList.h"


#define fieldToIndex(x, y) (x) + (y*8)
#define indexToX(index) (index) % 8
#define indexToY(index) (index) / 8

class ChessBoard {
public:
    ChessPiece board[64] = {
            'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',  // white
            'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',  // white
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',  // black
            'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R',  // black
    };
    // нужно учитывать то, что фигуру могут съесть
    byte blackFiguresArray[16] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
    LinkedList<byte> tempList = LinkedList<byte>();
    LinkedList<byte>& allPossibleMoves = tempList;  // TODO(me) переделать на ебучее множество
    LinkedList<byte>& kingMoves = tempList;
    LinkedList<byte> availableTempMoves;

    bool whitePlays = true;

    bool kingMoved = false;
    bool kingRookMoved = false;
    bool queenRookMoved = false;

    void printBoard() const;
    void performMove(const ChessMove& move);
    LinkedList<byte> possibleMoves(byte index) const;
    LinkedList<byte> possibleMoves(byte index, bool whitePlays) const;
    ChessBoard();
    ChessBoard(const ChessPiece board[64], const byte blackFiguresArray[16]);
    bool validMove(const ChessMove& move);
    /// Return values: 0 = game not ended; 1 = white has won; 2 = black has won
    byte gameEnded();
    std::set<byte> getAllPossibleMoves() const;
    LinkedList<byte> filterPossibleMovesForKing(LinkedList<byte> countedPossibleMoves) const;
    LinkedList<byte> getAllPossibleMovesWithKingExchange() const;
    std::vector<byte> checkIfElementInList(const byte kingAvailableMovesList, const std::set<byte> allPossibleMovesIfKingMoved) const;
    void pushToKingMoves(byte item);
};

