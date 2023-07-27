#ifndef _HAL_UART_T
#define _HAL_UART_T
#include <inttypes.h>

typedef union
{
    uint8_t value;
    struct
    {
        // 1 - последний тактовый импульс присутствует, 0 - отсутствует.
        unsigned lastTick : 1;
        // 1 - первый тик на срезе, 0 - первый тик на фронте.
        unsigned firstTick : 1;
        // 1 - логич. 1 вне транзакции, 0 - 0.
        unsigned idlePolarity : 1;
        // 1 - тактирование включено.
        unsigned enabled : 1;
    };
} UsartClockSetup;

typedef union
{
    volatile uint32_t value;
    volatile struct
    {
        volatile unsigned UE : 1;
        volatile unsigned _reserved0 : 1;
        volatile unsigned RE : 1;
        volatile unsigned TE : 1;
        volatile unsigned IDLEIE : 1;
        volatile unsigned RXNEIE : 1;
        volatile unsigned TCIE : 1;
        volatile unsigned TXEIE : 1;
        volatile unsigned PEIE : 1;
        volatile unsigned PS : 1;
        volatile unsigned PCE : 1;
        volatile unsigned _reserved1 : 1;
        volatile unsigned M0 : 1;
        volatile unsigned _reserved2 : 15;
        volatile unsigned M1 : 1;
        volatile unsigned _reserved3 : 3;
    };
} UART_CTRL1_Type;

typedef union
{
    volatile uint32_t value;
    volatile struct
    {
        volatile unsigned _reserved0 : 6;
        volatile unsigned LBDIE : 1;
        volatile unsigned _reserved1 : 1;
        volatile unsigned LBCL : 1;
        volatile unsigned CPHA : 1;
        volatile unsigned OCPL : 1;
        volatile unsigned CLKEN : 1;
        volatile unsigned _reserved2 : 1;
        volatile unsigned STOP : 1;
        volatile unsigned LBM : 1;
        volatile unsigned SWAP : 1;
        volatile unsigned RXINV : 1;
        volatile unsigned TXINV : 1;
        volatile unsigned DATAINV : 1;
        volatile unsigned MSBFIRST : 1;
    };
} UART_CTRL2_Type;

typedef union
{
    volatile uint32_t value;
    volatile struct
    {
        volatile unsigned EIE : 1;
        volatile unsigned _reserved0 : 1;
        volatile unsigned SBKRQ : 1;
        volatile unsigned HDSEL : 1;
        volatile unsigned _reserved1 : 2;
        volatile unsigned DMAR : 1;
        volatile unsigned DMAT : 1;
        volatile unsigned RTSE : 1;
        volatile unsigned CTSE : 1;
        volatile unsigned CTSIE : 1;
        volatile unsigned _reserved2 : 1;
        volatile unsigned OVRDIS : 1;
    };
} UART_CTRL3_Type;

typedef union
{
    volatile uint32_t value;
    volatile struct
    {
        volatile unsigned PE : 1;
        volatile unsigned FE : 1;
        volatile unsigned NF : 1;
        volatile unsigned ORE : 1;
        volatile unsigned IDLE : 1;
        volatile unsigned RXNE : 1;
        volatile unsigned TC : 1;
        volatile unsigned TXE : 1;
        volatile unsigned LBDF : 1;
        volatile unsigned CTSIF : 1;
        volatile unsigned CTS : 1;
        volatile unsigned _reserved0 : 5;
        volatile unsigned BUSY : 1;
        volatile unsigned _reserved1 : 4;
        volatile unsigned TEACK : 1;
        volatile unsigned REACK : 1;
    };
} UART_FLAGS_Type;

typedef union
{
    volatile uint32_t value;
    volatile struct
    {
        volatile unsigned _reserved0 : 1;
        volatile unsigned DSRIF : 1;
        volatile unsigned RIIF : 1;
        volatile unsigned DCDIF : 1;
        volatile unsigned _reserved1 : 1;
        volatile unsigned DSR : 1;
        volatile unsigned RI : 1;
        volatile unsigned DCD : 1;
        volatile unsigned DTR : 1;
    };
} UART_MODEM_Type;

typedef struct
{
    volatile UART_CTRL1_Type CONTROL1;
    volatile UART_CTRL2_Type CONTROL2;
    volatile UART_CTRL3_Type CONTROL3;
    volatile uint32_t DIVIDER;
    volatile uint32_t _reserved0;
    volatile uint32_t _reserved1;
    volatile uint32_t _reserved2;
    volatile UART_FLAGS_Type FLAGS;
    volatile uint32_t _reserved3;
    volatile uint32_t RXDATA;
    volatile uint32_t TXDATA;
    volatile UART_MODEM_Type MODEM;

} HAL_UART_Type;

#endif