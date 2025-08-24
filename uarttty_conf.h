//
// Created by notfound on 25-8-21.
//

#ifndef UARTTTY_CONF_H
#define UARTTTY_CONF_H

#define UARTTTY_EN 0

#if UARTTTY_EN

#define UARTTTY_HANDLE_UART huart3
#define UARTTTY_RECV_BUF_SIZE 512
#define UARTTTY_FUNC_PARA_MAX 5         // 0-10
#define UARTTTY_FUNC_TABLE_MAX 20
#define UARTTTY_PARA_STR_MAX_LEN 128

#define UARTTTY_CVT_PARA_TO_DOUBLE(para,var_name)   double var_name     = *(double*)&para
#define UARTTTY_CVT_PARA_TO_CHAR_P(para,var_name)   char* var_name      = (char*)(uint32_t)para
#define UARTTTY_CVT_PARA_TO_INT64(para,var_name)    int64_t var_name    = (int64_t)para
#define UARTTTY_CVT_PARA_TO_INT32(para,var_name)    int32_t var_name    = (int32_t)para
#define UARTTTY_CVT_PARA_TO_INT16(para,var_name)    int16_t var_name    = (int16_t)para
#define UARTTTY_CVT_PARA_TO_INT8(para,var_name)     int8_t var_name     = (int8_t)para
#define UARTTTY_CVT_PARA_TO_UINT64(para,var_name)   uint64_t var_name   = (uint64_t)para
#define UARTTTY_CVT_PARA_TO_UINT32(para,var_name)   uint32_t var_name   = (uint32_t)para
#define UARTTTY_CVT_PARA_TO_UINT16(para,var_name)   uint16_t var_name   = (uint16_t)para
#define UARTTTY_CVT_PARA_TO_UINT8(para,var_name)    uint8_t var_name    = (uint8_t)para

#endif

#endif //UARTTTY_CONF_H
