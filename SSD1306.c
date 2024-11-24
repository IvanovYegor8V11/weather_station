#include "SSD1306.h"
#include <math.h>

uint8_t OLED_BuffOut[8][128] = {0x00};	

uint8_t Sym_0[6] = {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00};				// 0
uint8_t Sym_1[4] = {0x00, 0x42, 0x7F, 0x40};							// 1
uint8_t Sym_2[6] = {0x42, 0x61, 0x51, 0x49, 0x46, 0x00};				// 2
uint8_t Sym_3[6] = {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00};				// 3
uint8_t Sym_4[6] = {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00};				// 4
uint8_t Sym_5[6] = {0x27, 0x45, 0x45, 0x45, 0x39, 0x00};				// 5
uint8_t Sym_6[6] = {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00};				// 6
uint8_t Sym_7[6] = {0x01, 0x79, 0x09, 0x05, 0x03, 0x00};				// 7
uint8_t Sym_8[6] = {0x36, 0x49, 0x49, 0x49, 0x36, 0x00};				// 8
uint8_t Sym_9[6] = {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00};				// 9
uint8_t Sym_A[6] = {0x7E, 0x09, 0x09, 0x09, 0x7E, 0x00};				// A
uint8_t Sym_B[6] = {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00};				// B
uint8_t Sym_C[6] = {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00};				// C
uint8_t Sym_D[6] = {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00};				// D
uint8_t Sym_E[6] = {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00};				// E
uint8_t Sym_F[6] = {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00};				// F
uint8_t Sym_Point[2] = {0x80,0x00};                                     // .


SymbolType HexSym[17] = {
{	6,	8,	Sym_0},	{	4,	8,	Sym_1},	{	6,	8,	Sym_2},	{	6,	8,	Sym_3},
{	6,	8,	Sym_4},	{	6,	8,	Sym_5},	{	6,	8,	Sym_6},	{	6,	8,	Sym_7},
{	6,	8,	Sym_8},	{	6,	8,	Sym_9},	{	6,	8,	Sym_A},	{	6,	8,	Sym_B},
{	6,	8,	Sym_C},	{	6,	8,	Sym_D},	{	6,	8,	Sym_E},	{	6,	8,	Sym_F},
{   2,  8,  Sym_Point}
};


void OLED_Reset() {
	
	GPIOA->BSRR = GPIO_BSRR_BR9;
	GPIOA->BSRR = GPIO_BSRR_BS8;
	DelayMicro(1000);
	GPIOA->BSRR = ~GPIO_BSRR_BS8;
	GPIOA->BSRR = GPIO_BSRR_BR8;
	DelayMicro(10000);
	GPIOA->BSRR = ~GPIO_BSRR_BR8;
	GPIOA->BSRR = GPIO_BSRR_BS8;
	DelayMicro(10000);
}
// OLED Initialization, Send init command via I2C
void OLED_Init() {
	OLED_Reset();	
	OLED_SendByte(0xAE, 1);			// display OFF 
		
	OLED_SendByte(0x20, 1);			// Adressing mode
	OLED_SendByte(0x00, 1);			// Horizontal Addressing Mode

	OLED_SendByte(0xA1, 1);			// Columns Remap (127..0)
	
	OLED_SendByte(0xC8, 1);		    // Strings Remap (63..0)
	
	OLED_SendByte(0x8D, 1);			// Charge Pump Setting
	OLED_SendByte(0x14, 1);			// Enable Charge Pump
	OLED_SendByte(0xAF, 1);			// Display ON
	
	OLED_SendByte(0x21, 1);			// Set Column Address
	OLED_SendByte(0, 1);			// 0 is start
	OLED_SendByte(127, 1);			// 127 is end
	
	OLED_SendByte(0x22, 1);			//	Set String Address	
	OLED_SendByte(0, 1);			//  0 is start
	OLED_SendByte(7, 1);			//  7 is end
	
	OLED_Clear();
}

// Clear OLED, Send zeros to OLED via I2C
void OLED_Clear(void) {
    uint16_t i;
    d_I2C_Start();											
    while(!d_StartEnd);
        
    d_I2C_Byte(SSD1306_ADDRESS);	        // OLED slave address (0x78)		
    while(!d_AdrSendEnd);
    d_I2C_SR2_Clear();										

    d_I2C_Byte(0x40);						// OLED data mode						

    for(i = 0; i < 1024; i++) {
        d_I2C_Byte(0x00);					// Send zero data to OLED, 8 pages x 128 columns, OLED_BuffOut[8][128]	
        while(!d_ByteSendEnd);	
    }

    d_I2C_Stop();
    while(I2C1->SR2 & I2C_SR2_BUSY);
}

