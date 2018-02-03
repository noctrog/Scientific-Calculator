#include "calculator.hpp"
#include "mbed.h"

Serial pc(USBTX, USBRX); // tx, rx

calc::calc(Keypad* _keypad, ST7565* _screen) : keypad(_keypad), screen (_screen), settings(31, 10, deg), bErrorThrown(false)
{
    memset(infix, '\0', MAX_EXPR_LENGHT);
    memset(postfix, '\0', MAX_EXPR_LENGHT);
    Ans = 0;
    cursor = 0;
}

/* strings operations */
/* insert a string into other */
void calc::strins(char* _string1, int _pos, char* _string2)
{
    char buffer[MAX_EXPR_LENGHT] = "";
    memcpy(buffer, _string1, _pos);
    strcat(buffer, _string2);
    for (int i = _pos; i < strlen(_string1); i++) {
        buffer[strlen(_string2) + i] = _string1[i];
    }
    strcpy(_string1, buffer);
}
/* delete one char of the string at a given position */
void calc::DelOneChar(char* _string)
{
    if (cursor > 0) {
        for (int i = cursor; i <= strlen(_string); i++) {
            *(_string + i - 1) = *(_string + i);
        }
        cursor--;
    } else {
        for (int i = cursor; i <= strlen(_string); i++) {
            *(_string + i) = *(_string + i + 1);
        }
    }
}

void calc::nfxChrIns(char _chr, int _pos)
{
    char aux[2] = " ";
    aux[0] = _chr;
    strins(infix, _pos, aux);
    cursor++;
}

/* delete surplus 0s in result */
void calc::TrimResult(char* _result)
{
    for (short i = strlen(_result) - 1; _result[i] == '0' || _result[i] == '.'; i--) {
        if (_result[i] == '.') {
            _result[i] = '\0';
            break;
        }
        _result[i] = '\0';
    }
}
/* tokenize input */
char* calc::tokenize(char* _ptr)
{
    static char* ptr = nullptr;
    static char aux = 0;
    static unsigned short count = 0;
    static bool terminado = false;

    if (terminado) {
        terminado = false;
        return nullptr;
    }
    // if new string
    if (_ptr) {
        ptr = _ptr;
        aux = 0;

        if (isdigit(*ptr) || *ptr == '.') {
            for (int i = 0; ; i++) {
                if (*(ptr + i) == '\0')
                    return nullptr;

                if (!isdigit(*(ptr + i)) && *(ptr + i) != '.') {
                    aux = *(ptr + i);
                    *(ptr + i) = '\0';
                    count = i;
                    break;
                }
            }
            return ptr;
        } else {
            aux = *(ptr + 1);
            *(ptr + 1) = '\0';
            count = 1;
            if (*ptr == '-')
            {
                // unary minus
                return "\xF0";
            }
            return ptr;
        }
    }
    // continue with current string
    else {
        // repair string
        *(ptr + count) = aux;
        ptr = ptr + count;

        if (isdigit(*ptr) || *ptr == '.') {
            for (int i = 0; ; i++) {
                // end of string
                if (*(ptr + i) == '\0') {
                    terminado = true;
                    return ptr;
                }
                // loop until operator is found
                if (!isdigit(*(ptr + i)) && *(ptr + i) != '.')  {
                    aux = *(ptr + i);
                    *(ptr + i) = '\0';
                    count = i;
                    break;
                }
            }
            return ptr;
        } else {
            if (*(ptr + 1) == '\0') {
                terminado = true;
                return ptr;
            } else {
                aux = *(ptr + 1);
                *(ptr + 1) = '\0';
                count = 1;
                if (*ptr == '-')
                {
                    // if previous token is an operator or next token is '('
                    if ((!isdigit(*(ptr - 1)) && *(ptr - 1) != '.') || *(ptr + 1) == '(') {
                        return "\xF0";
                    }
                    // return binary - operator
                    else
                        return ptr;
                }
                else
                    return ptr;
            }
        }
    }
}
/* repair input string broken by strtok */
void calc::RepairInput(char* _cadena, unsigned short size)
{
    int i;
    for (i = size - 1; _cadena[i] == '\0'; i--)
        { }
    for (; i >= 0; i--) {
        _cadena[i] == '\0' ? _cadena[i] = ' ' : 0;
    }
}

