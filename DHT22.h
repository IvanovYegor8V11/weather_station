#ifndef __DHT22_H
#define __DHT22_H

#include "stm32f10x.h"
#include "delay.h"

void DelayMicro(__IO uint32_t micros);
void DHT22_Start(void);
void DHT22_Stop(void);
uint8_t DHT22_ReadBit(void);
uint8_t DHT22_ReadByte(void);

#endif
