//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#include <iostream>
#include <string>
#include <stdio.h>
#include "Board.h"

void Board::_copyFrom(const Board &board) {
    for (int row = 0; row < NRows; row++) {
        for (int col = 0; col < NCols; col++) {
            _cells[row][col] = board._cells[row][col];
        }
    }
    _emptyPositions = board._emptyPositions;
}

void Board::_clear() {
    _emptyPositions.clear();
    _emptyPositions.reserve(NRows * NCols);
    for (int row = 0; row < NRows; row++) {
        for (int col = 0; col < NCols; col++) {
            _cells[row][col] = Seed::Empty;
            _emptyPositions.insert(Pos(row, col));
        }
    }
}

Board::Board() {
    _clear();
}

Board::Board(const Board &board) {
    _copyFrom(board);
}

Board &Board::operator=(const Board &board) {
    _copyFrom(board);
    return *this;
}

void Board::setSeedAtPosition(const Pos &pos, const Seed &seed) {
    const Seed &currentSeed = _cells[pos.getRow()][pos.getCol()];
    if (currentSeed.getValue() != Seed::Value::Empty) {
        std::cout <<
                  "Position ("
                  << pos.getRow()
                  << ","
                  << pos.getCol()
                  << ") is used already!\t";
        return;
    }
    if (seed.getValue() == Seed::Value::Empty) return;
    _cells[pos.getRow()][pos.getCol()] = seed;
    _emptyPositions.erase(pos);
}

GameStatus Board::getGameStatus() const {
    int rowScores[NRows] = {};
    int colScores[NCols] = {};
    int diag1Score = 0;
    int diag2Score = 0;

    for (int row = 0; row < NRows; row++) {
        for (int col = 0; col < NCols; col++) {
            Seed seed = _cells[row][col];
            int delta = _getDelta(seed);
            rowScores[row] += delta;
            colScores[col] += delta;

            if (row == col) {
                diag1Score += delta;
            }

            if (row == NCols - col - 1) {
                diag2Score += delta;
            }
        }
    }

    const int n = NRows > NCols ? NRows : NCols;

    for (const auto &seed : {SeedO, SeedX}) {
        int winPoints = n * _getDelta(seed);
        for (int i = 0; i < n; i++) {
            if (rowScores[i] == winPoints || colScores[i] == winPoints)
                return GameStatus(true, seed);
        }
        if (diag1Score == winPoints || diag2Score == winPoints)
            return GameStatus(true, seed);
    }

    return GameStatus(getEmptyPositions().empty(), SeedEmpty);
}

std::string Board::toString() const {
    std::string s;
    s.reserve(160);
    s.append("  | 0 | 1 | 2 |\n");
    s.append("--+---+---+---+\n");
    for (int row = 0; row < NRows; row++) {
        for (int col = 0; col < NCols; col++) {
            if (col == 0) {
                char sRow[8];
                snprintf(sRow, sizeof(sRow), "%d", row);
                s += sRow;
                s += " | ";
            }
            s += _cells[row][col].getCharValue();
            s += " | ";
            if (col == NCols - 1) {
                s.append("\n--+---+---+---+\n");
            }
        }
    }
    return s;
}