/* calculator functions */
int calc::RPN()
{
    int error = false;
    pc.printf("InicioRPN\n");
    char* aux;
    aux = tokenize(infix);

    while (aux != nullptr) {
        /* if the token begins with a digit, then it must be a number */
        if (isdigit(aux[0]) || aux[0] == '.' || aux[0] == _PI_ || aux[0] == _E_) {
            strcat (postfix, aux);  //add number to string
            strcat (postfix, " ");  //separate number with a space
        } else if (strcmp(aux, "(") == 0) {
            operator_stack.push('(');
        } else if  (strcmp(aux, ")") == 0) {
            if (operator_stack.size() == 0) {
                /* Syntax error! */
                pc.printf("SyntaxError");
                error = 1;
                SyntaxError();
                break;
            }
            while (operator_stack.top() != '(') {
                char i[2] = " ";
                i[0] = operator_stack.top();
                operator_stack.pop();

                if (operator_stack.size() == 0) {
                    /* Syntax error! */
                    pc.printf("SyntaxError");
                    error = 1;
                    SyntaxError();
                    break;
                }

                strcat (postfix, i);
                strcat (postfix, " ");
            }
            operator_stack.pop();
        }
        /* then it must be an operator */
        else {
            while (operator_stack.size() > 0 && get_operator_id(operator_stack.top()) >= get_operator_id(aux[0])) {
                char i[2] = " ";
                i[0] = operator_stack.top();
                operator_stack.pop();
                strcat(postfix, i);
                strcat(postfix, " ");
            }
            operator_stack.push(aux[0]);
        }

        aux = tokenize(nullptr);
    }

    while (operator_stack.size() > 0) {
        char i[2] = " ";
        i[0] = operator_stack.top();
        operator_stack.pop();
        if (i[0] == '(' || i[0] == ')') {
            /* Syntax error */
            pc.printf("SyntaxError");
            error = 1;
            SyntaxError();
            break;
        }
        strcat(postfix, i);
        strcat(postfix, " ");
    }

    pc.printf("EndRPN\n");

    return error;
}
/* postfix to result */
void calc::result()
{
    char* aux;
    aux = strtok(postfix, " ");

    while(aux != nullptr) {
        /* if it is a number */
        if (isdigit(aux[0])) {
            double_stack.push(atof(aux));
        }
        /* Ans */
        else if (aux[0] == ANS) {
            double_stack.push(Ans);
        }
        /* Pi */
        else if (aux[0] == _PI_) {
            double_stack.push(M_PI);
        }
        /* e */
        else if (aux[0] == _E_) {
            double_stack.push(M_E);
        }
        /* if it is not a constant */
        else {
            if (double_stack.empty())
            {
                SyntaxError();
                goto error;
            }
            double d1 = double_stack.top();
            double_stack.pop();

            switch(aux[0]) {
                    /* check first for unary operators (sqrt, ln, log, ...) */
                case MINUS_UNARY:
                    double_stack.push((-1)*(d1));
                    break;
                    /* square root */
                case SQRT:
                    double_stack.push(sqrt(d1));
                    break;
                    /* base 10 log */
                case LOG:
                    double_stack.push(log10(d1));
                    break;
                    /* natural log */
                case LN:
                    double_stack.push(log(d1));
                    break;
                    /* sinus */
                case SIN:
                    if (settings.angleUnit == deg) d1 = (d1 * M_PI) / 180.0f;
                    else if (settings.angleUnit == gra) d1 = (d1 * M_PI) / 200.0f;
                    double_stack.push(sin(d1));
                    break;
                    /* cosine */
                case COS:
                    if (settings.angleUnit == deg) d1 = (d1 * M_PI) / 180.0f;
                    else if (settings.angleUnit == gra) d1 = (d1 * M_PI) / 200.0f;
                    double_stack.push(cos(d1));
                    break;
                    /* tangent */
                case TAN:
                    if (settings.angleUnit == deg)
                    {
                        if (fmod(d1-90, 180) == 0)
                        {
                            MathError();
                            goto error;
                        }
                        d1 = (d1 * M_PI) / 180.0f;
                    }
                    else if (settings.angleUnit == gra) d1 = (d1 * M_PI) / 200.0f;
                    double_stack.push(tan(d1));
                    break;
                    /* arc sinus */
                case ASIN:
                    if (d1 > 1 || d1 < -1)
                    {
                        MathError();
                        goto error;
                    }
                    d1 = asin(d1);
                    if (settings.angleUnit == deg) d1 = d1 * 180 / M_PI;
                    else if (settings.angleUnit == gra) d1 = d1 * 200 / M_PI;
                    double_stack.push(d1);
                    break;
                    /* arc cosine */
                case ACOS:
                    if (d1 > 1 || d1 < -1)
                    {
                        MathError();
                        goto error;
                    }
                    d1 = acos(d1);
                    if (settings.angleUnit == deg) d1 = d1 * 180 / M_PI;
                    else if (settings.angleUnit == gra) d1 = d1 * 200 / M_PI;
                    double_stack.push(d1);
                    break;
                    /* arc tangent */
                case ATAN:
                    d1 = atan(d1);
                    if (settings.angleUnit == deg) d1 = d1 * 180 / M_PI;
                    else if (settings.angleUnit == gra) d1 = d1 * 200 / M_PI;
                    double_stack.push(d1);
                    break;
                    /* factorial */
                case '!':
                    if (d1 != (int)d1)
                    {
                        MathError();
                        goto error;
                    }

                    int n;
                    for (n = d1 - 1; n > 1; n--)
                        d1 *= n;
                    double_stack.push(d1);
                    break;
                    /* 'normal' operators */
                default:
                    if (double_stack.empty())
                    {
                        SyntaxError();
                        goto error;
                    }
                    double d2 = double_stack.top();
                    double_stack.pop();

                    switch (aux[0]) {
                        case '+':
                            double_stack.push((d2 + d1));
                            break;
                        case '-':
                            double_stack.push((d2 - d1));
                            break;
                        case 'x':
                            double_stack.push((d2 * d1));
                            break;
                        case '/':
                            double_stack.push((d2 / d1));
                            break;
                        case '^':
                            double_stack.push(pow(d2, d1));
                            break;
                            /* x root y */
                        case ROOT:
                            if (d2 == 0)
                            {
                                MathError();
                                goto error;
                            }
                            double_stack.push(pow(d1, 1.0f/d2));
                            break;
                        case 'E':
                            double_stack.push(d2 * pow(10, d1));
                            break;
                    }
            }
        }

        aux = strtok(nullptr, " ");
    }

    Ans = double_stack.top();
    double_stack.pop();

    error:
    memset(postfix, 0, MAX_EXPR_LENGHT);

    pc.printf("%lf\n", Ans);
}

