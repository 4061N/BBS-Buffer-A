#include "main.h"

//初始化time64，使用systick作为时钟源
//芯片自带64位systick计数器，可以将其作为64位时钟源，设置时钟为144M/8=18MHz
//溢出时间为2^64/18M≈1024819115206秒≈11861332天≈32496年
void systick_time64_init()
{
    SysTick->CNT=0;//清空计数器
    SysTick->CMP=0xFFFFFFFFFFFFFFFF;//初始化比较寄存器为最大值
    SysTick->CTLR=1;//设置SysTick启动，时钟为HCLK/8，关闭中断和自动更新计数
}
//返回ms单位的时间
uint64_t get_time64()
{
    return (SysTick->CNT)/(18000);
}

//延时ms单位的时间
void delay_ms(uint64_t ms)
{
    uint64_t T=SysTick->CNT;
    uint64_t T_end=T+ms*18000;
    while(SysTick->CNT<T_end);
}
//延时us单位的时间
void delay_us(uint64_t us)
{
    uint64_t T=SysTick->CNT;
    uint64_t T_end=T+us*18;
    while(SysTick->CNT<T_end);
}
