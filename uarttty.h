//
// Created by notfound on 25-8-21.
//

#ifndef UARTTTY_H
#define UARTTTY_H

#include <stdint.h>

#include "uarttty_conf.h"

#if UARTTTY_EN

#include "usart.h"

int32_t UARTTTY_start(void);
int32_t UARTTTY_Init(void);
int32_t UARTTTY_Process(const UART_HandleTypeDef *huart, uint16_t Size);
int32_t UARTTTY_register(void* func, char* func_name, const uint8_t para_num,const char* para_type,char * func_help);


enum func_para_type {
    UARTTTY_TYPE_INTEGER,
    UARTTTY_TYPE_DOUBLE,
    UARTTTY_TYPE_STRING,
    UARTTTY_TYPE_UNKNOWN
};

typedef struct func_table {
    void* func;
    char* func_name;
    char* func_help;
    uint8_t para_num;
    uint8_t para_type[UARTTTY_FUNC_PARA_MAX];
}func_table_t;

typedef struct tty_t {
    UART_HandleTypeDef *huart;
    uint8_t func_num;
    func_table_t func_table[UARTTTY_FUNC_TABLE_MAX];
    char recv_buf[UARTTTY_RECV_BUF_SIZE];
    char recv_str_buf[UARTTTY_FUNC_PARA_MAX][UARTTTY_PARA_STR_MAX_LEN];
}uart_tty_t;

#endif

#endif //UARTTTY_H
