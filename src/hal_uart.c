#include <hal_uart.h>

void HAL_UART_EnableQuick(HAL_UART_Type *dev, uint32_t bod)
{
    HAL_UART_Reset(dev);
    dev->DIVIDER = (32000000 / bod);
    dev->CONTROL1.RE = 1;
    dev->CONTROL1.TE = 1;
    dev->CONTROL1.UE = 1;
    while (!(dev->FLAGS.TEACK))
        ;
    while (!(dev->FLAGS.REACK))
        ;
}
void HAL_UART_Enable(HAL_UART_Type *dev, uint32_t bod, uint8_t dirs,
                    uint8_t frameLength, uint8_t parityBit, uint8_t firstBit,
                    uint8_t dataPolarity, uint8_t txPolarity, uint8_t rxPolarity,
                    bool swap, uint8_t stopType, UsartClockSetup clock,
                    bool enableCTS, bool enableRTS)
{
    HAL_UART_Reset(dev);
    // basic setup
    dev->DIVIDER = (32000000 / bod);
    dev->CONTROL1.M0 = frameLength & 1;
    dev->CONTROL1.M1 = (frameLength >> 1) & 1;
    dev->CONTROL1.PCE = parityBit & 1;
    dev->CONTROL1.PS = (parityBit >> 1) & 1;
    dev->CONTROL2.MSBFIRST = firstBit & 1;
    dev->CONTROL2.DATAINV = dataPolarity & 1;
    dev->CONTROL2.TXINV = txPolarity & 1;
    dev->CONTROL2.RXINV = rxPolarity & 1;
    dev->CONTROL2.SWAP = swap;
    dev->CONTROL2.STOP = stopType & 1;
    // clock
    dev->CONTROL2.CLKEN = clock.enabled;
    dev->CONTROL2.OCPL = clock.idlePolarity;
    dev->CONTROL2.CPHA = clock.firstTick;
    dev->CONTROL2.LBCL = clock.lastTick;
    // signals
    dev->CONTROL3.CTSE = enableCTS;
    dev->CONTROL3.RTSE = enableRTS;
    // waking up
    if (dirs != TX_ONLY)
    {
        // we need RX
        dev->CONTROL1.RE = 1;
    }
    if (dirs != RX_ONLY)
    {
        // we need TX
        dev->CONTROL1.TE = 1;
    }
    dev->CONTROL1.UE = 1; // powering up
    if (dirs != TX_ONLY)
    {
        // we need RX
        while (!(dev->FLAGS.REACK))
            ;
    }
    if (dirs != RX_ONLY)
    {
        // we need TX
        while (!(dev->FLAGS.TEACK))
            ;
    }
}

void HAL_UART_Disable(HAL_UART_Type *dev)
{
    dev->CONTROL1.UE = 0;
}

void HAL_UART_Reset(HAL_UART_Type *dev)
{
    HAL_UART_Disable(dev);
    dev->CONTROL1.value = 0;
    dev->CONTROL2.value = 0;
    dev->CONTROL3.value = 0;
    dev->DIVIDER = 0;
}

void HAL_UART_Send(HAL_UART_Type *dev, uint16_t val)
{
    dev->TXDATA = val;
    while (!(dev->FLAGS.TC))
        ;
}

void HAL_UART_Send8(HAL_UART_Type *dev, uint8_t *buffer, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {
        HAL_UART_Send(dev, (uint16_t)buffer[i]);
    }
}

void HAL_UART_Send16(HAL_UART_Type *dev, uint16_t *buffer, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {
        HAL_UART_Send(dev, buffer[i]);
    }
}

void HAL_UART_SendNT(HAL_UART_Type *dev, char *string)
{
    unsigned i = 0;
    while (string[i] != 0)
    {
        HAL_UART_Send(dev, string[i]);
        i++;
    }
}

uint16_t HAL_UART_Receive(HAL_UART_Type *dev)
{
    while (!HAL_UART_HasInput(dev))
        ;
    return HAL_UART_Read(dev);
}

