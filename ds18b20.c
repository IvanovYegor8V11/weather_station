#include "ds18b20.h"

//----------------------------------------------------------
void ds18b20_PortInit(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;	    // I/O Port B clock enabled, RCC->APB2ENR |= 1000b, IOPBEN = 1;
    GPIOB->CRH |= GPIO_CRH_MODE11;			// Output Mode, max speed 50MHz, GPIOB->CRH |=   11000000000000b, MODE11 = 11
    GPIOB->CRH |= GPIO_CRH_CNF11_0;			// General purpose output Open-drain, GPIOB->CRH |=  100000000000000b, CNF11 = 01
    GPIOB->CRH &= ~GPIO_CRH_CNF11_1;		// General purpose output Open-drain, GPIOB->CRH &= 0111111111111111b, CNF11 = 01
}
//----------------------------------------------------------
uint8_t ds18b20_Reset(void) {                       // Initialization
    uint16_t status;
    
    GPIOB->BSRR = GPIO_BSRR_BR11;					// set 0 to PIN11 
    DelayMicro(480);                                // delay 480 mcs (RESET PULSE)
    
    GPIOB->BSRR = GPIO_BSRR_BS11;					// set 1 to PIN11 
    DelayMicro(40);									// delay 60 mcs (RISING EDGE)
    
    status = GPIOB->IDR & GPIO_IDR_IDR11;	        // check value on PIN11, if not zero then device not responds
    DelayMicro(120);								// delay 480 mcs (PRESENCE)
    
    return (status ? 1 : 0);
}
//----------------------------------------------------------
uint8_t ds18b20_ReadBit(void) {
    uint8_t bit = 0;
    
    GPIOB->BSRR = GPIO_BSRR_BR11;					// set 0 to PIN11
    DelayMicro(1);									// delay 1 mcs
    
    GPIOB->BSRR = GPIO_BSRR_BS11;					// set 1 to PIN11 
    DelayMicro(14);									// delay 14 mcs
    
    bit = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);    // check value on PIN11
    DelayMicro(45);									// delay 45 mcs
    
    return bit;
}
//----------------------------------------------------------
uint8_t ds18b20_ReadByte(void) {
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++)
        data |= ds18b20_ReadBit() << i;			    // read bit by bit from sensor
    
    return data;
}
//----------------------------------------------------------
void ds18b20_WriteBit(uint8_t bit) {
    GPIOB->BSRR = GPIO_BSRR_BR11;					// set 0 to PIN11
    DelayMicro(bit ? 1 : 60);						// if bit is 1 delay 1mcs else 60mcs
    
    GPIOB->BSRR = GPIO_BSRR_BS11;					// set 1 to PIN11 
    DelayMicro(bit ? 60 : 1);						// if bit is 1 delay 60mcs else 1mcs
}
//----------------------------------------------------------
void ds18b20_WriteByte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        ds18b20_WriteBit(data >> i & 1); 			// write bit by bit to sendor
        DelayMicro(5);								// delay for protection
    }
}
//----------------------------------------------------------
void ds18b20_MatchRom(uint8_t* address) {
    uint8_t i;
    ds18b20_Reset();
    ds18b20_WriteByte(MATCH_CODE);			        // send match rom command to sensor
    for (i = 0; i < 8; i++) {
        ds18b20_WriteByte(address[i]); 		        // send address to match with sensor address
    }
}
//----------------------------------------------------------
void ds18b20_Init_R9(uint8_t mode, uint8_t* address) {
    ds18b20_Reset();
    if (mode == 0) {				                //if skip rom mode selected
        ds18b20_WriteByte(SKIP_ROM); 				//send skip ROM command
    } 
    else {
        ds18b20_MatchRom(address); 					//send match code command with address
    }
    
    ds18b20_WriteByte(WRITE_SCRATCHPAD);            //send write scratchpad command
    ds18b20_WriteByte(0x64); 						//send Th = 100
    ds18b20_WriteByte(0xFF9E); 						//send Tl = -30
    ds18b20_WriteByte(RESOLUTION_12BIT);            //set resolution 12 bit
}
//----------------------------------------------------------
void ds18b20_Init_R12(uint8_t mode, uint8_t* address) {
    ds18b20_Reset();
    if (mode == 0) {				                //if skip rom mode selected
        ds18b20_WriteByte(SKIP_ROM); 				//send skip ROM command
    } 
    else {
        ds18b20_MatchRom(address); 					//send match code command with address
    }
    
    ds18b20_WriteByte(WRITE_SCRATCHPAD);            //send write scratchpad command
    ds18b20_WriteByte(0x64); 						//send Th = 100
    ds18b20_WriteByte(0xFF9E); 						//send Tl = -30
    ds18b20_WriteByte(RESOLUTION_12BIT);            //set resolution 12 bit
}
//----------------------------------------------------------
void ds18b20_ConvertTemp(uint8_t mode, uint8_t DevNum, uint8_t* address) {
    ds18b20_Reset();
    if (mode == 0) {			                    //if skip rom mode selected
        ds18b20_WriteByte(SKIP_ROM); 			    //send skip ROM command
    } 
    else {
        ds18b20_MatchRom(address); 				    //send match code command with address
    }
    
    ds18b20_WriteByte(CONVERT_TEMP); 		        //send convert temp command
}
//----------------------------------------------------------
void ds18b20_ReadStratchpad(uint8_t mode, uint8_t *Data, uint8_t DevNum, uint8_t* address) {
    ds18b20_Reset();
    if (mode == 0) {
        ds18b20_WriteByte(SKIP_ROM); 			    //if skip rom mode selected
    } 
    else {
        ds18b20_MatchRom(address); 				    //send match code command with address
    }
    
    ds18b20_WriteByte(READ_SCRATCHPAD);             //send read scratchpad command
    for (uint8_t i = 0; i < 9; i++) {
        Data[i] = ds18b20_ReadByte();			    // read scratchpad byte by byte
    }
}
//----------------------------------------------------------
void ds18b20_ReadROM(uint8_t *Data) {
    ds18b20_Reset();
    ds18b20_WriteByte(READ_ROM);			        //send read rom command 		
    for (uint8_t i = 0; i < 8; i++) {
        Data[i] = ds18b20_ReadByte(); 	            // read rom byte by byte
    }
}
//----------------------------------------------------------
uint8_t Compute_CRC8(uint8_t* data, uint8_t length) {
    uint8_t polynomial = 0x8C, crc = 0x0, j = 0, lsb = 0, inbyte = 0;
    while (length--) {
        inbyte = data[j];
        for (uint8_t i = 0; i < 8; i++) {
            lsb = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (lsb) {
                crc ^= polynomial;
            }                             
            inbyte >>= 1;
        }
        j++;
    }

    return crc; 
}
//----------------------------------------------------------