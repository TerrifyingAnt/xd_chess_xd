//
// Created by Viktor Hundahl Strate on 18/02/2019.
//

#include "ChessBoard.h"
#include "logger.h"

void ChessBoard::printBoard() const
{
    const ChessPiece* piece;

    for (int y = 7; y >= 0; y--) {
        if (y == 7) {
            Print("   ");
            for (int x = 0; x < 8; x++) {
                char buffer[] = {
                        static_cast<char>('a' + x), ' ', '\0'
                };
                Print(buffer);
            }
            Println("");
        }

        {
            char buffer[] = {
                    static_cast<char>('1' + y), ' ', '|', '\0'
            };
            Print(buffer);
        }

        for (int x = 0; x < 8; x++) {
            piece = &board[fieldToIndex(x, y)];

            char buffer[] = {
                    piece->key, '|', '\0'
            };
            Print(buffer);
        }

        {
            char buffer[] = {
                    ' ', static_cast<char>('1' + y), '\n', '\0'
            };
            Print(buffer);
        }

        if (y == 0) {
            Print("   ");
            for (int x = 0; x < 8; x++) {
                char buffer[] = {
                        static_cast<char>('a' + x), ' ', '\0'
                };
                Print(buffer);
            }
            Println("");
        }
    }

    Println("");
}

void ChessBoard::performMove(const ChessMove& move) {
    // castling code (move rooks for white player only)
    if (board[move.from].key == 'k' && move.from == 4 && move.to == 2) {
        board[3] = board[0];
        board[0] = ' ';
        queenRookMoved = true;
    } else {
        if (board[move.from].key == 'k' && move.from == 4 && move.to == 6) {
            board[5] = board[7];
            board[7] = ' ';
            kingRookMoved = true;
        }
    }
    if (board[move.from].key == 'k') {
        kingMoved = true;
    }
    board[move.to] = board[move.from];
    board[move.from] = ' ';
    // if (!whitePlays) {
    //     for (int i = 0; i < sizeof(blackFiguresArray); i++) {
    //         if (blackFiguresArray[i] == move.from) {
    //             blackFiguresArray[i] = move.to;
    //             break;
    //         }
    //     }
    //     // getAllPossibleMoves();
    // }
    whitePlays = !whitePlays;
}

