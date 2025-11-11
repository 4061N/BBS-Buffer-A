#include "WS2812.h"
#include <stdlib.h>

#define RGB_H() port->BSHR = pin
#define RGB_L() port->BCR = pin

void WS2812_class::init(unsigned char _num, GPIO_TypeDef *_port, uint16_t _pin)
{
    num = _num;
    RGB_buf = new uint32_t[num * 3];
    port = _port;
    pin = _pin;
    RGB_L();
    if (port == GPIOA)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }
    else if (port == GPIOB)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
    else if (port == GPIOC)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }
    else if (port == GPIOD)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
    else if (port == GPIOE)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    }

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(port, &GPIO_InitStructure);
    clear();
}
WS2812_class::~WS2812_class()
{
    delete RGB_buf;
}

void WS2812_class::clear(void)
{
    unsigned char i;
    for (i = 0; i < num * 3; i++)
    {
        RGB_buf[i] = 0;
    }
}
void WS2812_class::RST(void)
{
    RGB_L();

    for (int i = 0; i < 10000; i++)
    {
        __NOP();
    }
}

void WS2812_class::updata()
{
    int i, j;
    int max = num * 3;
    uint32_t DATA;
    for (i = 0; i < max; i++)
    {
        DATA = RGB_buf[i];
        for (j = 0; j < 24; j++)
        {
            if ((DATA >> j) & 0x01)
                RGB_H();
            else
                RGB_L();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP(); //@144Mhz
        }
    }
    RST();
}
void WS2812_class::set_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned char index)
{
    uint32_t DATA = 0;
    int i;
    for (i = 0; i < 8; i++)
    {
        DATA <<= 3;
        DATA |= 1;
        DATA |= ((G >> i) & 1) << 1;
    }
    RGB_buf[index + 0] = DATA;
    DATA = 0;
    for (i = 0; i < 8; i++)
    {
        DATA <<= 3;
        DATA |= 1;
        DATA |= ((R >> i) & 1) << 1;
    }
    RGB_buf[index + 1] = DATA;
    DATA = 0;
    for (i = 0; i < 8; i++)
    {
        DATA <<= 3;
        DATA |= 1;
        DATA |= ((B >> i) & 1) << 1;
    }
    RGB_buf[index + 2] = DATA;
}
