#ifndef DS18B20_H_
#define DS18B20_H_
//--------------------------------------------------
#include "stm32f10x.h"
#include "delay.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
//--------------------------------------------------
#define SKIP_ROM 			0xCC
#define ROM_SEARCH 			0xF0
#define WRITE_SCRATCHPAD	0x4E
#define READ_SCRATCHPAD 	0xBE
#define CONVERT_TEMP		0x44
#define MATCH_CODE			0x55
#define READ_ROM			0x33

//--------------------------------------------------
#define RESOLUTION_9BIT     0x1F
#define RESOLUTION_10BIT    0x3F
#define RESOLUTION_11BIT    0x5F
#define RESOLUTION_12BIT    0x7F
//--------------------------------------------------
void ds18b20_PortInit(void);
void ds18b20_Init(uint8_t mode, uint8_t*);
void ds18b20_ConvertTemp(uint8_t mode, uint8_t DevNum, uint8_t* address);
void ds18b20_ReadStratchpad(uint8_t mode, uint8_t *Data, uint8_t DevNum, uint8_t* address);
void ds18b20_MatchRom(uint8_t* address);
void DelayMicro(__IO uint32_t micros);
//--------------------------------------------------
#endif /* DS18B20_H_ */