# 注意: 该项目基于HAL库开发

## 配置过程

### 1. 使用STM32CubeMX启用自定的`UARTx`并使能关联的中断`uart global interrupt`

### 2. 打开目录中的`uarttty_conf.h`

将`#define UARTTTY_EN 0`改为`#define UARTTTY_EN 1`

其中`uarttty_conf.h`中可配置的参数如下
```c
#define UARTTTY_HANDLE_UART huart1
#define UARTTTY_RECV_BUF_SIZE 512
#define UARTTTY_FUNC_PARA_MAX 5
#define UARTTTY_FUNC_TABLE_MAX 20
#define UARTTTY_PARA_STR_MAX_LEN 128
```

`UARTTTY_RECV_BUF_SIZE`指定了一次接受命令的最大长度  
`UARTTTY_FUNC_PARA_MAX`指定了注册函数可用的最大参数数量  
`UARTTTY_FUNC_TABLE_MAX`指定了可注册函数的最大数量  
`UARTTTY_PARA_STR_MAX_LEN`指定了每个函数字符串参数的最大长度  
合理配置上述参数可节约程序占用空间

### 3. 使用自定的`huartx`替换`huart1`


### 4. 在`main.c`和`stm32xxxx_it.c`文件中添加头文件(视具体文件而定)
```c
#include "uarttty.h"
```

### 5. 在`main`函数中添加初始化函数
```c
UARTTTY_Init();
```

### 6. 在中断处理文件`stm32xxxx_it.c`中找到与`UARTx`关联的中断函数

在`HAL_UART_IRQHandler`后添加`UARTTTY_start();`以重新打开串口接收

例如
```c
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */
  UARTTTY_start();
  /* USER CODE END USART1_IRQn 1 */
}
```
### 7. 在`HAL_UARTEx_RxEventCallback`中添加`UARTTTY_Process`函数

若还没有自定义过`HAL_UARTEx_RxEventCallback`,则可以在中断处理文件`stm32xxxx_it.c`末尾添加:

```c
/* USER CODE BEGIN 1 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    UARTTTY_Process(huart, Size);
}
/* USER CODE END 1 */
```

至此已经可以完成了基本配置,可以在串口交互了

## 自定义注册串口命令

### 自定义函数的参数列表

对于需要注册为串口命令的函数,其参数类型必须为`uint64_t`返回值类型必须为`int64_t`,在函数内使用`UARTTTY_CVT_TO_XXX`进行参数解析

```c
int64_t add_demo(uint64_t para1,uint64_t para2, uint64_t para3,uint64_t para4) {
    
    UARTTTY_CVT_PARA_TO_INT32(para1,n1);
    UARTTTY_CVT_PARA_TO_INT32(para2,n2);
    UARTTTY_CVT_PARA_TO_DOUBLE(para3,d1);
    UARTTTY_CVT_PARA_TO_DOUBLE(para4,d2);
    
    printf("%lf+%lf=%lf\r\n", d1, d2, d1+d2);

    return n1+n2;
}
```

### 注册自定义函数

使用`UARTTTY_register`函数便可实现自定义串口命令的注册
```c
int32_t UARTTTY_register(void* func, char* func_name, const uint8_t para_num,const char* para_type,char * func_help)
```

`func`为被注册的函数指针  
`func_name`指定在串口中用什么命令代指被注册函数  
`para_num`为被注册函数的参数数量  
`para_type`传入被注册函数的参数类型  
`func_help`传入被注册函数的函数帮助  

其中`para_type`传入字符指针,`UARTTTY_register`根据`para_num`解析前指定个数的字符

当字符为`s`或`S`时类型为`UARTTTY_TYPE_STRING`  
当字符为`n`或`N`时类型为`UARTTTY_TYPE_INTEGER`  
当字符为`d`或`D`时类型为`UARTTTY_TYPE_DOUBLE`  

```c
UARTTTY_register(add_demo,"add_demo",4,"NNDD",
    "function 'add_demo' is a para simple test\r\n"
    "para1(int32_t) is first integer\r\n"
    "para2(int32_t) is second integer\r\n"
    "para3(double) is first double\r\n"
    "para4(double) is second double\r\n"
    "e.g. add_demo(-15, 0x15, -12.5, 30.25)"
);
```

```c
> add_demo(-15, 5, -12.5, 30.25)
-12.500000+30.250000=17.750000
retValue: -10
```
