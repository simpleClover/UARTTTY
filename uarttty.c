//
// Created by notfound on 25-8-21.
//

#include "uarttty_conf.h"

#if UARTTTY_EN

#include "uarttty.h"

#include <stdio.h>
#include "uarttty_str.h"
#include "uarttty_reg_func.h"


uart_tty_t tty={0};

int32_t UARTTTY_Init(void) {
    tty.func_num = 0;
    tty.huart = &UARTTTY_HANDLE_UART;
    for (int i = 0 ; i< UARTTTY_RECV_BUF_SIZE;i++) {
        tty.recv_buf[i] = 0;
    }
    UARTTTY_func_reg();
    UARTTTY_start();
    return 0;
}

int32_t UARTTTY_start() {
    return HAL_UARTEx_ReceiveToIdle_IT(tty.huart,(uint8_t *)tty.recv_buf,UARTTTY_RECV_BUF_SIZE);
}

int32_t UARTTTY_register(void* func, char* func_name, const uint8_t para_num,const char* para_type,char * func_help) {
    func_table_t func_table;
    func_table.func = func;
    func_table.func_name = func_name;
    func_table.para_num = para_num;
    func_table.func_help = func_help;

    for (int i = 0; i < para_num; i++) {
        uint8_t tmp;
        switch (para_type[i] | 0x20) {
            case 's':
                tmp = UARTTTY_TYPE_STRING;
            break;
            case 'n':
                tmp = UARTTTY_TYPE_INTEGER;
            break;
            case 'd':
                tmp = UARTTTY_TYPE_DOUBLE;
            break;
            default:
                tmp = UARTTTY_TYPE_UNKNOWN;
        }
        func_table.para_type[i] = tmp;
    }

    if (tty.func_num >= UARTTTY_FUNC_TABLE_MAX) {
        return -1;
    }
    tty.func_table[tty.func_num++] = func_table;
    return 0;
}

