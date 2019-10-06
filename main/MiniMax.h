//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_MINIMAX_H
#define TICTACTOE_MINIMAX_H


#include "Pos.h"
#include "Seed.h"
#include "Board.h"
#include "Score.h"

class MiniMax {
private:
    struct Line {
        Pos _line[3];
        bool _isEmpty;

        Line(const Pos &pos1, const Pos &pos2, const Pos &pos3) {
            _line[0] = pos1;
            _line[1] = pos2;
            _line[2] = pos3;
            _isEmpty = false;
        }

        Line() : _isEmpty(true) {}

        const Pos &getPos(int index) const {
            return _line[index];
        }

        bool isEmpty() const {
            return _isEmpty;
        }
    };

    static const Line _lines[];
    Seed _ourSeed, _oppSeed;

    static Seed _invertSeed(const Seed &seed) {
        if (seed.getValue() == Seed::Value::Empty) return seed;
        return seed == SeedO ? SeedX : SeedO;
    }

    int _evaluateLine(const Board &board, const Line &line) const;


    int _evaluate(const Board &board) const;

    Score miniMax(Board &board, const Seed &seed) const;

public:
    const Pos *findOptimalMovement(Board &board, const Seed &seed);
};


#endif //TICTACTOE_MINIMAX_H
