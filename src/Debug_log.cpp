#include "Debug_log.h"
#include <inttypes.h>
#include <cstring>

#ifdef Debug_log_on
//mbed::Timer USB_debug_timer;
DMA_InitTypeDef Debug_log_DMA_InitStructure;
bool if_Debug_send_idle = true;

void Debug_log_init()
{


    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef  NVIC_InitStructure = {0};

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // USART3 TX-->B.10  RX-->B.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    USART_InitStructure.USART_BaudRate = Debug_log_baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_Even;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Configure DMA1 channel 2 for USART3 TX
    Debug_log_DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DATAR;
    Debug_log_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;
    Debug_log_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    Debug_log_DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    Debug_log_DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    Debug_log_DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    Debug_log_DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    Debug_log_DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    Debug_log_DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    Debug_log_DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    Debug_log_DMA_InitStructure.DMA_BufferSize = 0;

    USART_Cmd(USART3, ENABLE);

    if_Debug_send_idle = true;
}

uint64_t Debug_log_count64()
{
    return 0;
}

void Debug_log_time()
{

}

void Debug_log_write(const void *data)
{

    int i = strlen((const char*)data);
    Debug_log_write_num((const char*)data,i);

}
uint8_t buf[1024];
bool Debug_log_using = false;
void Debug_log_write_num(const void *data, int num)
{
    // 等待上一个传输完成
    //while(DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);
    if(Debug_log_using)
    {
        return;
    }
    Debug_log_using = true;
    if(!if_Debug_send_idle)
    {
        memcpy(buf,data,num);
        data = buf;
    }
    while(!if_Debug_send_idle);
    if_Debug_send_idle = false;

    DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_DeInit(DMA1_Channel2);
    // Configure DMA1 channel 2 for USART3 TX
    Debug_log_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data;
    Debug_log_DMA_InitStructure.DMA_BufferSize = num;
    DMA_Init(DMA1_Channel2, &Debug_log_DMA_InitStructure);
    DMA_Cmd(DMA1_Channel2, ENABLE);
    // 使能USART3 DMA发送
    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
    Debug_log_using = false;
/*

    DMA1_Channel2->CFGR &= (uint16_t)(~DMA_CFGR1_EN); // 关闭通道
    DMA1_Channel2->MADDR = (uint32_t)data;            // 更新内存地址
    DMA1_Channel2->CNTR = num;                     // 更新传输长度
    DMA1_Channel2->CFGR |= DMA_CFGR1_EN;              // 重新使能
    // Enable USART3 DMA send
    USART3->CTLR3 |= USART_DMAReq_Tx;*/
}


extern "C" void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_TC) != RESET) // DMA-USART3 Tx send over
    {
        USART_ClearITPendingBit(USART3, USART_IT_TC);
        if_Debug_send_idle = true;
    }
}
#endif