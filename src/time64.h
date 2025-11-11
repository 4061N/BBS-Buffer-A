
#pragma once
#include "main.h"
extern void systick_time64_init();//初始化time64，使用systick作为时钟源
extern uint64_t get_time64();//返回ms单位的时间
extern void delay_ms(uint64_t ms);//延时ms单位的时间
extern void delay_us(uint64_t us);//延时us单位的时间
