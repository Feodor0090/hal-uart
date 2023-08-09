#include <hal_uart.h>

void HAL_UART_EnableQuick(HAL_UART_Type *dev, uint32_t baseFreq, uint32_t bod)
{
    if (!dev)
        return;
    if (baseFreq == 0)
        baseFreq = 32000000;
    HAL_UART_Reset(dev);
    dev->DIVIDER = (baseFreq / bod);
    dev->CONTROL1.RE = 1;
    dev->CONTROL1.TE = 1;
    dev->CONTROL1.UE = 1;
    while (!(dev->FLAGS.TEACK))
        ;
    while (!(dev->FLAGS.REACK))
        ;
}
void HAL_UART_Enable(HAL_UART_Type *dev, UART_InitData *init)
{
    if (!dev)
        return;
    if (!init)
        return;
    HAL_UART_Reset(dev);
    // basic setup
    dev->DIVIDER = (init->baseFreq / init->bod);
    dev->CONTROL1.M0 = init->frameLength & 1;
    dev->CONTROL1.M1 = (init->frameLength >> 1) & 1;
    dev->CONTROL1.PCE = init->parityBit & 1;
    dev->CONTROL1.PS = (init->parityBit >> 1) & 1;
    dev->CONTROL2.MSBFIRST = init->firstBit & 1;
    dev->CONTROL2.DATAINV = init->dataPolarity & 1;
    dev->CONTROL2.TXINV = init->txPolarity & 1;
    dev->CONTROL2.RXINV = init->rxPolarity & 1;
    dev->CONTROL2.SWAP = init->swap;
    dev->CONTROL2.STOP = init->stopType & 1;
    // clock
    dev->CONTROL2.CLKEN = init->clock.enabled;
    dev->CONTROL2.OCPL = init->clock.idlePolarity;
    dev->CONTROL2.CPHA = init->clock.firstTick;
    dev->CONTROL2.LBCL = init->clock.lastTick;
    // signals
    dev->CONTROL3.CTSE = init->enableCTS;
    dev->CONTROL3.RTSE = init->enableRTS;
    // waking up
    if (init->dirs != TX_ONLY)
    {
        // we need RX
        dev->CONTROL1.RE = 1;
    }
    if (init->dirs != RX_ONLY)
    {
        // we need TX
        dev->CONTROL1.TE = 1;
    }
    dev->CONTROL1.UE = 1; // powering up
    if (init->dirs != TX_ONLY)
    {
        // we need RX
        while (!(dev->FLAGS.REACK))
            ;
    }
    if (init->dirs != RX_ONLY)
    {
        // we need TX
        while (!(dev->FLAGS.TEACK))
            ;
    }
}

void HAL_UART_Disable(HAL_UART_Type *dev)
{
    if (!dev)
        return;
    dev->CONTROL1.UE = 0;
}

void HAL_UART_Reset(HAL_UART_Type *dev)
{
    if (!dev)
        return;
    HAL_UART_Disable(dev);
    dev->CONTROL1.value = 0;
    dev->CONTROL2.value = 0;
    dev->CONTROL3.value = 0;
    dev->DIVIDER = 0;
}

uint8_t HAL_UART_Send(HAL_UART_Type *dev, uint16_t val)
{
    if (!dev)
        return 1;
    dev->TXDATA = val;
    for (unsigned i = 0; i < TIMEOUT_TICKS; i++)
    {
        if (dev->FLAGS.TC)
        {
            return 0;
        }
    }
    return 1;
}

uint8_t HAL_UART_Send8(HAL_UART_Type *dev, uint8_t *buffer, unsigned count)
{
    if (!buffer)
        return 1;
    for (unsigned i = 0; i < count; i++)
    {
        if (HAL_UART_Send(dev, (uint16_t)buffer[i]))
            return 1;
    }
    return 0;
}

uint8_t HAL_UART_Send16(HAL_UART_Type *dev, uint16_t *buffer, unsigned count)
{
    if (!buffer)
        return 1;
    for (unsigned i = 0; i < count; i++)
    {
        if (HAL_UART_Send(dev, buffer[i]))
            return 1;
    }
    return 0;
}