unsigned char calc::get_operator_id(char op)
{
    switch(op) {
        case '+':
        case '-':
            return 1;
        case 'x':
        case '/':
            return 2;
        case '^':
        case 'q':
        case '!':
        case SIN: case COS: case TAN: case ASIN: case ACOS: case ATAN:
        case ROOT: case SQRT: case LOG: case LN: case MINUS_UNARY:
            return 3;
        case '(':
        case ')':
            return 0;
        default:
            return 4;
    }
}

void calc::ReadingInput()
{
    unsigned char key;
    while ((key = WaitingInput()) != '=') {
        if (isdigit(key) || key == '.') {
            nfxChrIns(key, cursor);
        } else if (key == '<') {
            if (cursor > 0)
                cursor--;
            else
                cursor = strlen(infix);
        } else if (key == '>') {
            if (cursor < strlen(infix))
                cursor++;
            else
                cursor = 0;
        } else if (key == DEL) {
            DelOneChar(infix);
        } else if (key == AC) {
            memset(infix, 0, MAX_EXPR_LENGHT);
            cursor = 0;
        } else if (key == SHIFT) {
            //keypad->ToggleShift();
            keypad->bShift = true;
        } else if (key == SETUP) {
            Menu();
        }
        else {
            nfxChrIns(key, cursor);
        }

        PrintInput(true, true);
    }

    EqualPressed();
    WaitingInputAfterEqualPressed();
}

