//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#include <iostream>
#include "Game.h"

void Game::_turnToTheOppositePlayer(const Player &oppositePlayer) {
    GameStatus status = _board.getGameStatus();
    if (status.isOver()) {
        _nextPlayer = nullptr;
        _gameIsOverHandler.handleGameIsOver(getBoard(), status.getWinnerSeed());
    } else {
        _nextPlayer = &oppositePlayer;
    }
}

void Game::_checkPlayer(const Player &player) {
    if (_nextPlayer != &player) {
        std::cout << "It's not your step now!\n";
    }
}

Game::Game(GameOverHandler &gameIsOverHandler) :
        _playerHuman(SeedX, _board),
        _playerMachine(SeedO, _board),
        _gameIsOverHandler(gameIsOverHandler) {
    _nextPlayer = &_playerHuman;
}

void Game::doHumanMoveTo(Pos pos) {
    _checkPlayer(_playerHuman);
    _playerHuman.moveTo(pos);
    _turnToTheOppositePlayer(_playerMachine);
}

Pos Game::doHumanMoveToAi() {
    _checkPlayer(_playerHuman);
    Pos pos = _playerHuman.moveToAi();
    _turnToTheOppositePlayer(_playerMachine);
    return pos;
}

Pos Game::doMachineMove() {
    _checkPlayer(_playerMachine);
    Pos pos = _playerMachine.moveToAi();
    _turnToTheOppositePlayer(_playerHuman);
    return pos;
}
