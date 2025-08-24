//
// Created by notfound on 25-8-21.
//

#ifndef UARTTTY_STR_H
#define UARTTTY_STR_H

#include <stdint.h>

int32_t uarttty_str_find_index(const char *str, int32_t start_index, char ch);
int32_t uarttty_strcmp(const char *cmp1,const char *cmp2);
void uarttty_strcpy(const char *src, char *dest);
void uarttty_strncpy(const char *src, char *dest, uint32_t size);
int64_t uarttty_str_to_int(const char *str);
double uarttty_str_to_double(const char *str);
int32_t uarttty_is_blank(char ch);

#endif //UARTTTY_STR_H
