//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_SCORE_H
#define TICTACTOE_SCORE_H


#include "Pos.h"

class Score {
private:
    const Pos* _pos;
    int _scorePoints;
public:

    Score(const Pos* pos, int scorePoints) : _pos(pos), _scorePoints(scorePoints) {}

    const Pos* getPos() const {
        return _pos;
    }

    int getScorePoints() const {
        return _scorePoints;
    }
};


#endif //TICTACTOE_SCORE_H
