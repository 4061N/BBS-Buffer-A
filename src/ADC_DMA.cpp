#include "ADC_DMA.h"
int16_t ADC_Calibrattion_Val = 0;
#define ADC_num 2 //转换数量，从ADC1的通道0开始计算
#define ADC_filter_n_pow 8        // 滑动窗口滤波的窗口长度(为2的几次方)
constexpr int mypow(int a, int b) // 定义一个函数用于简单计算次方
{
    int x = 1;
    while (b--)
        x *= a;
    return x;
}
constexpr const int ADC_filter_n = mypow(2, ADC_filter_n_pow); // 滑动窗口滤波的窗口长度n
// 说明：滑动滤波后信号的频率变为f/(2n),f约为8000,n=256,则滤波后变为15hz
uint16_t ADC_data[ADC_filter_n][ADC_num];
float ADC_V[ADC_num];

void ADC_DMA_init()
{
    // 设置IO模式
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        uint32_t pinmask=1;
        for(int i=0;i<ADC_num;i++)
        {
            pinmask|=1<<i;
        }
        GPIO_InitStructure.GPIO_Pin=pinmask;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    // 初始化DMA
    {
        DMA_InitTypeDef DMA_InitStructure;

        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        DMA_DeInit(DMA1_Channel1);
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->RDATAR;
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_data; // 数组名代表的是首项地址
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStructure.DMA_BufferSize = ADC_filter_n * ADC_num;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        DMA_Init(DMA1_Channel1, &DMA_InitStructure);

        DMA_Cmd(DMA1_Channel1, ENABLE); // 打开DMA
    }

    // 初始化ADC
    {
        ADC_DeInit(ADC1);
        RCC_ADCCLKConfig(RCC_PCLK2_Div8);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        ADC_InitTypeDef ADC_InitStructure;
        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ScanConvMode = ENABLE;
        ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
        ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfChannel = ADC_num;
        ADC_Init(ADC1, &ADC_InitStructure);

        ADC_Cmd(ADC1, ENABLE);
        ADC_BufferCmd(ADC1, DISABLE); // 关闭buff

        ADC_ResetCalibration(ADC1); // 重置ADC校准
        while (ADC_GetResetCalibrationStatus(ADC1))
            ;
        ADC_StartCalibration(ADC1); // 开始ADC校准
        while (ADC_GetCalibrationStatus(ADC1))
            ;
        ADC_Calibrattion_Val = Get_CalibrationValue(ADC1); // 保存ADC校准值
        for (int i = 0; i < ADC_num; i++)
            ADC_RegularChannelConfig(ADC1, i, i + 1, ADC_SampleTime_239Cycles5); // 设置ADC_num个通道为规则通道,约72KH转换一次
        ADC_DMACmd(ADC1, ENABLE);                                                // 打开ADC的DMA模式
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);                                  // 开启ADC转换
    }

    delay_ms(ADC_filter_n); // 等待至少8次缓冲区域满
}

float *ADC_DMA_get_value()
{
    for (int i = 0; i < ADC_num; i++)
    {
        int data_sum = 0;
        for (int j = 0; j < ADC_filter_n; j++)
        {
            uint16_t val = ADC_data[j][i]; // 第j次的i通道上的数据
            int sum = val + ADC_Calibrattion_Val;
            if (sum < 0 || val == 0)
                ; // data_sum += 0;
            else if (sum > 4095 || val == 4095)
                data_sum += 4095;
            else
                data_sum += sum;
        }
        data_sum >>= ADC_filter_n_pow;             // 取平均值
        ADC_V[i] = ((float)data_sum) / 4096 * 3.3; // 得到电压值
    }

    return ADC_V;
}