int32_t UARTTTY_Process(const UART_HandleTypeDef *huart, const uint16_t Size) {
    if (huart->Instance == tty.huart->Instance) {

        char* recv_buf = tty.recv_buf;
        const uint16_t recv_size = Size;

        // ---查找第一个"("和最后一个")"---

        int32_t open_paren_index = uarttty_str_find_index(recv_buf, 0, '(');
        int32_t close_paren_index = -1;
        for (int i = recv_size - 1 ; i >= 0; i--) {
            if (uarttty_is_blank(recv_buf[i])) {
                continue;
            }
            if (recv_buf[i] == ')') {
                close_paren_index = i;
                break;
            }
            close_paren_index = -1;
            break;
        }

        // err : 找不到"("或")"
        if (open_paren_index == -1 || close_paren_index == -1) {
            printf("%s has wrong parameter list\r\n" , recv_buf);
            printf("use 'func_name()' to call func\r\n");
            return -1;
        }

        uint16_t func_name_len = open_paren_index;
        uint16_t func_para_len = close_paren_index-open_paren_index-1;

        char func_name_buf[UARTTTY_RECV_BUF_SIZE] = {0};
        char func_para_buf[UARTTTY_RECV_BUF_SIZE] = {0};

        uarttty_strncpy(recv_buf,func_name_buf,func_name_len);
        uarttty_strncpy(recv_buf+open_paren_index+1,func_para_buf,func_para_len);

        // ---补充最后一个逗号便于参数解析---
        func_para_buf[func_para_len] = ',';
        func_para_buf[func_para_len+1] = '\0';
        func_para_len++;

        // ---查找函数名称---

        int32_t func_index = -1;

        for (int i = 0; i < tty.func_num; i++) {
            if (uarttty_strcmp(func_name_buf,tty.func_table[i].func_name)==0) {
                func_index = i;
                break;
            }
        }

        // err : 找不到对应函数
        if (func_index == -1) {
            printf("unknown function '%s'\r\n",func_name_buf);
            printf("get function list by '?(\"\")'\r\n");
            printf("use '?(\"func_name\")' to get function help\r\n");
            return -1;
        }

        // ---处理函数参数---
        uint64_t para[UARTTTY_FUNC_PARA_MAX] = {0};
        uint16_t tty_str_buf_index = 0;

        for (int i = 0; i < tty.func_table[func_index].para_num; i++) {

            if (tty.func_table[func_index].para_type[i] == UARTTTY_TYPE_STRING) {

                int32_t begin_quote_index;
                int32_t end_quote_index;
                int32_t backslash_num = 0;
                int32_t quote_index_t = 0;

                begin_quote_index = uarttty_str_find_index(func_para_buf,0,'\"');

                backslash_num = 0;
                quote_index_t = begin_quote_index - 1;
                while (quote_index_t >= 0) {
                    if (func_para_buf[quote_index_t] != '\\') {
                        break;
                    }
                    backslash_num++;
                    quote_index_t--;
                }

                // err : 找不到左引号
                if (backslash_num % 2 != 0 || begin_quote_index == -1) {
                    printf("parameter %d missing begin quote\r\n", i);
                    return -1;
                }

                // err : 左引号前有其他字符
                for (int j = 0; j < begin_quote_index; j++) {
                    if (!uarttty_is_blank(func_para_buf[i])) {
                        printf("parameter %d has unexpected character before open quote\r\n", i);
                        return -1;
                    }
                }

                end_quote_index = uarttty_str_find_index(func_para_buf,begin_quote_index + 1,'\"');

                // err : 缺失右引号
                if (end_quote_index == -1) {
                    printf("parameter %d missing close quote\r\n", i);
                    return -1;
                }

                do {
                    backslash_num = 0;
                    quote_index_t = end_quote_index - 1;
                    while (quote_index_t >= 0) {
                        if (func_para_buf[quote_index_t] != '\\') {
                            break;
                        }
                        backslash_num++;
                        quote_index_t--;
                    }
                    if (backslash_num % 2 == 0) {
                        break;
                    }
                    end_quote_index = uarttty_str_find_index(func_para_buf,end_quote_index + 1,'\"');
                }while (end_quote_index != -1);

                // err : 没有找到合适的右引号
                if (end_quote_index == -1) {
                    printf("parameter %d missing close quote\r\n", i);
                    return -1;
                }

                const uint16_t comma_index = uarttty_str_find_index(func_para_buf,end_quote_index + 1,',');
                // 其实不会出现这种情况
                // // err : 找不到参数后的逗号
                // if (comma_index == -1) {
                //     printf("parameter %d missing comma\r\n", i);
                // }
                for (int j = end_quote_index + 1; j < comma_index; j++) {

                    // err : 在逗号和右引号之间有其他字符
                    if (!uarttty_is_blank(func_para_buf[j])) {
                        printf("parameter %d has unexpected character after close quote\r\n", i);
                        return -1;
                    }
                }

                // ---拷贝字符串到tty_str_buf---
                char* target_strbuf_p = tty.recv_str_buf[tty_str_buf_index];
                uarttty_strncpy(func_para_buf+begin_quote_index+1, target_strbuf_p,end_quote_index-begin_quote_index-1);

                // ---把逗号及之前的字符移除---
                uarttty_strcpy(func_para_buf+comma_index+1,func_para_buf);

                // ---处理转义字符---
                int32_t dash_pos = -1;
                while (1) {
                    dash_pos = uarttty_str_find_index(target_strbuf_p,dash_pos+1,'\\');
                    if (dash_pos == -1) {
                        break;
                    }
                    if (target_strbuf_p[dash_pos + 1] == 'n') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\n';
                    }else if (target_strbuf_p[dash_pos + 1] == 'a') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\a';
                    }else if (target_strbuf_p[dash_pos + 1] == 't') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\t';
                    }else if (target_strbuf_p[dash_pos + 1] == 'v') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\v';
                    }else if (target_strbuf_p[dash_pos + 1] == 'f') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\f';
                    }else if (target_strbuf_p[dash_pos + 1] == 'r') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\r';
                    }else if (target_strbuf_p[dash_pos + 1] == '\\') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\\';
                    }else if (target_strbuf_p[dash_pos + 1] == '\'') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\'';
                    }else if (target_strbuf_p[dash_pos + 1] == '\"') {
                        uarttty_strcpy(target_strbuf_p+dash_pos+1,target_strbuf_p+dash_pos);
                        target_strbuf_p[dash_pos] = '\"';
                    }else {
                        printf("parameter %d has unsupported escape character\r\n", i);
                        return -1;
                    }
                }

                tty_str_buf_index++;
                para[i] = (uint32_t)target_strbuf_p;

            }else if (tty.func_table[func_index].para_type[i] == UARTTTY_TYPE_INTEGER) {
                uint16_t comma_index = uarttty_str_find_index(func_para_buf,0,',');
                int64_t res = uarttty_str_to_int(func_para_buf);

                // ---把逗号及之前的字符移除---
                uarttty_strcpy(func_para_buf+comma_index+1,func_para_buf);

                para[i] = res;
            }else if (tty.func_table[func_index].para_type[i] == UARTTTY_TYPE_DOUBLE) {
                uint16_t comma_index = uarttty_str_find_index(func_para_buf,0,',');
                double res = uarttty_str_to_double(func_para_buf);

                // ---把逗号及之前的字符移除---
                uarttty_strcpy(func_para_buf+comma_index+1,func_para_buf);

                para[i] = *(uint64_t*)&res;
            }
        }

        // ---函数调用---
        int64_t ret_value = 0;
        switch (tty.func_table[func_index].para_num) {
#if UARTTTY_FUNC_PARA_MAX >= 0
            case 0: {
                typedef int64_t(*target_func_t)();
                target_func_t target_func = (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func();
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 1
            case 1: {
                typedef int64_t(*target_func_t)(uint64_t);
                target_func_t target_func = (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 2
            case 2: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t);
                target_func_t target_func = (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 3
            case 3: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t);
                target_func_t target_func = (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 4
            case 4: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t,uint64_t);
                target_func_t target_func= (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2],para[3]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 5
            case 5: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
                target_func_t target_func= (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2],para[3],para[4]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 6
            case 6: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
                target_func_t target_func= (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2],para[3],para[4],para[5]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 7
            case 7: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
                target_func_t target_func= (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2],para[3],para[4],para[5],para[6]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 8
            case 8: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
                target_func_t target_func= (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2],para[3],para[4],para[5],para[6],para[7]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 9
            case 9: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
                target_func_t target_func= (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2],para[3],para[4],para[5],para[6],para[7],para[8]);
                break;
            }
#endif
#if UARTTTY_FUNC_PARA_MAX >= 10
            case 10: {
                typedef int64_t(*target_func_t)(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
                target_func_t target_func= (target_func_t)tty.func_table[func_index].func;
                ret_value = target_func(para[0],para[1],para[2],para[3],para[4],para[5],para[6],para[7],para[8],para[9]);
                break;
            }
#endif
            default:
                break;
        }
        printf("retValue: %lld\r\n", ret_value);

        // ---清理残留数据---

        for (int i = 0 ; i< UARTTTY_RECV_BUF_SIZE;i++) {
            tty.recv_buf[i] = 0;
        }

        for (int i = 0 ; i< UARTTTY_FUNC_PARA_MAX;i++) {
            for (int j = 0;j < UARTTTY_PARA_STR_MAX_LEN; j++) {
                tty.recv_str_buf[i][j] = 0;
            }
        }
    }
    return 0;
}

#ifdef __GNUC__

int __io_putchar(int ch) {
    HAL_UART_Transmit(tty.huart,(uint8_t*)&ch,1,1000);
    return ch;
}
int __io_getchar(void) {
    int ch;
    HAL_UART_Receive(tty.huart,(uint8_t*)&ch,1,1000);
    return ch;
}

#else

int fputc(int ch,FILE* f) {
    HAL_UART_Transmit(tty.huart,(uint8_t*)&ch,1,1000);
    return ch;
}
int fgetc(FILE* f) {
    int ch;
    HAL_UART_Receive(tty.huart,(uint8_t*)&ch,1,1000);
    return ch;
}

#endif

#endif
