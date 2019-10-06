//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_GAME_H
#define TICTACTOE_GAME_H

#include "Player.h"
#include "GameOverHandler.h"

class Game {
private:
    Board _board;
    Player _playerHuman, _playerMachine;
    GameOverHandler &_gameIsOverHandler;
    const Player *_nextPlayer;
    void _turnToTheOppositePlayer(const Player &oppositePlayer);
    void _checkPlayer(const Player &player);
public:
    Game(GameOverHandler &gameIsOverHandler);
    inline const Board &getBoard() const {
        return _board;
    }

    void doHumanMoveTo(Pos pos);
    Pos doHumanMoveToAi();
    Pos doMachineMove();
};


#endif //TICTACTOE_GAME_H
