#include <mbed.h>
#include <string.h>
#include <calculator.hpp>
#include <Keypad.hpp>

// Screen connections
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
#define _A0_      PTD0
#define _CS_      PTC16
#define _RST_     PTC17
#define _MOSI_    PTD2
#define _SCK_     PTD1

// Keypad connections
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
#define COLUMN1     PTB9
#define COLUMN2     PTB8
#define COLUMN3     PTB3
#define COLUMN4     PTB2
#define COLUMN5     PTB1
#define ROW1        PTC0
#define ROW2        PTB19 /* PTC11 */
#define ROW3        PTB18 /* PTC10 */
#define ROW4        PTB17 /* PTC6 */
#define ROW5        PTB16 /* PTC5 */
#define ROW6        PTB11
#define ROW7        PTB10

DigitalOut kp_columns[] = {DigitalOut(COLUMN1),DigitalOut(COLUMN2),DigitalOut(COLUMN3),DigitalOut(COLUMN4),DigitalOut(COLUMN5)};
DigitalIn kp_rows[] = {DigitalIn(ROW1), DigitalIn(ROW2), DigitalIn(ROW3), DigitalIn(ROW4), DigitalIn(ROW5), DigitalIn(ROW6), DigitalIn(ROW7)};

int main() {

    Keypad keypad(5, 7, kp_columns, kp_rows);

    ST7565 screen(_MOSI_, _SCK_, _CS_, _RST_, _A0_); // mosi, sclk, cs, rst, a0
    screen.begin(0x1F);
    screen.clear();
    screen.display();

    calc Calc(&keypad, &screen);

    while(1) {
        Calc.ReadingInput();
    }
}
