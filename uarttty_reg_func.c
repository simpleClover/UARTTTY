//
// Created by notfound on 25-8-22.
//

#include "uarttty_conf.h"

#if UARTTTY_EN

#include "uarttty_reg_func.h"

#include <stdint.h>
#include <stdio.h>
#include "uarttty.h"
#include "uarttty_str.h"

extern struct tty_t tty;
int32_t mem_show_line_cnt = 8;

int64_t uarttty_help(uint64_t para1) {

    UARTTTY_CVT_PARA_TO_CHAR_P(para1,str);

    if (str[0]=='\0') {
        for (int i = 0; i < tty.func_num; i++) {
            printf("func_name: '%s'\r\npara_num: %d\r\npara_type: ", tty.func_table[i].func_name, tty.func_table[i].para_num);
            for (int j =0;j<tty.func_table[i].para_num;j++) {
                switch (tty.func_table[i].para_type[j]) {
                    case UARTTTY_TYPE_INTEGER:
                        printf("%s ","Integer");
                    break;
                    case UARTTTY_TYPE_STRING:
                        printf("%s ","String");
                    break;
                    case UARTTTY_TYPE_DOUBLE:
                        printf("%s ","Double");
                    break;
                    default:
                        printf("%s ","Unknown");
                    break;
                }
            }
            putchar('\r');
            putchar('\n');
            putchar('\r');
            putchar('\n');
        }
    }else {
        for (int i = 0; i < tty.func_num; i++) {
            if (uarttty_strcmp(str, tty.func_table[i].func_name) == 0) {
                printf("%s\r\n", tty.func_table[i].func_help);
                break;
            }
            if (i == tty.func_num - 1) {
                printf("no function called '%s'\r\n",str);
                printf("get function list by '?(\"\")'\r\n");
            }
        }
    }
    return 0;
}

int64_t show_uarttty_cfg() {
    printf("MemTools Cfg:\r\n");
    printf("  mem_show_line_cnt=%d\r\n",mem_show_line_cnt);
    return 0;
}

int64_t uarttty_cfg(uint64_t para1,uint64_t para2) {
    UARTTTY_CVT_PARA_TO_CHAR_P(para1,para);
    UARTTTY_CVT_PARA_TO_INT32(para2,num);

    if (uarttty_strcmp(para,"mem_show_line_cnt") == 0) {
        mem_show_line_cnt = num;
        printf("set mem_show_line_cnt=%d\r\n",mem_show_line_cnt);
    }
    return 0;
}

int64_t read_byte(uint64_t para1,uint64_t para2,uint64_t para3) {

    UARTTTY_CVT_PARA_TO_UINT32(para1,address);
    UARTTTY_CVT_PARA_TO_UINT16(para2,length);
    UARTTTY_CVT_PARA_TO_UINT8(para3,bit_mode);

    uint8_t inline_num = 0;
    printf("Memory Table from %10p to %10p", (uint8_t* )address, (uint8_t* )address + length - 1);
    for (uint32_t addr = address; addr < address + length; addr++) {
        const uint8_t data = *(uint8_t*)addr;
        if (bit_mode == 0) {
            if (inline_num == 0) {
                printf("\r\n%10p : %02x", (uint8_t* )addr, (unsigned int)data);
            }else {
                printf(" %02x", (unsigned int)data);
            }
        }else {
            if (inline_num == 0) {
                printf("\r\n%10p : ", (uint8_t* )addr);
                for (uint8_t mask = 0x80 ; mask != 0; mask >>= 1)
                    putchar(data & mask ? '1' : '0');
                putchar(' ');
            }else {
                for (uint8_t mask = 0x80 ; mask != 0; mask >>= 1)
                    putchar(data & mask ? '1' : '0');
                putchar(' ');
            }
        }
        inline_num++;
        inline_num %= mem_show_line_cnt;
    }
    putchar('\r');
    putchar('\n');
    return 0;
}

int64_t write_byte(uint64_t para1,uint64_t para2) {

    UARTTTY_CVT_PARA_TO_UINT32(para1,address);
    UARTTTY_CVT_PARA_TO_UINT8(para2,data);

    *(uint8_t*)address = data;
    volatile uint8_t rd_data = 0;
    rd_data = *(uint8_t*)address;
    printf("1st read %10p : %02x\r\n", (uint8_t* )address, (unsigned int)rd_data);
    rd_data = *(uint8_t*)address;
    printf("2nd read %10p : %02x\r\n", (uint8_t* )address, (unsigned int)rd_data);
    return 0;
}

int64_t add_demo(uint64_t para1,uint64_t para2, uint64_t para3,uint64_t para4) {

    UARTTTY_CVT_PARA_TO_INT32(para1,n1);
    UARTTTY_CVT_PARA_TO_INT32(para2,n2);
    UARTTTY_CVT_PARA_TO_DOUBLE(para3,d1);
    UARTTTY_CVT_PARA_TO_DOUBLE(para4,d2);

    printf("%lf+%lf=%lf\r\n", d1, d2, d1+d2);

    return n1+n2;
}

int64_t printf_demo(uint64_t para1,uint64_t para2,uint64_t para3) {

    UARTTTY_CVT_PARA_TO_CHAR_P(para1,fmt_str);
    UARTTTY_CVT_PARA_TO_INT32(para2,n);
    UARTTTY_CVT_PARA_TO_DOUBLE(para3,d);

    printf(fmt_str,n,d);
    return 0;
}

void UARTTTY_func_reg() {
    UARTTTY_register(uarttty_help,"?",1,"S",
        "function '?' is for function list or function list\r\n"
        "use '?(\"\")' to get all registered function\r\n"
        "use '?(\"func_name\") to get target function help\r\n'");

    UARTTTY_register(show_uarttty_cfg,"show_uarttty_cfg",0,"",
        "function 'show_uarttty_cfg' shows the self defined config\r\n"
        "all the config type is int32_t\r\n");

    UARTTTY_register(uarttty_cfg,"uarttty_cfg",2,"SN",
        "function 'uarttty_cfg' can edit the self defined config\r\n"
        "para1(string) is the config name\r\n"
        "para2(int32_t) is the config value\r\n");

    UARTTTY_register(read_byte,"read_byte",3,"NNN",
         "function 'read_byte' can read bytes and display value to the serialport\r\n"
         "para1(uint32_t) is the begin memory address\r\n"
         "para2(uint16_t) is the length to read\r\n"
         "para3(uint8_t) is bit_mode, input 0 or 1\r\n"
         "e.g. read_byte(0x00000100,32,0)");

    UARTTTY_register(write_byte,"write_byte",2,"NN",
        "function 'write_byte' can write bytes to memory\r\n"
        "para1(uint32_t) is the target memory address\r\n"
        "para2(uint8_t) is the value\r\n");

    UARTTTY_register(printf_demo,"printf_demo",3,"SND",
        "function 'printf_demo' is a para simple test\r\n"
        "para1(string) is the printf format\r\n"
        "para2(int32_t) is a number\r\n"
        "para3(double) is a number\r\n"
        "in function will call the printf(para1,para2,para3)\r\n"
        "e.g. printf_demo(\"recv value: %d %lf\\r\\n\",42,-42.5)\r\n");

    UARTTTY_register(add_demo,"add_demo",4,"NNDD",
    "function 'add_demo' is a para simple test\r\n"
    "para1(int32_t) is first integer\r\n"
    "para2(int32_t) is second integer\r\n"
    "para3(double) is first double\r\n"
    "para4(double) is second double\r\n"
    "e.g. add_demo(-15, 0x15, -12.5, 30.25)\r\n");
}
#endif
