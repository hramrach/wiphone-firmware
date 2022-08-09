/*
Copyright © 2019, 2020, 2021 HackEDA, Inc.
Licensed under the WiPhone Public License v.1.0 (the "License"); you
may not use this file except in compliance with the License. You may
obtain a copy of the License at
https://wiphone.io/WiPhone_Public_License_v1.0.txt.

Unless required by applicable law or agreed to in writing, software,
hardware or documentation distributed under the License is distributed
on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "Arduino.h"

/* ========== SOME CONSTANTS ========= */

#define LCD_DRIVER_ST7735S    7735
#define LCD_DRIVER_ST7789V    7789

#define AUDIO_CODEC_WM8731    8731
#define AUDIO_CODEC_WM8750    8750

#define SD_CARD_FREQUENCY     15000000
#define SERIAL_BAUD           500000

/* ===== HARDWARE SELECTION ===== */
#define WIPHONE_INTEGRATED_1_4      // Aamir's preproduction version
//#define WIPHONE_INTEGRATED_1_3      // Mr. Tan's preproduction version
//#define WIPHONE_MAINBOARD_2         // the big dev board
#define WIPHONE_INTEGRATED_1        // single board which integrates all the hardware
#define WIPHONE_BOARD               // are pins 36 and 38 available? is battery gauge on the board? INVERTED_LED?  (as oposed to a random dev board)
#define WIPHONE_KEYBOARD            // MZJ keyboard layout (otherwise - stock keyboard)
#define LCD_DRIVER          LCD_DRIVER_ST7789V
#define AUDIO_CODEC         8750
//#define MOTOR_DRIVER        8833    // 8833 - include library DRV8833.h
#define LED_BOARD                   // initialize APA102 driver and enable "LED Board" app

#ifndef TEST_DB_PINS
#define USER_SERIAL                 // listen to user hardware UART? (pins 22, 38)
#endif
#define USER_SERIAL_BAUD          9600
#define USER_SERIAL_CONFIG        SERIAL_8N1
#define USER_SERIAL_BUFFER_SIZE   2048
//#define USE_VIRTUAL_KEYBOARD
#define VIRTUAL_KEYBOARD_PORT     10101
//#define AUDIO_MCLK_CRYSTAL_KHZ    12228     // Normal mode
//#define AUDIO_MCLK_CRYSTAL_KHZ    12000     // USB mode
#define AUDIO_MCLK_CRYSTAL_KHZ    0      // ESP32 mode (MCLK is fed by ESP32 I2S clock)
#define RMT_TX_CHANNEL            RMT_CHANNEL_0
/* ============================== */

#if AUDIO_CODEC == AUDIO_CODEC_WM8731

#include "src/drivers/WM8731.h"
#define AUDIO_CODEC_CLASS     WM8731
#define AUDIO_CODEC_I2C_ADDR  WM8731_I2C_ADDR_CSB_LOW
#else
#if AUDIO_CODEC == AUDIO_CODEC_WM8750

#include "src/drivers/WM8750.h"
#define AUDIO_CODEC_CLASS     WM8750
#define AUDIO_CODEC_I2C_ADDR  WM8750_I2C_ADDR_CSB_LOW

#endif // AUDIO_CODEC_WM8750
#endif // AUDIO_CODEC_WM8731

#include "src/drivers/SN7326.h"     // keypad
#include "src/drivers/CW2015.h"     // battery gauge

#if defined(MOTOR_DRIVER) && MOTOR_DRIVER == 8833
#include "src/drivers/DRV8833/DRV8833.h"
extern DRV8833 motorDriver;
#endif

/* ============ PINOUT ========== */
#ifdef WIPHONE_INTEGRATED_1_4
#define WIPHONE_INTEGRATED
#define LCD_INVERTED_LED

#include "src/drivers/SX1509/SparkFunSX1509.h"       // better GPIO extender

#define GPIO_EXTENDER                   1509
extern SX1509 gpioExtender;
#define EXTENDER_FLAG                   0x40
#define EXTENDER_PIN(x)                 (x+EXTENDER_FLAG)

#define GPIO_EXTENDER_INTERRUPT_PIN     39
#define TF_CARD_DETECT_PIN              EXTENDER_PIN(1)      // rev1.3 = 38, rev1.4 = EXTENDER_PIN(1)
#define BATTERY_PPR_PIN                 37                   // TODO: use it somehow
#define AMPLIFIER_SHUTDOWN              22                   // rev1.3 - EXTENDER_PIN_B7, rev1.4 - 22
#define HEADPHONE_DETECT_PIN            36
#define BATTERY_CHARGING_STATUS_PIN     EXTENDER_PIN(0)      // rev1.3 - 36, rev1.4 - EXTENDER_PIN(0)

