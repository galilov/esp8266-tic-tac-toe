//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#include "MiniMax.h"

const MiniMax::Line MiniMax::_lines[] =
        {
                MiniMax::Line(Pos(0, 0), Pos(0, 1), Pos(0, 2)),// row 0
                MiniMax::Line(Pos(1, 0), Pos(1, 1), Pos(1, 2)),// row 1
                MiniMax::Line(Pos(2, 0), Pos(2, 1), Pos(2, 2)),// row 2
                MiniMax::Line(Pos(0, 0), Pos(1, 0), Pos(2, 0)),// col 0
                MiniMax::Line(Pos(0, 1), Pos(1, 1), Pos(2, 1)),// col 1
                MiniMax::Line(Pos(0, 2), Pos(1, 2), Pos(2, 2)),// col 2
                MiniMax::Line(Pos(0, 0), Pos(1, 1), Pos(2, 2)),// diagonal
                MiniMax::Line(Pos(0, 2), Pos(1, 1), Pos(2, 0)),// alternate diagonal
                MiniMax::Line()
        };

int MiniMax::_evaluateLine(const Board &board, const MiniMax::Line &line) const {
    int score = 0;

    const Seed &cell1 = board.getSeedAtPosition(line.getPos(0));
    const Seed &cell2 = board.getSeedAtPosition(line.getPos(1));
    const Seed &cell3 = board.getSeedAtPosition(line.getPos(2));

    // Первая ячейка
    if (cell1 == _ourSeed) {
        score = 1;
    } else if (cell1 == _oppSeed) {
        score = -1;
    }

    // Вторая ячейка
    if (cell2 == _ourSeed) {
        if (score == 1) {   // cell1 is ourSeed
            score = 10;
        } else if (score == -1) {  // cell1 is oppSeed
            return 0;
        } else {  // cell1 is empty
            score = 1;
        }
    } else if (cell2 == _oppSeed) {
        if (score == -1) { // cell1 is oppSeed
            score = -10;
        } else if (score == 1) { // cell1 is ourSeed
            return 0;
        } else {  // cell1 is empty
            score = -1;
        }
    }

    // Третья ячейка
    if (cell3 == _ourSeed) {
        if (score > 0) {  // cell1 and/or cell2 is ourSeed
            score *= 10;
        } else if (score < 0) {  // cell1 and/or cell2 is oppSeed
            return 0;
        } else {  // cell1 and cell2 are empty
            score = 1;
        }
    } else if (cell3 == _oppSeed) {
        if (score < 0) {  // cell1 and/or cell2 is oppSeed
            score *= 10;
        } else if (score > 1) {  // cell1 and/or cell2 is ourSeed
            return 0;
        } else {  // cell1 and cell2 are empty
            score = -1;
        }
    }
    return score;
}


int MiniMax::_evaluate(const Board &board) const {
    int score = 0;
    // Вычисление суммарной эвристики по всем 8 линиям доски: 3 строки, 3 стобца, 2 диагонали
    for (size_t i = 0; !_lines[i].isEmpty(); i++) {
        score += _evaluateLine(board, _lines[i]);
    }
    return score;
}

Score MiniMax::miniMax(Board &board, const Seed &seed) const {
    // ourSeed зерно - ищем максимум, oppSeed зерно - ищем минимум
    int32_t bestScore = (seed == _ourSeed) ? INT32_MIN : INT32_MAX;
    const Pos *bestPos = nullptr;

    if (board.getGameStatus().isOver()) {
        // Game over, вычисляем очки для сложившейся игровой ситуации
        bestScore = _evaluate(board);
    } else {
        // Проходим в цикле по позициям, НЕ ЗАНЯТЫМ зернами
        for (auto const &move : board.getEmptyPositions()) {
            Board clonedBoard(board);
            // Пробуем сделать ход для текущего игрока
            clonedBoard.setSeedAtPosition(move, seed);
            if (seed == _ourSeed) {  // для ourSeed ищем максимум
                int currentScore = miniMax(clonedBoard, _oppSeed).getScorePoints();
                if (currentScore > bestScore) {
                    bestScore = currentScore;
                    bestPos = &move;
                }
            } else {  // для oppSeed ищем минимум
                int currentScore = miniMax(clonedBoard, _ourSeed).getScorePoints();
                if (currentScore < bestScore) {
                    bestScore = currentScore;
                    bestPos = &move;
                }
            }
        }
    }
    return Score(bestPos, bestScore);

}

const Pos* MiniMax::findOptimalMovement(Board &board, const Seed& seed) {
    _ourSeed = seed;
    _oppSeed = _invertSeed(_ourSeed);
    Score score = miniMax(board, _ourSeed);
    return score.getPos();
}