#include <ti/devices/msp432e4/driverlib/rom_map.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <math.h>

#define USART_TX_LEN 31  // 数据包长度：包头(1) + 6个浮点数(24) + 正弦波数据(4) + 校验和(1) + 包尾(1)
#define NUM_POINTS 1  // 每次发送一个正弦波数据点

// 数据包缓存区
uint8_t USART_TX_BUF[USART_TX_LEN];

// 正弦波数据
float sine_data;

// 时间步进变量
float x = 0.0f;

// 通过 UART 发送数据到蓝牙模块
void sendBluetoothData(const uint8_t *data, uint32_t length) {
    uint32_t i;
    for (i = 0; i < length; i++) {
        while (UARTSpaceAvail(UART0_BASE) == 0); // 等待发送缓冲区有空间
        UARTCharPutNonBlocking(UART0_BASE, data[i]);
    }
    while (UARTBusy(UART0_BASE)); // 等待发送完成
}

// 重定向 printf 到 UART2
int fputc(int c, FILE* stream) {
    UARTCharPutNonBlocking(UART2_BASE, c);
    return c;
}

int fputs(const char* restrict s, FILE* restrict stream) {
    uint16_t i, len;
    len = strlen(s);
    for (i = 0; i < len; i++) {
        UARTCharPutNonBlocking(UART2_BASE, s[i]);
    }
    return len;
}

int puts(const char *_ptr) {
    int count = fputs(_ptr, stdout);
    count += fputs("\n", stdout);
    return count;
}

int main(void) {
    // 设置系统时钟
    uint32_t g_ui32SysClock;
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                         SYSCTL_OSC_MAIN |
                                         SYSCTL_USE_PLL |
                                         SYSCTL_CFG_VCO_480), 160000000);

    // 启用 GPIOA 作为 UART0 端口
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // 启用 UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTConfigSetExpClk(UART0_BASE, g_ui32SysClock, 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    // 启用 GPIOF 作为 LED 端口（假设 LED 连接在 GPIO_PIN_0）
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

    // 启用 GPIOC 作为 UART2 端口
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinConfigure(GPIO_PA6_U2RX);  // 确保这些宏在头文件中定义
    GPIOPinConfigure(GPIO_PA7_U2TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    // 启用 UART2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    UARTConfigSetExpClk(UART2_BASE, g_ui32SysClock, 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    // 初始化数据包
    USART_TX_BUF[0] = 0xA5;  // 数据包头
    *((float *)(&USART_TX_BUF[1])) = 123.45; // float值
    *((float *)(&USART_TX_BUF[5])) = 65.78; // float值
    *((float *)(&USART_TX_BUF[9])) = 30.67; // float值
    *((float *)(&USART_TX_BUF[13])) = 15.25; // float值
    *((float *)(&USART_TX_BUF[17])) = 8.99; // float值
    *((float *)(&USART_TX_BUF[21])) = 0.08; // float值

    // 主循环
    while (1) {
        // 发送简单的“hello”消息到蓝牙
        const char hello_msg[] = "Hello, Bluetooth!\n";
        sendBluetoothData((const uint8_t *)hello_msg, strlen(hello_msg));

        // 更新正弦波数据（随时间步进）
        if (x >= 6.28) {
            x = 0.0f; // 重置时间步进
        }
        sine_data = 127.0f * sinf(x); // 计算正弦值
        x += 0.05f; // 时间步进

        // 更新数据包中的正弦波数据
        memcpy(&USART_TX_BUF[25], &sine_data, 4);

        // 计算校验和
        uint8_t checksum = 0;
        for (uint8_t i = 1; i < USART_TX_LEN - 2; i++) {  // 从第二个字节到倒数第二个字节（不包括包尾）
            checksum += USART_TX_BUF[i];
        }
        USART_TX_BUF[USART_TX_LEN - 2] = checksum; // 校验位

        // 添加包尾
        USART_TX_BUF[USART_TX_LEN - 1] = 0x5A; // 包尾字节

        // 发送数据包到蓝牙
        sendBluetoothData(USART_TX_BUF, USART_TX_LEN);

        // LED 闪烁测试
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
        SysCtlDelay(g_ui32SysClock / 12000);  // 延时大约 10 毫秒

        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
        SysCtlDelay(g_ui32SysClock / 12000);  // 延时大约 10 毫秒
    }
}

