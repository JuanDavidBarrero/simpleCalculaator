#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Keypad.h"
#include "HD44780.h"

LCD lcd(0x27, 21, 22, 16, 2);

extern "C" void app_main()
{

    lcd.home();
    lcd.writeStr("Hello, World!");

    // Definir pines del teclado matricial
    gpio_num_t rowPins[] = {GPIO_NUM_0, GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3};
    gpio_num_t colPins[] = {GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7};

    Keypad keypad(rowPins, colPins, 4, 4);
    keypad.begin();

    while (1)
    {
        char key = keypad.getKey();
        if (key != '\0')
        {
            printf("Tecla presionada: %c\n", key);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // Pequeña pausa antes de volver a escanear
    }
}
