//
// Created by Viktor Hundahl Strate on 18/02/2019.
//


#pragma once

#include "compatibility.h"
#include "ChessPiece.h"
#include "ChessMove.h"
#include "LinkedList.h"
#include <set>
#include <vector>

#define fieldToIndex(x, y) (x) + (y*8)
#define indexToX(index) (index) % 8
#define indexToY(index) (index) / 8

class ChessBoard
{
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

    bool whitePlays = true;

    bool kingMoved = false;
    bool kingRookMoved = false;
    bool queenRookMoved = false;

    std::set<byte> attackedCells[64];

    void printBoard() const;
    void performMove(const ChessMove& move);
    LinkedList<byte> possibleMoves(byte index) const;
    LinkedList<byte> possibleMoves(byte index, bool whitePlays) const;
    bool validMove(const ChessMove& move) const;

    /// Return values: 0 = game not ended; 1 = white has won; 2 = black has won
    byte gameEnded();

    void createAttackedCells();

    void printAttackCells();

    LinkedList<byte> possibleAttacks(byte index, bool whitePlays) const;
};

