//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_POS_H
#define TICTACTOE_POS_H

class Pos {
private:
    int _row, _col;
public:
    Pos() : _row(0), _col(0) {}

    Pos(int row, int col) {
        _row = row;
        _col = col;
    }

    Pos(const Pos &pos) {
        _row = pos._row;
        _col = pos._col;
    }

    int getRow() const {
        return _row;
    }

    int getCol() const {
        return _col;
    }

    void setRow(int row)  {
        _row = row;
    }

    void setCol(int col) {
        _col = col;
    }

    bool operator==(const Pos &pos) const {
        return _row == pos._row && _col == pos._col;
    }

    Pos& operator=(const Pos &pos) {
        _row = pos._row;
        _col = pos._col;
        return *this;
    }
};


#endif //TICTACTOE_POS_H
