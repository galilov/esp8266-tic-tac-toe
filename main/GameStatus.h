//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_GAMESTATUS_H
#define TICTACTOE_GAMESTATUS_H

#include "Seed.h"

class GameStatus {
private:
    const bool _isOver;
    const Seed _winnerSeed;
public:
    GameStatus(bool isOver, const Seed &winnerSeed) :
            _isOver(isOver), _winnerSeed(winnerSeed) {}

    bool isOver() const {
        return _isOver;
    }

    const Seed& getWinnerSeed() const {
        return _winnerSeed;
    }
};


#endif //TICTACTOE_GAMESTATUS_H