uint8_t HAL_UART_SendNT(HAL_UART_Type *dev, char *string)
{
    if (!string)
        return 1;
    unsigned i = 0;
    while (string[i] != 0)
    {
        if (HAL_UART_Send(dev, string[i]))
            return 1;
        i++;
    }
    return 0;
}

uint16_t HAL_UART_Receive(HAL_UART_Type *dev)
{
    // блокирующее получение
    if (!dev)
        return 0;
    while (!HAL_UART_HasInput(dev))
        ;
    return HAL_UART_Read(dev);
}

uint16_t HAL_UART_Receive_t(HAL_UART_Type *dev, unsigned timeout, uint8_t *status)
{
    // получение через таймаут
    if (!dev)
    {
        *status = 1;
        return 0;
    }
    for (unsigned i = 0; i < timeout; i++)
    {
        if (HAL_UART_HasInput(dev))
            return HAL_UART_Read(dev);
    }
    *status = 1;
    return 0;
}

uint8_t HAL_UART_Receive8(HAL_UART_Type *dev, uint8_t *buf, unsigned count)
{
    if (!buf)
        return;
    uint8_t stat = 0;
    for (unsigned i = 0; i < count; i++)
    {
        buf[i] = (uint8_t)HAL_UART_Receive_t(dev, TIMEOUT_TICKS, &stat);
        if (stat)
            return 1;
    }
    return 0;
}

uint8_t HAL_UART_Receive16(HAL_UART_Type *dev, uint16_t *buf, unsigned count)
{
    if (!buf)
        return 1;
    uint8_t stat = 0;
    for (unsigned i = 0; i < count; i++)
    {
        buf[i] = HAL_UART_Receive_t(dev, TIMEOUT_TICKS, &stat);
        if (stat)
            return 1;
    }
    return 0;
}

int HAL_UART_Receive8Until(HAL_UART_Type *dev, uint8_t breakChar, uint8_t *buf, int maxCount, bool keepTerm, bool processBackspace)
{
    if (!dev)
        return 0;
    if (!buf)
        return 0; // длина строк

    if (keepTerm)
        maxCount--; // предпоследний элемент будет занят
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
        if (processBackspace && (next == 8)) // 8 = control BS
        {
            if (i > 0)
                i--;
            continue;
        }
        buf[i] = next;
        i++;
        if (i == maxCount - 1) // последний элемент будет нулём
        {
            buf[i] = 0;
            return -1;
        }
    }
}

int HAL_UART_Echo8Until(HAL_UART_Type *dev, uint8_t breakChar, uint8_t *buf, int maxCount, bool keepTerm)
{
    if (!dev)
        return 0;
    if (!buf)
        return 0; // длина строк
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
                // затирание символа
                HAL_UART_Send(dev, 8);
                HAL_UART_Send(dev, ' ');
                // возврат назад
                HAL_UART_Send(dev, 8);
                i--;
            }
            continue;
        }
        buf[i] = next;
        i++;
        HAL_UART_Send(dev, next);
        if (i == maxCount - 1) // последний элемент будет нулём
        {
            buf[i] = 0;
            return -1;
        }
    }
}

int HAL_UART_ReceiveAsciiInt(HAL_UART_Type *dev, uint8_t breakChar, bool echo)
{
    if (!dev)
        return 0;
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
        if (len == 0 // знак можно поставить только перед числом
            && next == '-')
        {
            negative = true;
            if (echo)
                HAL_UART_Send(dev, '-');
            continue;
        }
        if (next == 8)
        { // стирание
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
                // затирание символа
                HAL_UART_Send(dev, 8);
                HAL_UART_Send(dev, ' ');
                // возврат назад
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
        } // всё кроме дефиса, чисел и дефиса игнорируется
    }
}

uint8_t HAL_UART_SendAsciiInt(HAL_UART_Type *dev, int num)
{
    if (!dev)
        return 1;
    uint8_t status = 0;
    bool neg = num < 0;
    if (neg)
        num = -num; // минимальное число не будет обработано корректно (а надо?)
    unsigned char str[16]; // FILO кэш. В мин инте 11 цифр (-2147483648), 16 хватит.
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
        status |= HAL_UART_Send(dev, '-');
    }

    do
    {
        i--;
        status |= HAL_UART_Send(dev, (uint16_t)(str[i]));
    } while (i > 0);
    return status;
}