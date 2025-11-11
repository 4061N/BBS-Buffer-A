#include "main.h"

#include "WS2812.h"
#include <cmath>
WS2812_class SYS_RGB;

void RGB_init()
{
    SYS_RGB.init(1, GPIOD, GPIO_Pin_1);
}
void RGB_update()
{
    SYS_RGB.updata();
}
// 系统时钟初始化函数：使用16MHz HSE或者8M HSI作为时钟源，配置系统时钟为144MHz
// DEBUG：暂定采用HSI
static void SysClock144M_init()
{

    // 设置HSI作为系统时钟(不分频)
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* HCLK = SYSCLK */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* PCLK1 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV1;

    /* 配置PLL: PLLCLK = HSI(8M) * 18 = 144 MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL18); // HSI不分频或二分频送入PLL，倍频18

    /* 使能PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待PLL就绪 */
    while ((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 选择PLL作为系统时钟源 */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待PLL作为系统时钟源 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
    SystemCoreClock = 144000000;
}
#define MC_PWM_MAX 1440
// 设置PWM驱动电机
void MC_PWM_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // 开启TIM3时钟

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 定时器基础配置
    TIM_TimeBaseStructure.TIM_Period = MC_PWM_MAX-1;  // 周期（x+1）
    TIM_TimeBaseStructure.TIM_Prescaler = 9; // 预分频（x+1）
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // PWM模式配置
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; // 占空比
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC4Init(TIM3, &TIM_OCInitStructure); // PB1

    TIM_CtrlPWMOutputs(TIM3, ENABLE);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}



float GET_duty_by_HALL_ADC(float HALL_ADC_value)
{
    const float HALL_ADC_min = 2.4;
    const float HALL_ADC_max = 0.9;
    float duty=(HALL_ADC_value - HALL_ADC_min) / (HALL_ADC_max - HALL_ADC_min);
    if(duty<0)
    {
        duty=0;
    }
    else if(duty>1)
    {
        duty=1;
    }
    return duty;
}

void set_RGB_by_duty(float duty)
{
    int R=((duty-0.33)*2*255);
    int B=((0.66-duty)*2*255);
    int G=((duty-0.5)*0.3*255);
    if(G<0)
    {
        G=-G;
    }
    if(G>255)
    {
        G=255;
    }
    if(R<0)
    {
        R=0;
    }
    if(R>255)
    {
        R=255;
    }
    if(B<0)
    {
        B=0;
    }
    if(B>255)
    {
        B=255;
    }
    SYS_RGB.set_RGB(R>>3, G>>3, B>>3, 0);
}

void duty_to_PWM(float duty)
{
    uint16_t duty_PWM=uint16_t(duty*MC_PWM_MAX);
    if(duty_PWM>MC_PWM_MAX*0.99)
    {
        duty_PWM=MC_PWM_MAX*0.99;
    }
    if(duty_PWM<MC_PWM_MAX*0.01)
    {
        duty_PWM=MC_PWM_MAX*0.01;
    }
    TIM_SetCompare4(TIM3, duty_PWM);
}

int main(void)
{
    // 关闭看门狗
    WWDG_DeInit();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    SysClock144M_init();
    systick_time64_init();
    RGB_init();
    SYS_RGB.set_RGB(0x01, 0x01, 0x01, 0);
    RGB_update();
    MC_PWM_init();
    
    ADC_DMA_init();
    while (1)
    {
        float *ADC_value = ADC_DMA_get_value();
        duty_to_PWM(GET_duty_by_HALL_ADC(ADC_value[0]));
        set_RGB_by_duty(GET_duty_by_HALL_ADC(ADC_value[0]));
        RGB_update();
    }
    return 0;
}