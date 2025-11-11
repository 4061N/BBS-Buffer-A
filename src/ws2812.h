#pragma once
#include "main.h"


class WS2812_class
{
public:
    uint32_t *RGB_buf;
	unsigned char num;
    uint32_t GPIO_pin;
    WS2812_class(){}
    ~WS2812_class();
    void init(unsigned char _num, GPIO_TypeDef * _port, uint16_t _pin);
	void clear( void );
	void RST( void );
	void updata();
	void set_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned char index);
private:
    GPIO_TypeDef * port;
    uint16_t pin;
};

