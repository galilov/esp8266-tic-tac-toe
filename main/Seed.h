//
// Created by Alexander Galilov <alexander.galilov@gmail.com> on 9/28/19.
//

#ifndef TICTACTOE_SEED_H
#define TICTACTOE_SEED_H


class Seed {
public:
    enum Value {
        Empty, O, X
    };
private:
    Value _value;
public:
    Seed() : _value(Empty) {}

    Seed(Seed::Value value) : _value(value) {}

    Seed(const Seed &seed) {
        _value = seed._value;
    }

    char getCharValue() const {
        switch (_value) {
            case Empty:
                return '-';
            case O:
                return 'O';
            case X:
                return 'X';
            default:
                return '?';
        }
    }

    Seed::Value getValue() const {
        return _value;
    }

    bool operator==(const Seed &seed) const {
        return _value == seed._value;
    }
};

static const Seed SeedEmpty(Seed::Value::Empty);
static const Seed SeedO(Seed::Value::O);
static const Seed SeedX(Seed::Value::X);


#endif //TICTACTOE_SEED_H