#define KEYBOARD_RESET_PIN              -1
#define KEYBOARD_INTERRUPT_PIN          35
#define I2C_SDA_PIN                     15
#define I2C_SCK_PIN                     25

#define I2S_MCLK_GPIO0
#define I2S_BCK_PIN                     4                    // rev1.3 - 4 (wp05)
#define I2S_WS_PIN                      33
#define I2S_MOSI_PIN                    21
#define I2S_MISO_PIN                    34

#define USER_SERIAL_RX                  38                   // rev1.3 = 22, rev1.4 = 38
#define USER_SERIAL_TX                  32

// Shared pins between SD card and TFT screen
#define SPI_SCL_PIN                     TFT_SCLK             // must be 18
#define SPI_SDA_PIN                     TFT_MOSI             // must be 23
#define SPI_MISO_PIN                    TFT_MISO             // must be 19

#define SD_CARD_CS_PIN                  2
#define LCD_LED_PIN                     EXTENDER_PIN(9)      // rev1.3 = 0 (wp05), rev1.4 = EXTENDER_PIN(9)
#define LCD_RST_PIN                     EXTENDER_PIN(8)      // rev1.3 = N/A
#define LCD_RS_PIN                      TFT_DC               // must be 26
#define LCD_CS_PIN                      TFT_CS               // must be 5

#define KEYBOARD_RST                    EXTENDER_PIN(7)
#define KEYBOARD_LED                    EXTENDER_PIN(5)
#define VIBRO_MOTOR_CONTROL             EXTENDER_PIN(6)
#define POWER_CONTROL                   EXTENDER_PIN(3)
#define POWER_CHECK                     EXTENDER_PIN(2)      // END / POWER_OFF button (below BACK button on the right)
#define ENABLE_DAUGHTER_33V             EXTENDER_PIN(4)

#else // not WIPHONE_INTEGRATED_1_4

#ifdef WIPHONE_INTEGRATED_1_3
#define WIPHONE_INTEGRATED

#include "src/drivers/SN7325.h"       // GPIO extender

#define GPIO_EXTENDER_INTERRUPT_PIN     39
#define TF_CARD_DETECT_PIN              38      // rev1.3 = 38, rev1.4 = -1
#define BATTERY_PPR_PIN                 37      // TODO
#define BATTERY_CHARGING_STATUS_PIN     36
#define AMPLIFIER_SHUTDOWN              EXTENDER_PIN_B7      // WiPhone #1 - 22, WiPhone #2 - EXTENDER_PIN_B7

#define KEYBOARD_RESET_PIN              -1
#define KEYBOARD_INTERRUPT_PIN          35
#define I2C_SDA_PIN                     15
#define I2C_SCK_PIN                     25

#define I2S_MCLK_GPIO0
#define I2S_BCK_PIN                     4      // WiPhone #1 - 19, WiPhone #2 - 4 (wp05)
#define I2S_WS_PIN                      33
#define I2S_MOSI_PIN                    21
#define I2S_MISO_PIN                    34

#define USER_SERIAL_RX                  22      // rev1.3 = 22, rev1.4 = 38
#define USER_SERIAL_TX                  32

// Shared pins between SD card and TFT screen
#define SPI_SCL_PIN                     TFT_SCLK      // must be 18
#define SPI_SDA_PIN                     TFT_MOSI      // must be 23
#define SPI_MISO_PIN                    TFT_MISO      // must be 19

#define SD_CARD_CS_PIN                  2
#define LCD_LED_PIN                     -1             // WiPhone #1 = EXTENDER_PIN_A1,  WiPhone #2 = 0 (wp05), WiPhone #3 (rev. after 1.3) = -1
#define LCD_RST_PIN                     -1
#define LCD_RS_PIN                      TFT_DC        // must be 26
#define LCD_CS_PIN                      TFT_CS        // must be 5

#define KEYBOARD_LED                    EXTENDER_PIN_A0
#define VIBRO_MOTOR_CONTROL             EXTENDER_PIN_B6
#define POWER_CONTROL                   EXTENDER_PIN_A1
#define POWER_CHECK                     EXTENDER_PIN_A2       // END / POWER_OFF button (below BACK button on the right)
#define ENABLE_DAUGHTER_33V             EXTENDER_PIN_A3

#else // not WIPHONE_INTEGRATED_1_3
#ifdef WIPHONE_MAINBOARD_2

#include "src/drivers/SN7325.h"       // GPIO extender

