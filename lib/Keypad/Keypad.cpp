#include "Keypad.hpp"

const unsigned char Keypad::keymap[] =
{
        SHIFT, SHIFT, '<', '>', SETUP,
        '(', ')', '^', LOG, LN,
        SQRT, ROOT, SIN, COS, TAN,
        '7', '8', '9', DEL, AC,
        '4', '5', '6', 'x', '/',
        '1', '2', '3', '+', '-',
        '0', '.', 'E', ANS, '='
};

const unsigned char Keypad::shift_keymap[] = {
    SHIFT, 0, '>', '!', SETUP,
    1, 2, 3, 4, 146,
    ASIN, ACOS, ASIN, ACOS, ATAN,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    'X', 0, _PI_, _E_, '='
};

Keypad::Keypad(uint8_t _n_columns, uint8_t _n_rows, DigitalOut _columns[], DigitalIn _rows[])
    : n_columns(_n_columns), n_rows(_n_rows), columns(_columns), rows(_rows), holdTime(50), bShift(false), pc(USBTX, USBRX)
{
    lastChar = 0;
    for (int i = 0; i < n_rows; i++)
    {
        rows[i].mode(PullUp);
    }

    for (int j = 0; j < n_columns; j++)
    {
        columns[j] = 1;  //low
    }
}

unsigned char Keypad::getKey()
{
    lastResults = readResults;
    readResults.clear();
    for (int col = 0; col < n_columns; col++)
    {
        //"High"
        columns[col] = 0;
        for (int row = 0; row < n_rows; row++)
        {
            if (!rows[row])
            {
                wait_ms(20);
                if (!rows[row]) {
                    readResults.push_back(row*n_columns + col);
                }

                //if (readResults.front() == 0xF4) {      //shift
                //    bShift = true;
                //}
            }
        }
        //Low
        columns[col] = 1;
    }

    if (readResults == lastResults)
    {
        return 0;
    }
    else if (readResults.size() == 0)
    {
        lastChar = 0;
        return 0;
    }
    else
    {
        std::vector<unsigned char> intersec;
        for(std::vector<unsigned char>::iterator it = readResults.begin(); it != readResults.end(); it++)
        {
            bool bFound = false;
            for (std::vector<unsigned char>::iterator lr = lastResults.begin(); lr != lastResults.end(); lr++)
            {
                if (*it == *lr)
                {
                    bFound = true;
                    break;
                }
            }

            if (bFound == false)
            {
                intersec.push_back(*it);
            }
        }

        //si la letra puesta la ultima vez sigue, no hacer nada
        for (std::vector<unsigned char>::iterator it = intersec.begin(); it != intersec.end(); it++)
        {
            if (*it == lastChar)
                return 0;
        }

        lastChar = intersec.front();
        if (!bShift)
            return keymap[lastChar];
        else
        {
            bShift = false;
            return shift_keymap[lastChar];
        }
    }
}

unsigned char Keypad::waitForKey()
{
    unsigned char key = 0;
    while((key = getKey()) == 0);
    return key;
}

void Keypad::ToggleShift()
{
    bShift = !bShift;
    pc.printf("toggle\n");
}
