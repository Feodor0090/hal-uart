#include <hal_uart.h>
#include <xprintf.h>
#include <power_manager.h>

int main()
{
    PM->CLK_APB_M_SET |= PM_CLOCK_PAD_CONFIG_M | PM_CLOCK_WU_M | PM_CLOCK_PM_M;
    PM->CLK_APB_P_SET |= PM_CLOCK_UART_0_M;
    HAL_UART_EnableQuick(UART_P0, 115200);
    uint8_t buf8[] = {'a', 's', 'd', '\n'};
    unsigned i = 0;
    while (1)
    {
        HAL_UART_SendAsciiInt(UART_P0, 929);
        HAL_UART_Send(UART_P0, '\n');
        HAL_UART_SendAsciiInt(UART_P0, -129);
        HAL_UART_Send(UART_P0, '\n');
        HAL_UART_SendNT(UART_P0, "qwe\n");
        HAL_UART_Send8(UART_P0, buf8, 4);
        i++;
        for (volatile unsigned j = 0; j < 500000; j++)
            ;
    }
}