#define GPIO_EXTENDER_INTERRUPT_PIN     39
#define TF_CARD_DETECT_PIN              38
#define BATTERY_PPR_PIN                 37      // TODO: this effectively detects if USB is connected
#define BATTERY_CHARGING_STATUS_PIN     36
#define AMPLIFIER_SHUTDOWN              EXTENDER_PIN_B7

#define KEYBOARD_RESET_PIN              -1
#define KEYBOARD_INTERRUPT_PIN          35
#define I2C_SDA_PIN                     15
#define I2C_SCK_PIN                     25

#define I2S_BCK_PIN                     4
#define I2S_WS_PIN                      33
#define I2S_MOSI_PIN                    21
#define I2S_MISO_PIN                    34

#define LCD_RS_PIN                      26
#define LCD_CS_PIN                      5
#define LCD_SCL_PIN                     18
#define LCD_SDA_PIN                     23
#define SD_CARD_CS_PIN                  2        // SD card shares CS pin with LCD, however LCD is supposed to be HIGH to select

#define KEYBOARD_LED                    EXTENDER_PIN_A0
#define LCD_LED_PIN                     0
#define LCD_RST_PIN                     -1
#define LCD_MISO_PIN                    19                // used for SD card
#define USB_POWER_DETECT_PIN            EXTENDER_PIN_A2
#define VIBRO_MOTOR_CONTROL             EXTENDER_PIN_B6

#else // not WIPHONE_MAINBOARD_2
#ifdef WIPHONE_INTEGRATED_1
#define WIPHONE_INTEGRATED

#define GPIO_EXTENDER_INTERRUPT_PIN     39
#define TF_CARD_DETECT_PIN              38
#define BATTERY_PPR_PIN                 37      // TODO
#define BATTERY_CHARGING_STATUS_PIN     36
#define AMPLIFIER_SHUTDOWN              22      // WiPhone #1 - 22, WiPhone #2 - EXTENDER_PIN_B7

#define KEYBOARD_RESET_PIN              -1
#define KEYBOARD_INTERRUPT_PIN          35
#define I2C_SDA_PIN                     15
#define I2C_SCK_PIN                     25

#define I2S_BCK_PIN                     19      // WiPhone #1 - 19, WiPhone #2 - 4 (wp05)
#define I2S_WS_PIN                      33
#define I2S_MOSI_PIN                    21
#define I2S_MISO_PIN                    34

#define LCD_RS_PIN                      26
#define LCD_CS_PIN                      5
#define LCD_SCL_PIN                     18
#define LCD_SDA_PIN                     23
#define SD_CARD_CS_PIN                  LCD_CS_PIN        // SD card shares CS pin with LCD, however LCD is supposed to be HIGH to select

#define KEYBOARD_LED                    EXTENDER_PIN_A0
#define LCD_LED_PIN                     EXTENDER_PIN_A1             // WiPhone #1 - EXTENDER_PIN_A1,  WiPhone #2 - 0 (wp05)
#define LCD_RST_PIN                     -1
#define LCD_MISO_PIN                    -1                          // WiPhone #1 - -1,  WiPhone #2 - 19 (wp05)
#define USB_POWER_DETECT_PIN            EXTENDER_PIN_A2
#define VIBRO_MOTOR_CONTROL             EXTENDER_PIN_B6

# else // not WIPHONE_INTEGRATED

#ifdef WIPHONE_BOARD
#define USB_POWER_DETECT_PIN          37    // 38 (real USB_POWER_DETECT)
//#define POWER_PRESENCE_PIN            37
#define BATTERY_CHARGING_STATUS_PIN   36
#endif

#define KEYBOARD_RESET_PIN            27
#define KEYBOARD_INTERRUPT_PIN        34
#define I2C_SDA_PIN                   15
#define I2C_SCK_PIN                   14  // 25 (new) / 14 (old)

#define I2S_BCK_PIN                   19
#define I2S_WS_PIN                    2  // 33 (new) /  2 (old)
#define I2S_MOSI_PIN                  21
#define I2S_MISO_PIN                  35  // 32 (new) / 12 (old)

#define LCD_RS_PIN                    26
#define LCD_CS_PIN                    5
#define LCD_SCL_PIN                   18
#define LCD_SDA_PIN                   23
#define LCD_LED_PIN                   13  // 22 (new) / 13 (old)
#define LCD_RST_PIN                   32  // 14 or -1 (new) / 32 (old)
#define LCD_MISO_PIN                  -1

#endif // WIPHONE_INTEGRATED
#endif // WIPHONE_MAINBOARD_2
#endif // WIPHONE_INTEGRATED_1_3
#endif // WIPHONE_INTEGRATED_1_4


