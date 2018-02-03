#pragma once

#include <stack>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <string>
#include <Keypad.hpp>
#include <st7565LCD.h>

# define M_PI           3.14159265358979323846
# define M_E            2.7182818284590452354

#define MAX_EXPR_LENGHT 300
#define CHR_PER_LINE 21

enum angle_unit
{
    deg, rad, gra
};

struct calc_settings
{
    calc_settings(uint8_t _cnt, uint8_t _prec, angle_unit _au) : screen_contrast(_cnt), precision(_prec), angleUnit(_au) {};
    uint8_t screen_contrast;
    uint8_t precision;
    angle_unit angleUnit;
};

class calc
{
public:
    /* calculator settings */
    calc_settings settings;

    /* keypad class pointer */
    Keypad* keypad;
    /* screen class pointer */
    ST7565* screen;

    /* infix string */
    char infix[MAX_EXPR_LENGHT];
    /* postfix string */
    char postfix[MAX_EXPR_LENGHT];

    /* operator stack used when converting to RPN */
    std::stack<char> operator_stack;
    /* double stack used when converting to result */
    std::stack<double> double_stack;

    /* cursor position */
    unsigned short cursor;
    /* store result */
    double Ans;

    calc(Keypad* _keypad, ST7565* _screen);
    /* strings operations */
    /* insert a string into other */
    void strins(char* _string1, int _pos, char* _string2);
    /* insert single char into infix */
    void nfxChrIns(char _chr, int _pos);
    /* delete one char of the string at a given position */
    void DelOneChar(char* _string);
    /* delete surplus 0s in result */
    void TrimResult(char* _result);
    /* tokenize input */
    char* tokenize(char* _ptr);
    /* repair input string broken by strtok */
    void RepairInput(char* _cadena, unsigned short size);
    /* read the input and store it until '=' is pressed */
    void ReadingInput();
    /* waits and returns input, refreshes cursor */
    unsigned char WaitingInput();
    /* function called right after equal is pressed */
    /* does the calculation */
    void EqualPressed();
    /* funciton called after result is displayed
       it waits for input and operates accordingly to that input */
    void WaitingInputAfterEqualPressed();

    /* Enter menu */
    void Menu();
    /* Enter precision */
    void SetPrecision();

    /* calculator functions */
    /* infix to postfix, returning 1 if catches an error */
    int RPN();
    /* postfix to result */
    void result();
    /* return operator relevancy */
    unsigned char get_operator_id(char op);

    /* Syntax error: error when creating RPN string */
    void SyntaxError();
    /* Math error: error calculatong the result */
    void MathError();
    /* an error has ocured */
    bool bErrorThrown;

    /* update the bar */
    void UpdateBar();
    /* show input */
    void PrintInput(bool bCursor, bool bEraseResult);
    /* get total size of input string */
    int GetInputSize();
    /* get cursor position */
    int GetCursorPos();

    /* Graphing functions */
    /* Check if expression is an equation */
    bool isGraph();
    /* Start the graphing program */
    void Plot();

};
