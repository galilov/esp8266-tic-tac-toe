//
// Created by Alexander Galilov on 28.09.2019.
// E-Mail: alexander.galilov@gmail.com
//


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp8266/gpio_struct.h"
#include "esp8266/spi_struct.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_libc.h"

#include "driver/gpio.h"
#include "driver/spi.h"

#include "Max7219Display.h"
#include <unistd.h>
#include <vector>


#define GPIO_OUTPUT_IO_0    GPIO_NUM_15
//#define GPIO_OUTPUT_IO_1    16
//#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0)  | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)

volatile uint8_t spiBusy = 0;

static void IRAM_ATTR spi_event_callback(int event, void *arg)
{
    switch (event) {
        case SPI_INIT_EVENT: {
            gpio_set_level(GPIO_OUTPUT_IO_0, 1U);
        }
            break;

        case SPI_TRANS_START_EVENT: {
            //gpio_set_level(GPIO_OUTPUT_IO_0, 0U);

        }
            break;

        case SPI_TRANS_DONE_EVENT: {
            //gpio_set_level(GPIO_OUTPUT_IO_0, 1U);
            spiBusy = 0;
        }
            break;

        case SPI_DEINIT_EVENT: {

        }
            break;
    }
}


Max7219Display::Max7219Display(int nCols, int nRows) :
        _nColPixels(nCols * 8), _nRowPixels(nRows * 8),
        _nColMatrices(nCols), _nRowMatrices(nRows),
        _lengthInMatrices(nCols * nRows)/*,
        _spiChannel(channel), _isDirty(true)*/ {
    _rawData.reserve(_lengthInMatrices * 2); // 2 bytes per command per matrix
    _videoMatrix.reserve(_nColPixels * _nRowPixels);
    setRotation(ROTATION::LANDSCAPE);
    clear();

    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_RX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    spi_config.interface.bit_tx_order = 0;
    spi_config.interface.byte_tx_order = 0;
    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Cancel hardware cs
    spi_config.interface.cs_en = 0;
    // MISO pin is used for DC
    spi_config.interface.miso_en = 0;
    // CPOL: 0, CPHA: 0
    spi_config.interface.cpol = 0;
    spi_config.interface.cpha = 0;
    // Set SPI to master mode
    // 8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_2MHz_DIV;
    // Register SPI event callback function
    spi_config.event_cb = spi_event_callback;
    spi_init(HSPI_HOST, &spi_config);

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en =GPIO_PULLDOWN_DISABLE;
    //enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

Max7219Display::~Max7219Display() {
    //close(_fd);
}

bool Max7219Display::setPixel(int x, int y, bool to) {
    int index = 0;
    if (y >= getRowCount()) {
        printf("y >= getRowCount()");
        return false;
    }
    if (x >= getColCount()) {
        printf("x >= getColCount()");
        return false;
    }
    switch (_rotation) {
        case LANDSCAPE:
            index = x + y * _nColPixels;
            break;
        case PORTRAIT:
            index = _nColPixels - 1 - y + x * _nColPixels;
            break;
        case LANDSCAPE_INV:
            index = (_nColPixels - 1 - x) + (_nRowPixels - 1 - y) * _nColPixels;
            break;
        case PORTRAIT_INV:
            index = y + (_nRowPixels - 1 - x) * _nColPixels;
            break;
    }

    if (_videoMatrix[index] != to) {
        _videoMatrix[index] = to;
        _isDirty = true;
    }
    return _isDirty;
}

void Max7219Display::clear() {
    _videoMatrix.clear();
    for (int i = 0; i < _nColPixels * _nRowPixels; i++) {
        _videoMatrix.emplace_back(false);
    }
    _isDirty = true;
}

void Max7219Display::setBrightness(unsigned char brightness) {
    Max7219Cmd(MAX7219_BRIGHTNESS, brightness).addToVector(_rawData, _lengthInMatrices);
    _rawDataWrite();
}

void Max7219Display::update() {
    if (!_isDirty) return;
    _isDirty = false;
    for (size_t rowPixel = 0; rowPixel < 8; rowPixel++) {
        for (int rowMatrix = _nRowMatrices - 1; rowMatrix >= 0; --rowMatrix) {
            for (int colMatrix = _nColMatrices - 1; colMatrix >= 0; --colMatrix) {
                unsigned char pixels = 0;
                for (size_t colPixel = 0; colPixel < 8; colPixel++) {
                    const auto indexPixel =
                            colMatrix * 8 + rowMatrix * _nColPixels * 8 + rowPixel * _nColPixels + colPixel;
                    pixels *= 2;
                    pixels |= (_videoMatrix[indexPixel] ? 1U : 0);
                }
                Max7219Cmd(rowPixel + 1, pixels).addToVector(_rawData, 1);
            }
        }
        _rawDataWrite();
    }
}

void Max7219Display::hardwareInit() {
    Max7219Cmd commands[] = {Max7219Cmd(MAX7219_TEST, 0),
                             Max7219Cmd(MAX7219_DECODE_MODE, 0),
                             Max7219Cmd(MAX7219_SCAN_LIMIT, 7),
                             Max7219Cmd(MAX7219_SHUTDOWN, 0),
                             Max7219Cmd(MAX7219_BRIGHTNESS, DEFAULT_BRIGHTNESS),
                             Max7219Cmd(1, 0),
                             Max7219Cmd(2, 0),
                             Max7219Cmd(3, 0),
                             Max7219Cmd(4, 0),
                             Max7219Cmd(5, 0),
                             Max7219Cmd(6, 0),
                             Max7219Cmd(7, 0),
                             Max7219Cmd(8, 0),
                             Max7219Cmd(MAX7219_SHUTDOWN, 1)
                             };
    for (auto &cmd : commands) {
        cmd.addToVector(_rawData, _lengthInMatrices);
        _rawDataWrite();
    }
}

void Max7219Display::line(int x1, int y1, int x2, int y2, bool pixelValue) {
    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;
    int error = deltaX - deltaY;
    setPixel(x2, y2, pixelValue);
    while (x1 != x2 || y1 != y2) {
        setPixel(x1, y1, pixelValue);
        const int error2 = error * 2;
        //
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

void Max7219Display::rectangle(int x1, int y1, int x2, int y2, bool pixelValue) {
    int xStart = x1 > x2 ? x2 : x1;
    int xEnd = x1 > x2 ? x1 : x2;
    int yStart = y1 > y2 ? y2 : y1;
    int yEnd = y1 > y2 ? y1 : y2;
    for (int x = xStart; x <= xEnd; x++) {
        for (int y = yStart; y <= yEnd; y++) {
            setPixel(x, y, pixelValue);
        }
    }
}

void Max7219Display::circle(int x0, int y0, int radius, bool pixelValue) {
    int x = 0;
    int y = radius;
    int delta = 1 - 2 * radius;
    int error = 0;
    while (y >= 0) {
        setPixel(x0 + x, y0 + y, pixelValue);
        setPixel(x0 + x, y0 - y, pixelValue);
        setPixel(x0 - x, y0 + y, pixelValue);
        setPixel(x0 - x, y0 - y, pixelValue);
        error = 2 * (delta + y) - 1;
        if (delta < 0 && error <= 0) {
            ++x;
            delta += 2 * x + 1;
            continue;
        }
        error = 2 * (delta - x) - 1;
        if (delta > 0 && error > 0) {
            --y;
            delta += 1 - 2 * y;
            continue;
        }
        ++x;
        delta += 2 * (x - y);
        --y;
    }
}


void Max7219Display::_rawDataWrite() {
    //write(_fd, &_rawData[0], _rawData.size());
    //uint8_t buf[]={0x03, 0x02, 0x01, 0x00};
    //printf("data.size(): %d\n", _rawData.size());
    uint8_t data[4];
    gpio_set_level(GPIO_OUTPUT_IO_0, 0U);
    for(int i = 0; i < _rawData.size(); i+=2) {
        //printf("%#02x ", _rawData[i]);
        data[0] = _rawData[i];
        data[1] = _rawData[i + 1];
        spiBusy = 1;
        spi_trans_t trans = {0};
        trans.mosi = (uint32_t*)data;
        trans.bits.mosi = 8 * 2;
        spi_trans(HSPI_HOST, trans);
        while (spiBusy == 1){}

    }
    gpio_set_level(GPIO_OUTPUT_IO_0, 1U);
    //printf("\n");


    _rawData.clear();
}

