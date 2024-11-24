#include "I2C.h"

#define I2C_OWNADDRESS1_7BIT             0x00004000U
#define I2C_MODE_I2C                     0x00000000U
#define I2C_REQUEST_WRITE                       0x00
#define I2C_REQUEST_READ                        0x01

void I2C_Init(void) {
    uint32_t tmpreg;
    //RCC_AFIO_CLK_ENABLE(); // Alternative function clock enabling
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
    /* Delay after an APB2 Alternative function clock enabling */
    tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);	//  IO Port B clock enabling
    // PIN8, PIN9 configure to alternative function, open-drain output
    SET_BIT(GPIOB->CRH, GPIO_CRH_CNF9_1 | GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_0 | GPIO_CRH_CNF8_0 |\
                                    GPIO_CRH_MODE9_1 | GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_0 ); 
    
    //APB1 peripheral clock enabling
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);
    //Delay after an APB1 peripheral clock enabling
    tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);
    //Disable I2C peripheral
    CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);
    //ClockSpeed
    MODIFY_REG(I2C1->CR2, I2C_CR2_FREQ, 36);
    MODIFY_REG(I2C1->TRISE, I2C_TRISE_TRISE, 36 + 1); 
    MODIFY_REG(I2C1->CCR, (I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR), 180); // Thigh = Tlow = 180 * (1/36) = 5 mcs => f = 100 kHz
    //Configure I2C1 peripheral mode with parameter : I2C
    MODIFY_REG(I2C1->CR1, I2C_CR1_SMBUS | I2C_CR1_SMBTYPE | I2C_CR1_ENARP, I2C_MODE_I2C);
    //Enable I2C peripheral
    SET_BIT(I2C1->CR1, I2C_CR1_PE);
    //TypeAcknowledge
    MODIFY_REG(I2C1->CR1, I2C_CR1_ACK, I2C_CR1_ACK);
    //Set the 7bits Own Address2
    MODIFY_REG(I2C1->OAR2, I2C_OAR2_ADD2, 0);
    
    AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP; // Remap default I2C PB 7, PB6 to PB8, PB9 
}

void I2C_WriteData (uint8_t addr, uint8_t *buf, uint16_t bytes_count, uint8_t slave_address) {
    uint16_t i;
    //Disable Pos
    CLEAR_BIT(I2C1->CR1, I2C_CR1_POS);
    //Prepare the generation of a ACKnowledge condition
    //after the address receive match code or next received byte
    MODIFY_REG(I2C1->CR1, I2C_CR1_ACK, I2C_CR1_ACK);
    //Generate a START condition
    SET_BIT(I2C1->CR1, I2C_CR1_START);
    //Indicate the status of Start Bit (master mode)
    while (!READ_BIT(I2C1->SR1, I2C_SR1_SB)){}
    //read state
    (void) I2C1->SR1;
    //Transmit Address to SLAVE
    MODIFY_REG(I2C1->DR, I2C_DR_DR, slave_address | I2C_REQUEST_WRITE);
    while (!READ_BIT(I2C1->SR1, I2C_SR1_ADDR)){}
    (void) I2C1->SR1;
    (void) I2C1->SR2;
    //Transmit Reg Address begin 
    MODIFY_REG(I2C1->DR, I2C_DR_DR, addr);
    while (!READ_BIT(I2C1->SR1, I2C_SR1_TXE)){}

    for(i = 0;i < bytes_count;i++) {
        MODIFY_REG(I2C1->DR, I2C_DR_DR, buf[i]);
        while (!READ_BIT(I2C1->SR1, I2C_SR1_TXE)){}
    }
    SET_BIT(I2C1->CR1, I2C_CR1_STOP);
}

void I2C_ReadData (uint8_t addr, uint8_t *buf, uint16_t bytes_count, uint8_t slave_address) {
    uint16_t i;
    //Disable Pos
    CLEAR_BIT(I2C1->CR1, I2C_CR1_POS);
    //Prepare the generation of a ACKnowledge condition
    //after the address receive match code or next received byte
    MODIFY_REG(I2C1->CR1, I2C_CR1_ACK, I2C_CR1_ACK);
    //Generate a START condition
    SET_BIT(I2C1->CR1, I2C_CR1_START);
    //Indicate the status of Start Bit (master mode)
    while (!READ_BIT(I2C1->SR1, I2C_SR1_SB)){}
    //read state
    (void) I2C1->SR1;
    //Transmit Address to SLAVE 
    MODIFY_REG(I2C1->DR, I2C_DR_DR, slave_address | I2C_REQUEST_WRITE);
    while (!READ_BIT(I2C1->SR1, I2C_SR1_ADDR)){}
    (void) I2C1->SR1;
    (void) I2C1->SR2;
    //Transmit Reg Address begin
    MODIFY_REG(I2C1->DR, I2C_DR_DR, addr);
    while (!READ_BIT(I2C1->SR1, I2C_SR1_TXE)){}
    SET_BIT(I2C1->CR1, I2C_CR1_START);
    while (!READ_BIT(I2C1->SR1, I2C_SR1_SB)){}
    (void) I2C1->SR1;
    //Transmit Address to SLAVE    
    MODIFY_REG(I2C1->DR, I2C_DR_DR, slave_address | I2C_REQUEST_READ);
    while (!READ_BIT(I2C1->SR1, I2C_SR1_ADDR)){}
    (void) I2C1->SR1;
    (void) I2C1->SR2;
    for(i = 0; i < bytes_count; i++) {
        if(i < (bytes_count-1)) {
            while (!READ_BIT(I2C1->SR1, I2C_SR1_RXNE)){}
            buf[i] = READ_BIT(I2C1->DR, I2C_DR_DR); // buf[i] = I2C1->DR & I2C_DR_DR = I2C1->DR & 0xFF;
        }
        else {
            CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);
            SET_BIT(I2C1->CR1, I2C_CR1_STOP);
            while (!READ_BIT(I2C1->SR1, I2C_SR1_RXNE)){}
            buf[i] = READ_BIT(I2C1->DR, I2C_DR_DR); // buf[i] = I2C1->DR & I2C_DR_DR = I2C1->DR & 0xFF;
        }
    }
}
