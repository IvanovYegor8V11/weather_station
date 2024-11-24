#include "stm32f10x.h"
#include "I2C.h"
#include "delay.h"

#define SSD1306_ADDRESS  0x78
#define DATA_CODE				 0x40

typedef struct
{
	uint8_t Width;
	uint8_t Height;	
	uint8_t	*Symbol;
} SymbolType;


void OLED_Init	();
void OLED_Clear (void);
void OLED_ClearXY(uint8_t X, uint8_t Y, uint8_t Height, uint8_t Width);
void OLED_SendByte (uint8_t data, uint8_t mod);
void OLED_SendData (uint8_t* Data, uint16_t LenMsg, uint8_t Mod);
void OLED_SendBuffer();
void OLED_FillBuffer (uint8_t X, uint8_t Y, SymbolType *Data);
void OLED_OutHex(uint8_t X, uint8_t Y, uint32_t Data, uint8_t Len);
uint8_t OLED_OutDec(uint8_t X, uint8_t Y, int16_t Data, uint8_t sign);
void OLED_OutBin(uint8_t X, uint8_t Y, uint16_t Data);
void OLED_OutFloat_TMP36(uint8_t X, uint8_t Y, float Data) ;

extern SymbolType HexSym[17];
