//
// Created by notfound on 25-8-21.
//

#include "uarttty_conf.h"

#if UARTTTY_EN

#include "uarttty_str.h"

int32_t uarttty_str_find_index(const char *str, const int32_t start_index, const char ch)
{
    if (start_index < 0)
    {
        return -1;
    }
    int32_t index = start_index;
    while (str[index] != '\0')
    {
        if (ch == str[index])
        {
            return index;
        }
        index++;
    }
    return -1;
}

int32_t uarttty_is_blank(const char ch)
{
    switch (ch)
    {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
    case '\f':
        return 1;
    default:
        return 0;
    }
}
int64_t uarttty_str_to_int(const char *str)
{
    int64_t num = 0;
    int8_t negflag = 1;
    int16_t start_index = 0;
    uint8_t hexmode = 0;
    while (uarttty_is_blank(str[start_index]))
    {
        start_index++;
    }
    if (str[start_index] == '-')
    {
        negflag = -1;
        start_index += 1;
    }
    else if (str[start_index] == '+')
    {
        negflag = 1;
        start_index = 1;
    }
    else if (str[start_index + 1] == 'x' || str[start_index + 1] == 'X')
    {
        negflag = 1;
        start_index += 2;
        hexmode = 1;
    }

    for (int i = start_index; str[i] <= '9' && str[i] >= '0' || str[i] <= 'F' && str[i] >= 'A' || str[i] <= 'f' && str[i] >= 'a'; i++)
    {
        if (hexmode == 0)
            num = num * 10 + (str[i] - '0');
        else
        {
            if (str[i] <= 'f' && str[i] >= 'a')
                num = num * 16 + (str[i] - 'a' + 10);
            else if (str[i] <= 'F' && str[i] >= 'A')
                num = num * 16 + (str[i] - 'A' + 10);
            else if (str[i] <= '9' && str[i] >= '0')
                num = num * 16 + (str[i] - '0');
            else
                break;
        }
    }
    return num * negflag;
}

double uarttty_str_to_double(const char *str)
{
    double num = 0;
    double divnum = 1;
    int8_t negflag = 1;
    int8_t pointflag = 0;
    int16_t start_index = 0;
    while (uarttty_is_blank(str[start_index]))
    {
        start_index++;
    }
    if (str[start_index] == '-')
    {
        negflag = -1;
        start_index += 1;
    }
    else if (str[start_index] == '+')
    {
        negflag = 1;
        start_index += 1;
    }

    for (int i = start_index; (str[i] <= '9' && str[i] >= '0') || str[i] == '.'; i++)
    {
        if (str[i] == '.')
        {
            if (pointflag == 0)
            {
                pointflag = 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            num = num * 10 + (str[i] - '0');
            if (pointflag == 1)
                divnum = divnum * 10;
        }
    }
    return num * negflag / divnum;
}

void uarttty_strcpy(const char *src, char *dest)
{
    while (1)
    {
        *dest = *src;
        if (*dest == '\0')
            break;
        dest++;
        src++;
    };
}

void uarttty_strncpy(const char *src, char *dest, uint32_t size)
{
    do
    {
        if (size == 0)
        {
            *dest = '\0';
            return;
        }
        *dest = *src;
        if (*dest == '\0')
        {
            break;
        }
        dest++;
        src++;
    } while (size--);
}
int32_t uarttty_strcmp(const char *cmp1, const char *cmp2)
{
    int32_t index = 0;
    while (cmp1[index] == cmp2[index])
    {
        if (cmp1[index] == '\0')
        {
            return 0;
        }
        index++;
    }
    return cmp1[index] - cmp2[index];
}

#endif