LinkedList<byte> ChessBoard::possibleMoves(byte index, bool whitePlays) const {
    ChessPiece piece = board[index];
    char kind = piece.kind();

    LinkedList<byte> result;

    // You cannot move your opponents pieces
    if (piece.whiteOwns() != whitePlays)
        return result;

    int x = indexToX(index);
    int y = indexToY(index);

    auto offsetIndex = [&](byte xoffset, byte yoffset) {
        byte direction = piece.whiteOwns() ? 1 : -1;

        byte xNew = x + xoffset;
        byte yNew = y + yoffset*direction;

        if ((xNew > 7 || xNew < 0) || (yNew > 7 || yNew < 0))
            return -1;

        return fieldToIndex(xNew, yNew);
    };

    auto getPiece = [&](byte xoffset, byte yoffset) {
        int i = offsetIndex(xoffset, yoffset);
        if (i < 0)
            return ChessPiece::invalidPiece();

        const ChessPiece& checkPiece = board[i];
        return checkPiece;
    };

    auto oppositeOrFreePiece = [&](byte xoffset, byte yoffset) {
        const ChessPiece& checkPiece = getPiece(xoffset, yoffset);
        if (checkPiece.invalid()) return false;
        return checkPiece.empty() || checkPiece.whiteOwns() != whitePlays;
    };

    auto oppositePiece = [&](byte xoffset, byte yoffset) {
        const ChessPiece& checkPiece = getPiece(xoffset, yoffset);
        if (checkPiece.invalid()) return false;
        return !checkPiece.empty() && checkPiece.whiteOwns() != whitePlays;
    };

    auto checkPush = [&](byte xoffset, byte yoffset) {
        if (oppositeOrFreePiece(xoffset, yoffset)) {
            result.push(offsetIndex(xoffset, yoffset));
            if (oppositePiece(xoffset, yoffset)) return false;
        } else {
            return false;
        }
        return true;
    };

    if (kind == 'p') {
        if (getPiece(0, 1).empty())
            result.push(offsetIndex(0, 1));

        int startPosY = whitePlays ? 1 : 6;

        if (y == startPosY && getPiece(0, 1).empty() && getPiece(0, 2).empty())
            result.push(offsetIndex(0, 2));

        if (oppositePiece(1, 1))
            result.push(offsetIndex(1, 1));

        if (oppositePiece(-1, 1))
            result.push(offsetIndex(-1, 1));

        return result;
    }

    if (kind == 'n') {
        // i = -1 and 1
        for (int i = -1; i <= 1; i += 2) {
            for (int j = -1; j <= 1; j += 2) {
                checkPush(1*i, 2*j);
                checkPush(2*j, 1*i);
            }
        }

        return result;
    }

    if (kind == 'k') {
        if (whitePlays) {
            if (this->board[4].key == 'k' && this->board[3].empty() && this->board[2].empty() && this->board[1].empty() && this->board[0].key == 'r' && queenRookMoved == false && kingMoved == false) {
                result.push(2);
            }
            if (this->board[4].key == 'k' && this->board[5].empty() && this->board[6].empty() && this->board[7].key == 'r' && kingRookMoved == false && kingMoved == false) {
                result.push(6);
            }
        } else {
            if (this->board[60].key == 'k' && this->board[59].empty() && this->board[58].empty() && this->board[57].empty() && this->board[56].key == 'r') {
                result.push(58);
            }
            if (this->board[60].key == 'k' && this->board[61].empty() && this->board[62].empty() && this->board[63].key == 'r') {
                result.push(62);
            }
        }
    }

    bool up, down, left, right;
    up = down = left = right = true;

    bool ne, nw, se, sw;
    ne = nw = se = sw = true;

    for (int i = 1; i < 8; i++) {
        if (up)
            if (kind == 'r' || kind == 'q' || (kind == 'k' && i == 1)) up = checkPush(0, i);

        if (down)
            if (kind == 'r' || kind == 'q' || (kind == 'k' && i == 1)) down = checkPush(0, -i);

        if (right)
            if (kind == 'r' || kind == 'q' || (kind == 'k' && i == 1)) right = checkPush(i, 0);

        if (left)
            if (kind == 'r' || kind == 'q' || (kind == 'k' && i == 1)) left = checkPush(-i, 0);

        if (ne)
            if (kind == 'b' || kind == 'q' || (kind == 'k' && i == 1)) ne = checkPush(i, i);

        if (nw)
            if (kind == 'b' || kind == 'q' || (kind == 'k' && i == 1)) nw = checkPush(-i, i);

        if (se)
            if (kind == 'b' || kind == 'q' || (kind == 'k' && i == 1)) se = checkPush(i, -i);

        if (sw)
            if (kind == 'b' || kind == 'q' || (kind == 'k' && i == 1)) sw = checkPush(-i, -i);
    }

    return result;
}

LinkedList<byte> ChessBoard::possibleMoves(byte index) const {
    // TODO(me) - фильтрация для фигур, чтобы перекрывали шах
    if (board[index].kind() == 'k' && board[index].whiteOwns()) {
        return kingMoves;
        // Serial.println("got possible moves");
        // LinkedList<byte> temp = filterPossibleMovesForKing(tempPossibleMoves);
        // Serial.println("got filtered possible moves");
        // return kingMoves;
    }
    return this->possibleMoves(index, this->whitePlays);
}

bool ChessBoard::validMove(const ChessMove& move) {
    LinkedList<byte> moves = this->possibleMoves(move.from);

    return moves.contains(move.to);
}

byte ChessBoard::gameEnded()
{
    bool whiteKing = false;
    bool blackKing = false;

    for (const auto& piece : this->board) {
        if (piece.kind() == 'k') {
            if (piece.whiteOwns())
                whiteKing = true;
            else
                blackKing = true;
        }
    }

    if (whiteKing && blackKing) return 0;

    if (!whiteKing) return 1;

    return 2;
}

// TODO(me) - сейчас только для белых
std::set<byte> ChessBoard::getAllPossibleMoves() const {
    std::set<byte> blackFiguresResults;
    for (int i = 0; i < 64; i++) {
        if (!board[i].whiteOwns() && !board[i].empty()) {
            byte tempFigureField = i;
            std::vector<byte> tempResults = possibleMoves(tempFigureField, false).toList();
            for (int j = 0; j < tempResults.size(); j++) {
                blackFiguresResults.insert(tempResults[j]);
            }
        }
    }
    return blackFiguresResults;
}


