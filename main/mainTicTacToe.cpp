#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <iostream>
#include "driver/gpio.h"
#include "Game.h"
#include "Max7219Display.h"

extern "C" int runGame();

class MyGameOverHandler : public GameOverHandler {
private:
    bool _stopGame;
public:
    MyGameOverHandler() : _stopGame(false) {}

    void handleGameIsOver(const Board &board, const Seed &winner) {
        _stopGame = true;
        std::cout << board.toString();
        switch (winner.getValue()) {
            case Seed::Value::Empty:
                std::cout << "Draw\n";
                break;
            case Seed::Value::X:
                std::cout << "Man won\n";
                break;
            case Seed::Value::O:
                std::cout << "Computer won\n";
                break;
        }
    }

    bool isGameStopped() {
        return _stopGame;
    }
};

const int sym_rows = 4;
const int sym_cols = 4;

static const uint8_t symX[sym_rows][sym_cols] = {
        {1, 0, 0, 1},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {1, 0, 0, 1},
};

static const uint8_t symO[sym_rows][sym_cols] = {
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0},
};

static const uint8_t symEmpty[sym_rows][sym_cols] = {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
};


const int res_rows = 16;

static const char *resultAIWon[res_rows] = {
        ".###..###.......",
        "#...#..#........",
        "#...#..#........",
        "#...#..#........",
        "#####..#........",
        "#...#..#........",
        "#...#.###.......",
        "................",
        "#...#.###.#....#",
        "#...##...##....#",
        "#...##...##....#",
        "#...##...###...#",
        "#.#.##...##.#..#",
        "##.###...##..#.#",
        "#...#.###.#....#",
        "................",
};

static const char *resultYouWon[res_rows] = {
        "#...#.###.#...#.",
        "#...##...##...#.",
        ".#.#.#...##...#.",
        "..#..#...##...#.",
        "..#..#...##...#.",
        "..#..#...##...#.",
        "..#...###..###..",
        "................",
        "#...#.###.#....#",
        "#...##...##....#",
        "#...##...##....#",
        "#...##...###...#",
        "#.#.##...##.#..#",
        "##.###...##..#.#",
        "#...#.###.#....#",
        "................",
};

static const char *resultTie[res_rows] = {
        "#####.###.####..",
        "..#....#..#.....",
        "..#....#..#.....",
        "..#....#..###...",
        "..#....#..#.....",
        "..#....#..#.....",
        "..#...###.####..",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
};

void printSym(Max7219Display &display, const uint8_t (&sym)[sym_rows][sym_cols], int x, int y) {
    for (int row = 0; row < sym_rows; row++) {
        for (int col = 0; col < sym_cols; col++) {
            display.setPixel(x + col, y + row, sym[row][col] != 0);
        }
    }
}

void printRes(Max7219Display &display, const char *(&res)[res_rows], int x, int y) {
    for (int row = 0; row < res_rows; row++) {
        for (int col = 0; res[row][col] != '\0'; col++) {
            display.setPixel(x + col, y + row, res[row][col] == '#');
        }
    }
}


void printBoard(Max7219Display &display, const Board &board) {
    for (int row = 0; row < Board::NRows; row++) {
        for (int col = 0; col < Board::NCols; col++) {
            const uint8_t (*sym)[sym_rows][sym_cols];
            switch (board.getSeedAtPosition(Pos(row, col)).getValue()) {
                case Seed::Value::X:
                    sym = &symX;
                    break;
                case Seed::Value::O:
                    sym = &symO;
                    break;
                case Seed::Value::Empty:
                default:
                    sym = &symEmpty;
                    break;

            }
            printSym(display, *sym,
                     col * display.getColCount() / Board::NCols + (display.getColCount() % Board::NCols),
                     row * display.getRowCount() / Board::NRows + (display.getRowCount() % Board::NRows));
        }
    }

    for (int y = 0; y < display.getRowCount(); y += (sym_rows + 1)) {
        for (int x = 0; x < display.getRowCount(); x += (sym_cols + 1)) {
            display.setPixel(x, y, true);
        }
    }
}

#define BTN_RIGHT   GPIO_NUM_5
#define BTN_LEFT    GPIO_NUM_4
#define BTN_APPLY   GPIO_NUM_12
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<BTN_RIGHT) | (1ULL<<BTN_LEFT) | (1ULL<<BTN_APPLY))