//Clear square with Height and Width in OLED X,Y coordinates. 
//Function works with I2C interrupts (fill OLED_BuffOut and send it after I2C start condition)
void OLED_ClearXY(uint8_t X, uint8_t Y, uint8_t Height, uint8_t Width) {
	uint8_t i, j;
	uint8_t OffsetY = Y % 8;						// calculate Y offset for pages 
	
	if(OffsetY > 0) {
        Height = ((Height >> 3) + 1);		        // recalculate Height to get bytes (pages) count
    }		
	else {
        Height = Height >> 3;
    }		

	Y = Y >> 3;										// get page for Y
	
	for (i = 0; i < Height; i++) {
		for (j = 0; j < Width; j++) {			
			OLED_BuffOut[Y + i][X + j] = 0x00;			// clear square in OLED_BuffOut[8][128]	
		}			
	}	
}
//Send byte data to OLED via I2C
void OLED_SendByte(uint8_t data, uint8_t mod) {
    d_I2C_Start();
    while (!d_StartEnd);
        
    d_I2C_Byte(SSD1306_ADDRESS);
    while (!d_AdrSendEnd);
    d_I2C_SR2_Clear();

    if(mod)
        d_I2C_Byte(0x00);				// OLED command mode
    else
        d_I2C_Byte(0x40);				// OLED data mode
    while (!d_ByteSendEnd);

    d_I2C_Byte(data);					// Send data to OLED
    while (!d_ByteSendEnd);

    d_I2C_Stop();
    while (d_I2C_WaitBusy);
}

// Send message data to OLED via I2C 
void OLED_SendData (uint8_t* Data, uint16_t LenMsg, uint8_t Mod) {
	uint16_t i;
	
	d_I2C_Start();						
	while(!d_StartEnd);
		
	d_I2C_Byte(SSD1306_ADDRESS);			
	while(!d_AdrSendEnd);
	d_I2C_SR2_Clear();					
	
	if(Mod)
		d_I2C_Byte(0x00);					// OLED command mode		
	else
		d_I2C_Byte(0x40);					// OLED data mode
	
	while(!d_ByteSendEnd);

	for(i = 0; i < LenMsg; i++)
	{
		d_I2C_Byte(Data[i]);			// Send message to OLED
		while(!d_ByteSendEnd);	
	}

	d_I2C_Stop();							
	while(d_I2C_WaitBusy);		
}

// Send symbol from structure SymbolType
// Function works with I2C interrupts (fill OLED_BuffOut and send it after I2C start condition)
void OLED_FillBuffer (uint8_t X, uint8_t Y, SymbolType *Data)
{
	uint8_t i, j;
	uint8_t temp;
	uint8_t Height;
	uint8_t Width;
	uint8_t OffsetY = Y % 8;															// calculate Y offset for pages 
	
	if(OffsetY > 0)
		Height = ((Data->Height >> 3) + 1);									// recalculate Height to get bytes (pages) count
	else
		Height = Data->Height >> 3;		

	Width = Data->Width;
	Y = Y >> 3;																						// get page for Y
		
	for(i = 0; i < Height; i++)
	{
		for(j = 0; j < Width; j++)
		{
			temp = 0;
	
			if(i > 0)
				temp = ((Data->Symbol[(i - 1)*Width+j]) >> (8 - OffsetY));		// create mask for filling second page
			
			if(i < (Data->Height >> 3))
				temp |= ((Data->Symbol[i*Width+j]) << OffsetY);								// create mask for filling first page
			
			OLED_BuffOut[Y+i][X+j] = temp;																	// fill buffer by created mask 
		}			
	}
}

void OLED_OutHex(uint8_t X, uint8_t Y, uint32_t Data, uint8_t Len)
{
	uint8_t temp, i;	
	
	temp = 128 - (6 * (Len + 2));
	if((X < temp) && (Y < 56))
	{
		OLED_FillBuffer (X, Y, &HexSym[0]);			
		OLED_FillBuffer (X + 6, Y, &HexSym[16]);	
	}
	else
		return;
	
	if(Len > 8)
		Len = 8;
	
	X+=12;
	
	for(i = 0; i < Len; i++)
	{
		temp = ((Data >> ((Len - 1) * 4)) & 0x0F);
		OLED_ClearXY((X + (i * 6)), Y, 8, 8);
		OLED_FillBuffer((X + (i * 6)), Y, &HexSym[temp]);
		Data = Data << 4;
	}
}


void OLED_outSign (uint8_t X, uint8_t Y, int16_t Data) {
		OLED_ClearXY(X, Y, 8, 6);
		if (Data & 0x8000) {
				OLED_FillBuffer(X, Y, &HexSym[17]);
		} else {
				OLED_FillBuffer(X, Y, &HexSym[18]);
		}
}

uint8_t OLED_OutDec(uint8_t X, uint8_t Y, int16_t Data, uint8_t sign) {
		int16_t number = Data; 
		uint8_t symbol = 0, count = 0, pos = 0, digits_count = 0;
		uint8_t symbols [10] = {0};
		if (Data == 0) {
				OLED_ClearXY(X, Y, 8, 6);
				OLED_FillBuffer(X, Y, &HexSym[0]);
				digits_count = 1;
		} else {
				while (abs(number) > 0) {
					symbol = abs(number) % 10;
					number = number / 10;
					symbols[count] = symbol;
					count++;
				}
				digits_count = count;
				if (sign) {
						OLED_outSign(X,Y,Data);
						digits_count++;
						pos++;
				}
				while (count--) 
				{
						OLED_ClearXY((X + (pos * 6)), Y, 8, 6);
						OLED_FillBuffer((X + (pos * 6)), Y, &HexSym[symbols[count]]);
						pos++;
				}
		}
		return digits_count;
}

void OLED_SendBuffer () {
			I2C_WriteData(DATA_CODE,&OLED_BuffOut[0][0],1024,SSD1306_ADDRESS);
}

