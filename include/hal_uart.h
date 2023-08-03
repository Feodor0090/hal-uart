#ifndef _HAL_UART
#define _HAL_UART

#include <mcu32_memory_map.h>
#include <stdbool.h>
#include <hal_uart_types.h>

// 1 кадр имеет длину 7 бит.
#define FRAME_7BITS 2
// 1 кадр имеет длину 8 бит.
#define FRAME_8BITS 0
// 1 кадр имеет длину 9 бит.
#define FRAME_9BITS 1

// Запускается и передатчик, и приёмник.
#define TXRX 0
// Запускается только передатчик.
#define TX_ONLY 1
// Запускается только приёмник.
#define RX_ONLY 2

// Бит чётности выключен.
#define PB_DISABLED 0b00
// Используется бит чётности.
#define PB_ENABLE 0b01
// Используется бит нечётности.
#define PB_INVERTED 0b11

#define LSBF 0
#define MSBF 1

#define POLARITY_DEFAULT 0
#define POLARITY_INVERTED 1

// Используется 1 стоп-бит.
#define STOP_1 0
// Используется 2 стоп-бита.
#define STOP_2 1

#define UART_P0 ((HAL_UART_Type *)UART_0_BASE_ADDRESS)
#define UART_P1 ((HAL_UART_Type *)UART_1_BASE_ADDRESS)

typedef struct
{
    // Дескриптор устройства.
    HAL_UART_Type *dev;
    // Базовая частота (обычно 32_000_000).
    uint32_t baseFreq;
    // Скорость интерфейса в бодах. Будет округлена в большую сторону.
    uint32_t bod;
    // Одно из значений TXRX, TX_ONLY, RX_ONLY.
    uint8_t dirs;
    /**
     * Одно из значений FRAME_7BITS, FRAME_8BITS, FRAME_9BITS.
     * Использование 9-битных передач требует использования Send16/Receive16 методов,
     * в противном случае старший бит будет отбрасываться. При использовании битов
     * чётности требуется установить длину кадра на 1 бит длиннее реальной (бит вставляется в кадр).
     */
    uint8_t frameLength;
    // Одно из значений PB_DISABLED, PB_ENABLE, PB_INVERTED.
    uint8_t parityBit;
    // Одно из значений LSBF, MSBF.
    uint8_t firstBit;
    // Одно из значений POLARITY_DEFAULT, POLARITY_INVERTED.
    uint8_t dataPolarity;
    // Одно из значений POLARITY_DEFAULT, POLARITY_INVERTED.
    uint8_t txPolarity;
    // Одно из значений POLARITY_DEFAULT, POLARITY_INVERTED.
    uint8_t rxPolarity;
    // true для обмена RX и TX выходов местами.
    bool swap;
    // Одно из значений STOP_1, STOP_2.
    uint8_t stopType;
    // Параметры тактирования.
    UsartClockSetup clock;
    // true для включения.
    bool enableCTS;
    // true для включения.
    bool enableRTS;
} UART_InitData;

/**
 * Включает приёмник и передатчик с настройками по умолчанию.
 *
 * \param dev Дескриптор устройства.
 * \param baseFreq Базовая частота (обычно 32_000_000).
 * \param bod Скорость интерфейса в бодах. Будет округлена в большую сторону.
 */
void HAL_UART_EnableQuick(HAL_UART_Type *dev, uint32_t baseFreq, uint32_t bod);
/**
 * Приводит настройки модуля и включает его.
 *
 * \param dev Дескриптор устройства.
 * \param init Данные для инициализации.
 */
void HAL_UART_Enable(HAL_UART_Type *dev, UART_InitData* init);
/**
 * Выключает устройство.
 *
 * \param dev Дескриптор устройства.
 */
void HAL_UART_Disable(HAL_UART_Type *dev);
/**
 * Сбрасывает регистры устройства.
 *
 * \param dev Дескриптор устройства.
 */
void HAL_UART_Reset(HAL_UART_Type *dev);

/**
 * Отправляет один кадр (7-9 бит). Старшие биты игнорируются.
 *
 * \param dev Дескриптор устройства.
 * \param val Байт/слово для отправки.
 */
void HAL_UART_Send(HAL_UART_Type *dev, uint16_t val);
/**
 * Отправляет буфер данных (7-8 бит на кадр).
 *
 * \param dev Дескриптор устройства.
 * \param buffer Буфер.
 * \param count Длина буфера.
 */