static int prevBtnApplyStatus = 1, prevBtnLeftStatus = 1, prevBtnRightStatus = 1;
static int cursorAbsolutePos = 4; // center. Range is 0..8

MyGameOverHandler handler;
Game game(handler);
Max7219Display display(2, 2);

bool calcCursorPos(Pos &cursorPos) {
    if (cursorAbsolutePos > Board::NCols * Board::NRows - 1) {
        cursorAbsolutePos = 0;
    } else if (cursorAbsolutePos < 0) {
        cursorAbsolutePos = Board::NCols * Board::NRows - 1;
    }

    cursorPos.setRow(cursorAbsolutePos / Board::NCols);
    cursorPos.setCol(cursorAbsolutePos % Board::NCols);

    return game.getBoard().getSeedAtPosition(cursorPos).getValue() == Seed::Value::Empty;
}


int runGame() {

    display.hardwareInit();

    gpio_config_t io_conf = {0};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    bool cursorFlag = false;
    uint32_t gameOverCount = 0;
    bool gameIsOver = false;

    for (uint32_t nCycleCount = 0;; nCycleCount++) {
        int btnApplyStatus = gpio_get_level(BTN_APPLY);
        int btnLeftStatus = gpio_get_level(BTN_LEFT);
        int btnRightStatus = gpio_get_level(BTN_RIGHT);

        bool btnApplyIsTapped = false;
        bool btnLeftIsTapped = false;
        bool btnRightIsTapped = false;

        if (btnApplyStatus != prevBtnApplyStatus) {
            if (btnApplyStatus == 0) {
                // apply button is tapped
                btnApplyIsTapped = true;
            }
            prevBtnApplyStatus = btnApplyStatus;
        }

        if (btnLeftStatus != prevBtnLeftStatus) {
            if (btnLeftStatus == 0) {
                // left button is tapped
                btnLeftIsTapped = true;
            }
            prevBtnLeftStatus = btnLeftStatus;
        }

        if (btnRightStatus != prevBtnRightStatus) {
            if (btnRightStatus == 0) {
                // left button is tapped
                btnRightIsTapped = true;
            }
            prevBtnRightStatus = btnRightStatus;
        }

        if (gameIsOver) {
            if (gameOverCount++ > 60 /*3s*/) {
                switch (game.getBoard().getGameStatus().getWinnerSeed().getValue()) {
                    case Seed::Value::X:
                        printRes(display, resultYouWon, 0, 0);
                        break;
                    case Seed::Value::O:
                        printRes(display, resultAIWon, 0, 0);
                        break;
                    default:
                        printRes(display, resultTie, 0, 0);
                }
                display.update();
                break;
            }
        }

        Pos cursorPos = Pos(cursorAbsolutePos / Board::NCols, cursorAbsolutePos % Board::NCols);

        while (!gameIsOver) {
            if (btnLeftIsTapped) {
                cursorAbsolutePos++;
            } else if (btnRightIsTapped) {
                cursorAbsolutePos--;
            } else {
                break;
            }

            if (calcCursorPos(cursorPos)) break;
        }


        //printBoard(display, game.getBoard());
        if (nCycleCount % 5 == 0) {
            cursorFlag = !cursorFlag;
            printBoard(display, game.getBoard());
            int col = cursorPos.getCol();
            int row = cursorPos.getRow();
            printSym(display, cursorFlag ? symX : symEmpty,
                     col * display.getColCount() / Board::NCols + (display.getColCount() % Board::NCols),
                     row * display.getRowCount() / Board::NRows + (display.getRowCount() % Board::NRows));
            display.update();
        }

        if (btnApplyIsTapped) {
            game.doHumanMoveTo(Pos(cursorAbsolutePos / Board::NCols, cursorAbsolutePos % Board::NCols));
            gameIsOver = game.getBoard().getGameStatus().isOver();
            printBoard(display, game.getBoard());
            display.update();
            if (!gameIsOver) {
                game.doMachineMove();
                gameIsOver = game.getBoard().getGameStatus().isOver();
                printBoard(display, game.getBoard());
            }
            if (!gameIsOver) {
                while (!calcCursorPos(cursorPos)) {
                    cursorAbsolutePos++;
                }
            }
            display.update();
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    abort();
    return 0;
}