LinkedList<byte> ChessBoard::filterPossibleMovesForKing(LinkedList<byte> countedPossibleMoves) const {
    LinkedList<byte> temp = LinkedList<byte>();
    for (int i = 0; i < countedPossibleMoves.size(); i++) {
        byte currentVal = countedPossibleMoves.get(i)->value;
        bool xd = allPossibleMoves.contains(currentVal);
        if (!xd) {
            temp.push(countedPossibleMoves.get(i)->value);
        }
    }
    return temp;
}

ChessBoard::ChessBoard() {
    // LinkedList<byte> temp = getAllPossibleMoves();
    // allPossibleMoves = temp;
}

LinkedList<byte> ChessBoard::getAllPossibleMovesWithKingExchange() const {
    kingMoves.clear();
    byte index = 0;
    for (int i = 0; i < 64; i++) {
        if (board[i].whiteOwns() && board[i].kind() == 'k') {
            index = i;
        }
    }
    std::vector<byte> tempList;
    LinkedList<byte> kingAvailableMoves = possibleMoves(index, true);
    kingAvailableMoves.printList();
    if (kingAvailableMoves.size() > 0) {
        std::vector<byte> kingAvailableMovesList = kingAvailableMoves.toList();
        for (int i = 0; i < kingAvailableMoves.size(); i++) {
            Serial.print("Iteration #");
            Serial.println(i + 1);
            ChessBoard newChessBoard = ChessBoard(board, blackFiguresArray);
            // TODO(me) - конвертировать индекс куда пошел король в ход
            ChessMove kingMove = ChessMove(index, kingAvailableMovesList[i]);
            newChessBoard.performMove(kingMove);
            newChessBoard.printBoard();
            // for (int j = 0; j < sizeof(blackFiguresArray); j++) {
            std::set<byte> allPossibleMovesIfKingMoved = newChessBoard.getAllPossibleMoves();
            tempList = checkIfElementInList(kingAvailableMovesList[i], allPossibleMovesIfKingMoved);
            Serial.print("    ");
            for (int i = 0; i < tempList.size(); i++) {
                const byte tempItem = tempList[i];
                // kingMoves.push(tempItem);
                Serial.print(tempList[i]);
                Serial.print(" ");
            }
            Serial.println();
            Serial.println();
        }
    }
    return availableTempMoves;
}


ChessBoard::ChessBoard(const ChessPiece *copyFromBoard, const byte *copyFromBlackFiguresArray) {
    for (int i = 0; i < 64; i++) {
        this->board[i] = copyFromBoard[i];
    }
    for (int i = 0; i < sizeof(copyFromBlackFiguresArray); i++) {
        this->blackFiguresArray[i] = copyFromBlackFiguresArray[i];
    }
}

std::vector<byte> ChessBoard::checkIfElementInList(const byte kingAvailableMove, const std::set<byte> allPossibleMovesIfKingMoved) const {
    std::vector<byte> temp;
    bool xd = true;
    std::set<byte>::iterator allPossibleMovesIfKingMovedIt = allPossibleMovesIfKingMoved.begin();
    // while (allPossibleMovesIfKingMovedIt != allPossibleMovesIfKingMoved.end()) {
    //     if (kingAvailableMove == *allPossibleMovesIfKingMovedIt) {
    //         Serial.print("  Bad king's moves: ");
    //         Serial.println(kingAvailableMove);
    //         break;
    //     }
    //     allPossibleMovesIfKingMovedIt++;
    // }
    if (std::find(allPossibleMovesIfKingMoved.begin(), allPossibleMovesIfKingMoved.end(), kingAvailableMove) == allPossibleMovesIfKingMoved.end()){
        // const byte tempVal = kingAvailableMove;
        temp.push_back(kingAvailableMove);
        Serial.print("  Cool king's moves: ");
        Serial.println(kingAvailableMove);
    }
    return temp;
}

void ChessBoard::pushToKingMoves(byte item) {
    kingMoves.push(item);
}