void calc::PrintInput(bool bCursor, bool bEraseResult)
{
    screen->clear_display();

    if (bEraseResult)
        for (int i = 0; i < 1024; i++)
            screen->st7565_buffer[i] = 0;
    else
        for (int i = 0; i < 896; i++)
            screen->st7565_buffer[i] = 0;

    int size = GetInputSize();

    char buffer[22];
    memset(buffer, 0, 21);
    int count = 0;
    unsigned char line = 1;

    for (int i = 0; i < MAX_EXPR_LENGHT; i++) {
        switch(infix[i]) {
            case LN:
                if ((21 - count) < 2) {
                    screen->drawstring(line, 0, buffer);
                    line++;
                    count = 0;
                    memset(buffer, 0, 21);
                }
                strcat(buffer, "ln");
                count += 2;
                break;
            case ROOT:
                if ((21 - count) < 2) {
                    screen->drawstring(line, 0, buffer);
                    line++;
                    count = 0;
                    memset(buffer, 0, 21);
                }
                strcat(buffer, "\x81\x80");
                count += 2;
                break;
            case SIN:
            case COS:
            case TAN:
            case LOG:
            case ANS:
                if ((21 - count) < 3) {
                    screen->drawstring(line, 0, buffer);
                    line++;
                    count = 0;
                    memset(buffer, 0, 21);
                }
                if (infix[i] == SIN) strcat(buffer, "sin");
                else if (infix[i] == COS) strcat(buffer, "cos");
                else if (infix[i] == TAN) strcat(buffer, "tan");
                else if (infix[i] == LOG) strcat(buffer, "log");
                else if (infix[i] == ANS) strcat(buffer, "Ans");

                count += 3;
                break;

            case ASIN:
            case ATAN:
            case ACOS:
                if ((21 - count) < 4) {
                    screen->drawstring(line, 0, buffer);
                    line++;
                    count = 0;
                    memset(buffer, 0, 21);
                }
                if (infix[i] == ASIN) strcat(buffer, "sin\x82");
                else if (infix[i] == ACOS) strcat(buffer, "cos\x82");
                else if (infix[i] == ATAN) strcat(buffer, "tan\x82");

                count += 4;
                break;
            default:
                if (count >= 21) {
                    screen->drawstring(line, 0, buffer);
                    line++;
                    count = 0;
                    memset(buffer, 0, 22);
                }
                char aux[] = " ";
                aux[0] = infix[i];
                strcat(buffer, aux);

                count++;
                break;
        }
    }

    if (bCursor) {
        int cursorPos = GetCursorPos();
        cursorPos *= 6;
        cursorPos += (cursorPos / (21*6)) * 2 + 128;
        screen->st7565_buffer[cursorPos] = 0xFF;
        screen->st7565_buffer[cursorPos + 1] = 0xFF;
    }

    UpdateBar();

    screen->display();
}

int calc::GetInputSize()
{
    int size = 0;

    for (int i = 0; i < MAX_EXPR_LENGHT; i++) {
        switch(infix[i]) {
            case SIN:
            case COS:
            case TAN:
            case LOG:
            case ANS:
                size += 3;
                break;
            case ASIN:
            case ATAN:
            case ACOS:
                size += 4;
                break;
            case LN:
            case ROOT:
                size += 2;
                break;
            default:
                size++;
                break;
        }
    }

    return size;
}

unsigned char calc::WaitingInput()
{
    unsigned char key = 0;
    Timer t;
    t.start();
    bool showCursor = false;
    int lastTime = 0;
    while ((key = keypad->getKey()) == 0) {
        if (t.read_ms() - lastTime > 500) {
            lastTime = t.read_ms();
            showCursor = !showCursor;
            PrintInput(showCursor, false);
        }
    }

    return key;
}

int calc::GetCursorPos()
{
    int pos = 0;

    for (int i = 0; i < cursor; i++) {
        switch(infix[i]) {
            case SIN:
            case COS:
            case TAN:
            case LOG:
            case ANS:
                pos += 3;
                break;
            case ASIN:
            case ATAN:
            case ACOS:
                pos += 4;
                break;
            case LN:
            case ROOT:
                pos += 2;
                break;
            default:
                pos++;
                break;
        }
    }

    return pos;
}

