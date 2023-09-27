// Keypad.cpp
#include "Keypad.h"

// Initialize the keymap
const char Keypad::keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

Keypad::Keypad(gpio_num_t *rowPins, gpio_num_t *colPins, int numRows, int numCols) {
    this->rowPins = rowPins;
    this->colPins = colPins;
    this->numRows = numRows;
    this->numCols = numCols;
}

void Keypad::begin() {
    // Configurar pines de filas como salidas y pines de columnas como entradas
    for (int i = 0; i < numRows; i++) {
        gpio_set_direction(rowPins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(rowPins[i], 0);
    }
    for (int i = 0; i < numCols; i++) {
        gpio_set_direction(colPins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(colPins[i], GPIO_PULLUP_ONLY);
    }
}

void Keypad::setRow(int row) {
    // Establecer una fila activa
    for (int i = 0; i < numRows; i++) {
        gpio_set_level(rowPins[i], (i == row) ? 1 : 0);
    }
}

bool Keypad::isKeyPressed(int row, int col) {
    // Leer el estado de una tecla específica
    return gpio_get_level(colPins[col]) == 0;
}

char Keypad::getKey() {
    // Escanear el teclado y devolver la tecla presionada (o '\0' si ninguna)
    for (int col = 0; col < numCols; col++) {
        for (int row = 0; row < numRows; row++) {
            setRow(row);
            if (isKeyPressed(row, col)) {
                return keymap[row][col];
            }
        }
    }
    return '\0'; // No se ha presionado ninguna tecla
}
