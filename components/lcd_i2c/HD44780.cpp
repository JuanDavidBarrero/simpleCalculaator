#include "HD44780.h"
#include <esp_log.h>
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static char tag[] = "LCD Driver";

// LCD module defines
#define LCD_LINEONE 0x00   // start of line 1
#define LCD_LINETWO 0x40   // start of line 2
#define LCD_LINETHREE 0x14 // start of line 3
#define LCD_LINEFOUR 0x54  // start of line 4

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_COMMAND 0x00
#define LCD_WRITE 0x01

#define LCD_SET_DDRAM_ADDR 0x80
#define LCD_READ_BF 0x40

// LCD instructions
#define LCD_CLEAR 0x01             // replace all characters with ASCII 'space'
#define LCD_HOME 0x02              // return cursor to first position on first line
#define LCD_ENTRY_MODE 0x06        // shift cursor from left to right on read/write
#define LCD_DISPLAY_OFF 0x08       // turn display off
#define LCD_DISPLAY_ON 0x0C        // display on, cursor off, don't blink character
#define LCD_FUNCTION_RESET 0x30    // reset the LCD
#define LCD_FUNCTION_SET_4BIT 0x28 // 4-bit data, 2-line display, 5 x 7 font
#define LCD_SET_CURSOR 0x80        // set cursor position

static uint8_t LCD_addr;
static uint8_t SDA_pin;
static uint8_t SCL_pin;
static uint8_t LCD_cols;
static uint8_t LCD_rows;

static esp_err_t I2C_init(void)
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_pin;
    conf.scl_io_num = SCL_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;

    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    return ESP_OK;
}

LCD::LCD(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows)
    : _addr(addr), _dataPin(dataPin), _clockPin(clockPin), _cols(cols), _rows(rows)
{
    I2C_init();
    vTaskDelay(100 / portTICK_RATE_MS); // Initial 40 mSec delay

    // Reset the LCD controller
    writeNibble(LCD_FUNCTION_RESET, LCD_COMMAND);    // First part of reset sequence
    vTaskDelay(10 / portTICK_RATE_MS);               // 4.1 mS delay (min)
    writeNibble(LCD_FUNCTION_RESET, LCD_COMMAND);    // second part of reset sequence
    ets_delay_us(200);                               // 100 uS delay (min)
    writeNibble(LCD_FUNCTION_RESET, LCD_COMMAND);    // Third time's a charm
    writeNibble(LCD_FUNCTION_SET_4BIT, LCD_COMMAND); // Activate 4-bit mode
    ets_delay_us(80);                                // 40 uS delay (min)

    // --- Busy flag now available ---
    // Function Set instruction
    writeByte(LCD_FUNCTION_SET_4BIT, LCD_COMMAND); // Set mode, lines, and font
    ets_delay_us(80);

    // Clear Display instruction
    writeByte(LCD_CLEAR, LCD_COMMAND); // clear display RAM
    vTaskDelay(2 / portTICK_RATE_MS);  // Clearing memory takes a bit longer

    // Entry Mode Set instruction
    writeByte(LCD_ENTRY_MODE, LCD_COMMAND); // Set desired shift characteristics
    ets_delay_us(80);

    writeByte(LCD_DISPLAY_ON, LCD_COMMAND); // Ensure LCD is set to on
}

void LCD::setCursor(uint8_t col, uint8_t row)
{
    if (row > LCD_rows - 1)
    {
        ESP_LOGE(tag, "Cannot write to row %d. Please select a row in the range (0, %d)", row, LCD_rows - 1);
        row = LCD_rows - 1;
    }
    uint8_t row_offsets[] = {LCD_LINEONE, LCD_LINETWO, LCD_LINETHREE, LCD_LINEFOUR};
    writeByte(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]), LCD_COMMAND);
}

void LCD::writeChar(char c)
{
    writeByte(c, LCD_WRITE); // Write data to DDRAM
}

void LCD::writeStr(const char *str)
{
    while (*str)
    {
        writeChar(*str++);
    }
}

void LCD::home()
{
    writeByte(LCD_HOME, LCD_COMMAND);
    vTaskDelay(2 / portTICK_RATE_MS); // This command takes a while to complete
}

void LCD::clearScreen()
{
    writeByte(LCD_CLEAR, LCD_COMMAND);
    vTaskDelay(2 / portTICK_RATE_MS); // This command takes a while to complete
}

void LCD::writeNibble(uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode | LCD_BACKLIGHT;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (LCD_addr << 1) | I2C_MASTER_WRITE, 1));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);

    pulseEnable(data); // Clock data into LCD
}

void LCD::writeByte(uint8_t data, uint8_t mode)
{
    writeNibble(data & 0xF0, mode);
    writeNibble((data << 4) & 0xF0, mode);
}

void LCD::pulseEnable(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (LCD_addr << 1) | I2C_MASTER_WRITE, 1));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data | LCD_ENABLE, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    ets_delay_us(1);

    cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (LCD_addr << 1) | I2C_MASTER_WRITE, 1));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (data & ~LCD_ENABLE), 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    ets_delay_us(500);
}
