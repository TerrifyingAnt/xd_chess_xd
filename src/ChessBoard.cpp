//
// Created by Viktor Hundahl Strate on 18/02/2019.
//

#include "ChessBoard.h"
#include "logger.h"

void ChessBoard::printBoard() const {
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

void ChessBoard::performMove(ChessMove move) {
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
    if (!whitePlays) {
        Serial.println("Попытка сделать ход");
        LinkedList<byte> temp = possibleAttacks(move.from, false);  // мб переделать, понять как работает
        for (int i = 0; i < temp.size(); i++) {
            attackedCells[temp.get(i)->value].erase(move.from);
        }
        // проверка, атакована ли данная клетка кем-то
        if (!attackedCells[move.from].empty()) {
            Serial.println("Попытка клеток с атаками");
            // список клеток, которые атакуют ту, с которой был сделан ход
            std::set<byte> currentCell = attackedCells[move.from];
            for (std::set<byte>::iterator i = currentCell.begin(); i != currentCell.end(); i++) {
                byte currentAttackCell = *i;
                if (board[currentAttackCell].kind() == 'b' ||
                    board[currentAttackCell].kind() == 'k' ||
                    board[currentAttackCell].kind() == 'q' ||
                    board[currentAttackCell].kind() == 'r') {
                    LinkedList<byte> tempAttackMoves = possibleAttacks(currentAttackCell, false);
                    for (int j = 0; j < tempAttackMoves.size(); j++) {
                        attackedCells[tempAttackMoves.get(j)->value].erase(currentAttackCell);
                    }
                }
            }
        }
    }
    Serial.println("Попытка обновить белого короля");
    if (whiteKing == move.from) {
        Serial.println("trying to update white king coords");
        whiteKing = move.to;
    } else {
        Serial.println("Попытка обновить черного короля");
        if (blackKing == move.from) {
            Serial.println("trying to update black king coords");
            blackKing = move.to;
        }
    }
    Serial.println("Попытка сделать ход");
    board[move.to] = board[move.from];
    board[move.from] = ' ';
    if (!whitePlays) {
        Serial.println("Попытка обновить атакованные фигуры");
        // в случае, если он считает фигуру белой, то все супер
        LinkedList<byte> temp = possibleAttacks(move.to, false);
        for (int i = 0; i < temp.size(); i++) {
            Serial.printf("Обновление атакующей клетки #%d \n", temp.get(i)->value);
            attackedCells[temp.get(i)->value].insert(move.to);
        }
        Serial.printf("Попытка обновить атакованные фигуры xd \n");
        if (!attackedCells[move.from].empty()) {
            Serial.printf("Реально зашли");
            std::set<byte> currentCell = attackedCells[move.from];
            for (std::set<byte>::iterator i = currentCell.begin(); i != currentCell.end(); i++) {
                byte currentAttackCell = *i;
                if (board[currentAttackCell].kind() != 'p' && board[currentAttackCell].kind() != 'n') {
                    LinkedList<byte> tempAttackMoves = possibleAttacks(currentAttackCell, false);
                    for (int j = 0; j < tempAttackMoves.size(); j++) {
                        Serial.printf("Попытка обновить атакованные фигуры %d \n", currentAttackCell);
                        attackedCells[tempAttackMoves.get(j)->value].insert(currentAttackCell);
                    }
                }
            }
        }
    }
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
        } else return false;
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
        if (whitePlays){
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
    return this->possibleMoves(index, this->whitePlays);
}

bool ChessBoard::validMove(const ChessMove& move) const {
    LinkedList<byte> moves = this->possibleMoves(move.from);

    return moves.contains(move.to);
}

byte ChessBoard::gameEnded() {
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

void ChessBoard::createAttackedCells() {
    for (int i = 0; i < 64; i++) {
        Serial.printf("Cell # %d \n", i);
        int y = indexToY(i);
        int x = indexToX(i);
        int dx, dy;
        Serial.printf("x: %d, y: %d \n", x, y);
        if (!board[i].whiteOwns() && !board[i].empty()) {
            if (board[i].kind() == 'p' || board[i].kind() == 'b') {
                // Serial.println(fieldToIndex(dx, dy));
                dy = y - 1;
                if (x == 0) {
                    dx = x + 1;
                    attackedCells[fieldToIndex(dx, dy)].insert(i);
                } else {
                    if (x == 7) {
                        dx = x - 1;
                        attackedCells[fieldToIndex(dx, dy)].insert(i);
                    } else {
                        dx = x + 1;
                        attackedCells[fieldToIndex(dx, dy)].insert(i);
                        dx = x - 1;
                        attackedCells[fieldToIndex(dx, dy)].insert(i);
                    }
                }
            } else {
                if (board[i].kind() == 'r') {
                    dy = y - 1;
                    attackedCells[fieldToIndex(x, dy)].insert(i);
                    if (x == 0) {
                        dx = x + 1;
                        attackedCells[fieldToIndex(dx, y)].insert(i);
                    } else {
                        if (x == 7) {
                            dx = x - 1;
                            attackedCells[fieldToIndex(dx, y)].insert(i);
                        }
                    }
                } else {
                    if (board[i].kind() == 'n') {
                        if (x == 1) {
                            dx = x + 2;
                            dy = y - 1;
                            attackedCells[fieldToIndex(dx, dy)].insert(i);
                        } else {
                            if (x == 6) {
                                dx = x - 2;
                                dy = y - 1;
                                attackedCells[fieldToIndex(dx, dy)].insert(i);
                            }
                        }
                        dx = x + 1;
                        dy = y - 2;
                        attackedCells[fieldToIndex(dx, dy)].insert(i);
                        attackedCells[fieldToIndex(dx, dy)].insert(i);
                    } else {
                        if (board[i].kind() == 'q' || board[i].kind() == 'k') {
                            dx = x + 1;
                            dy = y - 1;
                            attackedCells[fieldToIndex(dx, y)].insert(i);
                            attackedCells[fieldToIndex(dx, dy)].insert(i);
                            dx = x - 1;
                            attackedCells[fieldToIndex(dx, y)].insert(i);
                            attackedCells[fieldToIndex(dx, dy)].insert(i);
                            attackedCells[fieldToIndex(x, dy)].insert(i);
                        }
                    }
                }
            }
        }
    }
}

void ChessBoard::printAttackCells() {
    for (int i = 0; i < 64; i++) {
        Serial.printf("Current cell #%d is attacked by: ", i);
        for (auto& attackedBy : attackedCells[i]) {
            Serial.printf("%d; ", attackedBy);
    }
    Serial.println();
    }
}

LinkedList<byte> ChessBoard::possibleAttacks(byte index, bool whitePlays) const {
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
        byte yNew = y + yoffset * direction;

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
        return checkPiece.empty();
    };

    auto oppositePiece = [&](byte xoffset, byte yoffset) {
        const ChessPiece& checkPiece = getPiece(xoffset, yoffset);
        if (checkPiece.invalid()) return false;
        return !checkPiece.empty();
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
        int startPosY = whitePlays ? 1 : 6;

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
        if (whitePlays){
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
    LinkedList<byte> newResult;
    for (int i = 0; i < result.size(); i++) {
        if (result.get(i)->value < 64 || result.get(i) >= 0) {
            newResult.push(result.get(i)->value);
        }
    }
    return newResult;
}
