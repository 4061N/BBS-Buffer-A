#pragma once
#include "ch32v20x.h"
#include "stdlib.h"
#include "Debug_log.h"
#include "time64.h"
#include "ADC_DMA.h"


#define delay_any_us(time)\
{\
    const uint64_t _delay_any_div_time =(uint64_t)(8000000.0/time);\
    SysTick->SR &= ~(1 << 0);\
    SysTick->CMP = SystemCoreClock/_delay_any_div_time;\
    SysTick->CTLR |= (1 << 5) |(1 << 4)| (1 << 0);\
\
    while(!(SysTick->SR & 1));\
    SysTick->CTLR &= ~(1 << 0);\
}

#define delay_any_ms(time)\
{\
    const uint64_t _delay_any_div_time =(uint64_t)(80000.0/time);\
    SysTick->SR &= ~(1 << 0);\
    SysTick->CMP = SystemCoreClock/_delay_any_div_time;\
    SysTick->CTLR |= (1 << 5) |(1 << 4)| (1 << 0);\
\
    while(!(SysTick->SR & 1));\
    SysTick->CTLR &= ~(1 << 0);\
}
extern void RGB_set(unsigned char CHx,unsigned char R, unsigned char G, unsigned char B);

//#include "AMCU.h"