void HAL_UART_Receive8(HAL_UART_Type *dev, uint8_t *buf, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {
        buf[i] = (uint8_t)HAL_UART_Receive(dev);
    }
}

void HAL_UART_Receive16(HAL_UART_Type *dev, uint16_t *buf, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {
        buf[i] = HAL_UART_Receive(dev);
    }
}

int HAL_UART_Receive8Until(HAL_UART_Type *dev, uint8_t breakChar, uint8_t *buf, int maxCount, bool keepTerm, bool processBackspace)
{
    if (keepTerm)
        maxCount--;
    if (maxCount < 1)
        return -1;
    if (maxCount == 1)
    {
        buf[0] = 0;
        return -1;
    }

    int i = 0;

    while (1)
    {
        uint8_t next = (uint8_t)HAL_UART_Receive(dev);
        if (next == breakChar)
        {
            if (keepTerm)
            {
                buf[i] = next;
                i++;
            }
            buf[i] = 0;
            return i;
        }
        if (processBackspace && (next == 8))
        {
            if (i > 0)
                i--;
            continue;
        }
        buf[i] = next;
        i++;
        if (i == maxCount - 1)
        {
            buf[i] = 0;
            return -1;
        }
    }
}

int HAL_UART_Echo8Until(HAL_UART_Type *dev, uint8_t breakChar, uint8_t *buf, int maxCount, bool keepTerm)
{
    if (keepTerm)
        maxCount--;
    if (maxCount < 1)
        return -1;
    if (maxCount == 1)
    {
        buf[0] = 0;
        return -1;
    }

    int i = 0;

    while (1)
    {
        uint8_t next = (uint8_t)HAL_UART_Receive(dev);
        if (next == breakChar)
        {
            if (keepTerm)
            {
                buf[i] = next;
                i++;
            }
            buf[i] = 0;
            return i;
        }
        if (next == 8)
        {
            if (i > 0)
            {
                HAL_UART_Send(dev, 8);
                HAL_UART_Send(dev, ' ');
                HAL_UART_Send(dev, 8);
                i--;
            }
            continue;
        }
        buf[i] = next;
        i++;
        HAL_UART_Send(dev, next);
        if (i == maxCount - 1)
        {
            buf[i] = 0;
            return -1;
        }
    }
}

int HAL_UART_ReceiveAsciiInt(HAL_UART_Type *dev, uint8_t breakChar, bool echo)
{
    int len = 0;
    int number = 0;
    bool negative = false;
    while (1)
    {
        uint8_t next = (uint8_t)HAL_UART_Receive(dev);
        if (next == breakChar)
        {
            if (negative)
                return -number;
            return number;
        }
        if (len == 0 && next == '-')
        {
            negative = true;
            if (echo)
                HAL_UART_Send(dev, '-');
            continue;
        }
        if (next == 8)
        {
            if (len > 1)
            {
                number /= 10;
                len--;
            }
            else if (len == 1)
            {
                number = 0;
                len--;
            }
            else if (negative)
            {
                negative = false;
            }
            else
            {
                // nothing to erase
                continue;
            }
            // gets here if erase actually occurs
            if (echo)
            {
                HAL_UART_Send(dev, 8);
                HAL_UART_Send(dev, ' ');
                HAL_UART_Send(dev, 8);
            }
            continue;
        }
        if (next >= '0' && next <= '9')
        {
            number = (number * 10) + (next - '0');
            len++;
            if (echo)
                HAL_UART_Send(dev, next);
        }
    }
}

void HAL_UART_SendAsciiInt(HAL_UART_Type *dev, int num)
{
    bool neg = num < 0;
    if (neg)
        num = -num;
    unsigned char str[16];
    unsigned i = 0;
    while (num > 10)
    {
        str[i] = (num % 10) + '0';
        num /= 10;
        i++;
    }
    str[i] = num + '0';
    i++;
    if (neg)
    {
        HAL_UART_Send(dev, '-');
    }

    do
    {
        i--;
        HAL_UART_Send(dev, (uint16_t)(str[i]));
    } while (i > 0);
}