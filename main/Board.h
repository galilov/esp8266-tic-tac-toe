//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_BOARD_H
#define TICTACTOE_BOARD_H

#include <unordered_set>
#include <hash_set>
#include <string>
#include "Seed.h"
#include "Pos.h"
#include "GameStatus.h"

namespace std {
    template<>
    struct hash<Pos> {
        typedef Pos argument_type;
        typedef size_t result_type;

        size_t operator()(const Pos &pos) const {
            return pos.getCol() + pos.getRow() * 31;
        }
    };
}

class Board {
public:
    static const int NRows = 3, NCols = 3;
    typedef std::unordered_set<Pos> TEmptyPositions;
private:
    Seed _cells[NRows][NCols];
    TEmptyPositions _emptyPositions;

    void _copyFrom(const Board &board);

    inline static int _getDelta(const Seed &seed) {
        if (seed == Seed::X) return 1;
        else if (seed == Seed::O) return -1;
        return 0;
    }

    void _clear();

public:
    Board();

    Board(const Board &board);

    Board &operator=(const Board &board);

    inline const TEmptyPositions &getEmptyPositions() const {
        return _emptyPositions;
    }

    inline const Seed &getSeedAtPosition(const Pos &pos) const {
        return _cells[pos.getRow()][pos.getCol()];
    }

    void setSeedAtPosition(const Pos &pos, const Seed &seed);

    GameStatus getGameStatus() const;

    std::string toString() const;
};


#endif //TICTACTOE_BOARD_H
