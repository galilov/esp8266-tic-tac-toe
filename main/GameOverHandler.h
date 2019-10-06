//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_GAMEOVERHANDLER_H
#define TICTACTOE_GAMEOVERHANDLER_H

#include "Board.h"

class GameOverHandler {
public:
    virtual void handleGameIsOver(const Board& board, const Seed& winnerSeed) = 0;
};


#endif //TICTACTOE_GAMEOVERHANDLER_H
