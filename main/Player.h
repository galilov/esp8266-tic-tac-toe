//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_PLAYER_H
#define TICTACTOE_PLAYER_H


#include "Seed.h"
#include "Board.h"
#include "MiniMax.h"

class Player {
private:
    const Seed& _ourSeed;
    Board& _board;
    MiniMax _ai;
public:
    Player(const Seed& ourSeed, Board& board) : _ourSeed(ourSeed), _board(board) {}

    inline Pos moveToAi() {
        const Pos *pos = _ai.findOptimalMovement(_board, getSeed());
        _board.setSeedAtPosition(*pos, _ourSeed);
        return *pos;
    }

    inline void moveTo(const Pos &pos) {
        _board.setSeedAtPosition(pos, getSeed());
    }

    inline const Seed& getSeed() const {
        return _ourSeed;
    }
};


#endif //TICTACTOE_PLAYER_H