/* =========== LED BOARD ========= */
#if defined(LED_BOARD)
#include "src/drivers/APA102/APA102.h"
#define LED_BOARD_DATA        13
#define LED_BOARD_CLOCK       14
#define LED_BOARD_COUNT       60
#define LED_BOARD_BRIGHTNESS  1
extern APA102<LED_BOARD_DATA, LED_BOARD_CLOCK> ledBoard;        // can be any LED strip, actually, with APA102 LEDs
#if GPIO_EXTENDER == 1509
#define LED_BOARD_ENABLE      EXTENDER_PIN(11)
#endif
#endif


/* =========== KEYBOARD ========= */
#ifdef WIPHONE_KEYBOARD
// This is used for >=21-button keyboard
#define LOGIC_BUTTON_OK(x)      (x==WIPHONE_KEY_OK || x==WIPHONE_KEY_CALL || x==WIPHONE_KEY_SELECT)
#define LOGIC_BUTTON_BACK(x)    (x==WIPHONE_KEY_BACK || x==WIPHONE_KEY_END)
#else
// This is used for 16-button keyboard
#define LOGIC_BUTTON_OK(x)    (x==WIPHONE_KEY_OK)
#define LOGIC_BUTTON_BACK(x)  (x==WIPHONE_KEY_BACK)
#endif

/* =========== HELPERS ========= */

void allPinMode(int16_t pin, int16_t mode);
int  allDigitalRead(uint8_t pin);
bool allDigitalWrite(int16_t pin, int16_t val);
void allAnalogWrite(byte pin, byte val);
void lcdLedOnOff(bool turnOn);
void lcdLedOnOff(bool turnOn, uint8_t value);
void rmtTxInit(int16_t rmtPin, bool idleHigh);
void amplifierEnable(int level);

/* =========== EXTENDER ========= */
#ifndef WIPHONE_INTEGRATED_1_4
#ifndef GPIO_EXTENDER
#define GPIO_EXTENDER 7325
#endif
extern SN7325 gpioExtender;
#endif // WIPHONE_INTEGRATED_1_4

/* =========== WIPHONE KEY MASKS ========= */

// 32-bit mask to remember current state of up to 32 buttons (SN7326 returns state of the same button repeatedly)
#define WIPHONE_KEY_MASK_0         0b0000000010000000u
#define WIPHONE_KEY_MASK_1         0b0000000000000001u
#define WIPHONE_KEY_MASK_2         0b0000000000010000u
#define WIPHONE_KEY_MASK_3         0b0000000100000000u
#define WIPHONE_KEY_MASK_4         0b0000000000000010u
#define WIPHONE_KEY_MASK_5         0b0000000000100000u
#define WIPHONE_KEY_MASK_6         0b0000001000000000u
#define WIPHONE_KEY_MASK_7         0b0000000000000100u
#define WIPHONE_KEY_MASK_8         0b0000000001000000u
#define WIPHONE_KEY_MASK_9         0b0000010000000000u
#define WIPHONE_KEY_MASK_HASH      0b0000100000000000u
#define WIPHONE_KEY_MASK_ASTERISK  0b0000000000001000u
#define WIPHONE_KEY_MASK_BACK      0b0010000000000000u
#define WIPHONE_KEY_MASK_OK        0b0100000000000000u
#define WIPHONE_KEY_MASK_UP        0b0001000000000000u
#define WIPHONE_KEY_MASK_DOWN      0b1000000000000000u

#define WIPHONE_KEY_MASK_LEFT      0b0000000010000000000000000u
#define WIPHONE_KEY_MASK_RIFHT     0b0000000100000000000000000u
#define WIPHONE_KEY_MASK_SELECT    0b0000001000000000000000000u
#define WIPHONE_KEY_MASK_CALL      0b0000010000000000000000000u
#define WIPHONE_KEY_MASK_END       0b0000100000000000000000000u
#define WIPHONE_KEY_MASK_F1        0b0001000000000000000000000u
#define WIPHONE_KEY_MASK_F2        0b0010000000000000000000000u
#define WIPHONE_KEY_MASK_F3        0b0100000000000000000000000u
#define WIPHONE_KEY_MASK_F4        0b1000000000000000000000000u


/* =========== LoRa ========= */

#define RFM95_RST     -1
#define RFM95_CS      27
#define RFM95_INT     38

#define HSPI_MISO   12
#define HSPI_MOSI   13
#define HSPI_SCLK   14
#define HSPI_SS     27

#define RF95_FREQ 915.0

#endif // __HARDWARE_H
