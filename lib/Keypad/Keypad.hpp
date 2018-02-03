#pragma once

#include <stdint.h>
#include <vector>
#include "mbed.h"

enum class symbol : char
{
  hola
};

#define ANS     0xF7
#define DEL     0xF6
#define AC      0xF5
#define SHIFT   0xF4
#define ALPHA   0xF3
#define MODE    0xF2
#define SETUP   0xF1

#define SQRT    0x80
#define ROOT    0x81

#define MINUS_UNARY     0xF0

#define LOG     0xF8
#define LN      0xF9

#define SIN     0xFA
#define COS     0xFB
#define TAN     0xFC
#define ASIN    0xFD
#define ACOS    0xFE
#define ATAN    0xFF

#define _E_     0x83
#define _PI_    0x84

class Keypad
{
private:
    static const unsigned char keymap[];

    static const unsigned char shift_keymap[];

    const uint8_t n_columns;
    const uint8_t n_rows;

    std::vector<unsigned char> readResults;
    std::vector<unsigned char> lastResults;
    std::vector<unsigned char> lastIntersec;

    DigitalOut* const columns;
    DigitalIn* const rows;

    unsigned char lastChar;

    int holdTime;

    Serial pc;

public:
    Keypad(uint8_t _n_columns, uint8_t _n_rows, DigitalOut _columns[], DigitalIn _rows[]);

    unsigned char getKey();
    unsigned char waitForKey();

    bool bShift;

    void ToggleShift();
};
