#ifndef HD44780_H
#define HD44780_H

#include <cstdint>

class LCD {
public:
    LCD(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows);
    void setCursor(uint8_t col, uint8_t row);
    void home();
    void clearScreen();
    void writeChar(char c);
    void writeStr(const char* str);

private:
    uint8_t _addr;
    uint8_t _dataPin;
    uint8_t _clockPin;
    uint8_t _cols;
    uint8_t _rows;

    void writeNibble(uint8_t nibble, uint8_t mode);
    void writeByte(uint8_t data, uint8_t mode);
    void pulseEnable(uint8_t data);
};

#endif // LCD_H
