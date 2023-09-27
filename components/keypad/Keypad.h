// Keypad.h
#ifndef KEYPAD_H
#define KEYPAD_H

#include "driver/gpio.h"

class Keypad {
public:
    Keypad(gpio_num_t *rowPins, gpio_num_t *colPins, int numRows, int numCols);
    char getKey();
    void begin();
    
private:
    gpio_num_t *rowPins;
    gpio_num_t *colPins;
    int numRows;
    int numCols;
    void setRow(int row);
    bool isKeyPressed(int row, int col);

    // Define the keymap here
    static const char keymap[4][4];
};

#endif
