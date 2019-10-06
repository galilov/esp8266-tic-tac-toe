//
// Created by Alexander Galilov on 28.09.2019.
// E-Mail: alexander.galilov@gmail.com
//

#ifndef MAX7219DISPLAY_H
#define MAX7219DISPLAY_H

#include <stddef.h>
#include <vector>

struct Max7219Cmd {
    unsigned char cmd;
    unsigned char data;

    Max7219Cmd(unsigned char c, unsigned char d) : cmd(c), data(d) {}

    Max7219Cmd() : cmd(0), data(0) {}

    void addToVector(std::vector<unsigned char> &target, size_t n) {
        while (n-- > 0) {
            target.push_back(cmd);
            target.push_back(data);
        }
    }
};

class Max7219Display {
    static const unsigned char DEFAULT_BRIGHTNESS = 5;
    // MAX7219 LED Driver commands
    static const unsigned char MAX7219_TEST = 0x0f;
    static const unsigned char MAX7219_BRIGHTNESS = 0x0a;
    static const unsigned char MAX7219_SCAN_LIMIT = 0x0b;
    static const unsigned char MAX7219_DECODE_MODE = 0x09;
    static const unsigned char MAX7219_SHUTDOWN = 0x0C;
public:
    enum ROTATION {
        LANDSCAPE, PORTRAIT, LANDSCAPE_INV, PORTRAIT_INV
    };
public:
    // Columns and rows are in 8x8 LED modules
    Max7219Display(int nCols, int nRows);
    virtual ~Max7219Display();

    bool setPixel(int x, int y, bool to);
    void clear();
    void setBrightness(unsigned char brightness);
    void update();
    void hardwareInit();

    inline int getRowCount() {
        return _rotation == ROTATION::LANDSCAPE || _rotation == ROTATION::LANDSCAPE_INV ? _nRowPixels : _nColPixels;
    }

    inline int getColCount() {
        return _rotation == ROTATION::LANDSCAPE || _rotation == ROTATION::LANDSCAPE_INV ? _nColPixels : _nRowPixels;
    }

    void setRotation(ROTATION rotation) {
        _rotation = rotation;
    }

    void line(int x1, int y1, int x2, int y2, bool pixelValue);
    void rectangle(int x1, int y1, int x2, int y2, bool pixelValue);
    void circle(int x0, int y0, int radius, bool pixelValue);

private:
    Max7219Display(const Max7219Display &);

    const Max7219Display &operator=(const Max7219Display &);

    void _rawDataWrite();

private:
    const int _nColPixels, _nRowPixels;
    const int _nColMatrices, _nRowMatrices;
    const int _lengthInMatrices;
    //const int _spiChannel;
    std::vector<unsigned char> _videoMatrix;
    std::vector<unsigned char> _rawData;
    bool _isDirty;
    //int _fd;
    ROTATION _rotation;
};


#endif //TETRIS_MAX7219DISPLAY_H
