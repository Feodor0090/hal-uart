Библиотека для синхронной работы с USART на mik32.

## Использование
### Пример эха
```c
#include <hal_uart.h>
#include <power_manager.h>
// Тактирование управлений
PM->CLK_APB_M_SET |= PM_CLOCK_PAD_CONFIG_M | PM_CLOCK_WU_M | PM_CLOCK_PM_M;
// Тактирование USART
PM->CLK_APB_P_SET |= PM_CLOCK_UART_0_M;
// Включение модуля
HAL_UART_EnableQuick(UART_P0, 115200);
// Эхо
while(true) HAL_UART_Send(UART_P0, HAL_UART_Receive(UART_P0));
```
### Пример калькулятора
```c
#include <hal_uart.h>
#include <power_manager.h>
#include <xprintf.h>

int main()
{
    PM->CLK_APB_M_SET |= PM_CLOCK_PAD_CONFIG_M | PM_CLOCK_WU_M | PM_CLOCK_PM_M;
    PM->CLK_APB_P_SET |= PM_CLOCK_UART_0_M;
    HAL_UART_EnableQuick(UART_P0, 115200);
    while (1)
    {
        HAL_UART_SendNT(UART_P0, "Enter left operand: ");
        int left = HAL_UART_ReceiveAsciiInt(UART_P0, '\n');
        HAL_UART_SendNT(UART_P0, "\nEnter right operand: ");
        int right = HAL_UART_ReceiveAsciiInt(UART_P0, '\n');
        HAL_UART_SendNT(UART_P0, "\n = ");
        HAL_UART_SendAsciiInt(UART_P0, left+right);
        HAL_UART_Send(UART_P0, '\n');
    }
}
```