void HAL_UART_Send8(HAL_UART_Type *dev, uint8_t *buffer, unsigned count);
/**
 * Отправляет буфер данных (9 бит на кадр, старшие биты игнорируются).
 *
 * \param dev Дескриптор устройства.
 * \param buffer Буфер.
 * \param count Длина буфера.
 */
void HAL_UART_Send16(HAL_UART_Type *dev, uint16_t *buffer, unsigned count);
/**
 * Отправляет null-терминированный буфер (строку).
 *
 * \param dev Дескриптор устройства.
 * \param string Буфер.
 */
void HAL_UART_SendNT(HAL_UART_Type *dev, char *string);

/**
 * Проверяет, доступны ли данные для чтения. Вернёт 0, если читать нечего, 1 если кадр ждёт принятия.
 *
 * \param dev Дескриптор устройства.
 */
#define HAL_UART_HasInput(dev) (dev->FLAGS.RXNE)
/**
 * Возвращает кадр входящих данных.
 *
 * \param dev Дескриптор устройства.
 */
#define HAL_UART_Read(dev) (dev->RXDATA)
/**
 * Ждёт прибытия данных и возвращает 1 кадр.
 *
 * \param dev Дескриптор устройства.
 */
uint16_t HAL_UART_Receive(HAL_UART_Type *dev);
/**
 * Принимает буфер данных (7-8 бит на кадр).
 *
 * \param dev Дескриптор устройства.
 * \param buf Буфер.
 * \param count Длина буфера.
 */
void HAL_UART_Receive8(HAL_UART_Type *dev, uint8_t *buf, unsigned count);
/**
 * Принимает буфер данных (9 бит на кадр, старшие биты игнорируются).
 *
 * \param dev Дескриптор устройства.
 * \param buf Буфер.
 * \param count Длина буфера.
 */
void HAL_UART_Receive16(HAL_UART_Type *dev, uint16_t *buf, unsigned count);

/**
 * Читает входящие данные до символа-разделителя. Вернёт длину считанной строки. Вернёт -1, если длины буфера не хватило.
 *
 * \param dev Дескриптор устройства.
 * \param breakChar Символ, на котором чтение заканчивается. Например, '\\n' или '\\0'.
 * \param buf Буфер.
 * \param maxCount Длина буфера.
 * \param keepTerm Записать символ остановки в буфер. Например, входящие данные - '1','2','\\n', разделитель - '\\n'.
 * Если true, в буфер будет записано '1','2','\\n','\\0' и 3 будет возвращено. Если false, '\\n' будет опущено и 2 возвращено.
 * \param processBackspace Если false, входящая 8 будет записана в буфер. Если true, входящая 8 вернёт позицию буфера на 1 символ назад.
 */
int HAL_UART_Receive8Until(HAL_UART_Type *dev, uint8_t breakChar, uint8_t *buf, int maxCount, bool keepTerm, bool processBackspace);

/**
 * Читает входящие данные до символа-разделителя. Отправляет входящие данные обратно. Обрабатывает символ возврата. Вернёт длину считанной строки. Вернёт -1, если длины буфера не хватило.
 *
 * \param dev Дескриптор устройства.
 * \param breakChar Символ, на котором чтение заканчивается. Например, '\\n' или '\\0'.
 * \param buf Буфер.
 * \param maxCount Длина буфера.
 * \param keepTerm Записать символ остановки в буфер. Например, входящие данные - '1','2','\\n', разделитель - '\\n'.
 * Если true, в буфер будет записано '1','2','\\n','\\0' и 3 будет возвращено. Если false, '\\n' будет опущено и 2 возвращено.
 */
int HAL_UART_Echo8Until(HAL_UART_Type *dev, uint8_t breakChar, uint8_t *buf, int maxCount, bool keepTerm);

/**
 * Читает входящее число, записанное ASCII символами. Обрабатывает отрицательные числа и символ возврата.
 *
 * \param dev Дескриптор устройства.
 * \param breakChar Символ, на котором чтение заканчивается. Например, '\\n' или '\\0'.
 * \param echo Отпраавлять ввод обратно - true.
 */
int HAL_UART_ReceiveAsciiInt(HAL_UART_Type *dev, uint8_t breakChar, bool echo);

void HAL_UART_SendAsciiInt(HAL_UART_Type *dev, int num);

#define HAL_UART_SetDtr(dev, ready) dev->MODEM.DTR = ready & 1;
#define HAL_UART_GetDsr(dev) (dev->MODEM.DSR)

#endif