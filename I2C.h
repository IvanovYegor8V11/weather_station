#include "stm32f10x.h"

void I2C_Init(void);
void I2C_WriteData (uint8_t addr, uint8_t *buf, uint16_t bytes_count, uint8_t slave_address);
void I2C_ReadData (uint8_t addr, uint8_t *buf, uint16_t bytes_count, uint8_t slave_address);

#define d_StartEnd			(I2C1->SR1 & I2C_SR1_SB)		
#define d_AdrSendEnd		(I2C1->SR1 & I2C_SR1_ADDR)	
#define d_ByteSendEnd		(I2C1->SR1 & I2C_SR1_TXE)		
#define d_I2C_WaitBusy		(I2C1->SR2 & I2C_SR2_BUSY)	
#define d_I2C_Start()		I2C1->CR1 |= (I2C_CR1_START | I2C_CR1_PE)		
#define d_I2C_Stop()		I2C1->CR1 |= I2C_CR1_STOP		
#define d_I2C_Byte(Byte)	I2C1->DR = Byte							
#define d_I2C_SR2_Clear()	(void) I2C1->SR2			
		