void calc::WaitingInputAfterEqualPressed()
{
    unsigned char key;

    while((key = keypad->getKey()) == 0) {
        // Do nothing, wait for input
        // Maybe add some low power function
    }

    // if input is x, +, -, or /, make following string: "Ans*operand*"
    switch(key) {
        case '+':
        case '-':
        case 'x':
        case '/':
            // create new string
            memset(infix, 0, MAX_EXPR_LENGHT);
            cursor = 0;

            // insert Ans and operator
            nfxChrIns(ANS, cursor);
            nfxChrIns(key, cursor);
            screen->clear();
            break;

        // If = is pressed
        case '=':
            EqualPressed();
            WaitingInputAfterEqualPressed();
            break;
        // If 'AC' is pressed start a new string
        case AC:
            memset(infix, 0, MAX_EXPR_LENGHT);
            screen->clear();
            cursor = 0;
            break;
        // If DEL is pressed, do nothing
        case DEL:
            break;
        // If arrow keys are pressed
        case '<':
            if (cursor > 0)
                cursor--;
            else
                cursor = strlen(infix);
            break;
        case '>':
            if (cursor < strlen(infix))
                cursor++;
            else
                cursor = 0;
            break;
        case SETUP:
            Menu();
            break;
        case SHIFT:
            keypad->bShift = true;
            break;
        case ALPHA:
            break;
        // start a completely new string, and add key
        default:
            memset(infix, 0, MAX_EXPR_LENGHT);
            cursor = 0;
            nfxChrIns(key, cursor);
            break;
    }
}

void calc::EqualPressed()
{
    int error = 0;
    error = RPN();
    if (!error)
    {
        result();
        if (!bErrorThrown) {
            char cadenaResultado[20] = "";
            sprintf(cadenaResultado, "%.*lf", settings.precision, Ans);
            TrimResult(cadenaResultado);
            PrintInput(0, 0);
            screen->drawstring(7, 0, cadenaResultado);
            screen->display();
        } else
            bErrorThrown = false;
    }
}

void calc::Menu()
{
    // Print the menu
    screen->clear();
    screen->drawstring(0, 0, "        SETUP");
    screen->drawstring(1, 0, "1:Prec");
    screen->drawstring(2, 0, "2:Deg");
    screen->drawstring(3, 0, "3:Rad");
    screen->drawstring(4, 0, "4:Gra");

    screen->display();

    // wait for input
    unsigned char key = 0;

    input:
    key = keypad->waitForKey();

    switch(key)
    {
        case '1':
            SetPrecision();
            break;
        case '2':
            settings.angleUnit = deg;
            break;
        case '3':
            settings.angleUnit = rad;
            break;
        case '4':
            settings.angleUnit = gra;
            break;
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case AC:
        case SETUP:
            break;
        default:
            goto input;
    }

    PrintInput(true, true);
}

void calc::SetPrecision()
{
    screen->clear();

    uint8_t firstNumber = 0;
    uint8_t secondNumber = 0;
    char key = 0;

    screen->drawstring(1, 0, "PRECISION");
    screen->drawstring(2, 0, "Number 1 - 16?");
    screen->drawstring(7, 0, "                 [--]");
    screen->display();

    while (key = keypad->waitForKey())
    {
        if (isdigit(key))
            if (key < '2')
                break;
    }

    char str[] = "                 [--]";

    firstNumber = key - '0';
    str[18] = key;
    screen->drawstring(7, 0, str);
    screen->display();

    while (key = keypad->waitForKey())
    {
        if (isdigit(key))
            if (key >= '7' && key <= '9' && firstNumber == 0)
                break;
            else if (key >= '0' || key < '7')
                break;
    }

    secondNumber = key - '0';
    str[19] = key;
    screen->drawstring(7, 0, str);
    screen->display();

    wait_ms(400);

    settings.precision = firstNumber * 10 + secondNumber;
}

void calc::UpdateBar()
{
    // Clear bar
    screen->drawstring(0, 0, "                     ");

    // Shift button
    if (keypad->bShift)
    {
        screen->drawchar(0, 0, 0xFA);
        screen->drawchar(5, 0, 0xFB);
        screen->drawchar(10, 0, 0xFC);
        screen->drawchar(15, 0, 0xFD);
    }
}

void calc::SyntaxError()
{
    pc.printf("syntaxerror\n");

    screen->clear();

    screen->drawstring(3, 28, "SYNTAX ERROR");

    screen->display();

    memset(postfix, 0, MAX_EXPR_LENGHT);

    keypad->waitForKey();
}

void calc::MathError()
{
    bErrorThrown = true;
    screen->clear();

    screen->drawstring(3, 34, "MATH ERROR");

    screen->display();

    keypad->waitForKey();

    PrintInput(0, 0);
}

bool calc::isGraph()
{
    for (int i = 0; i <= sizeof(infix); i++)
        if (infix[i] == 'X')
            return true;

    return false;
}

void calc::Plot()
{

}
