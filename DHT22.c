#include "DHT22.h"

void DHT22_Start() {
    GPIOA->BSRR |= GPIO_BSRR_BR12;
    DelayMicro(18000);
    
    GPIOA->BSRR |= GPIO_BSRR_BS12;
    DelayMicro(30);
}

void DHT22_Stop() {
    GPIOA->BSRR |= GPIO_BSRR_BR12;
}

uint8_t DHT22_ReadBit() {
    DelayMicro(80);
    
    if (GPIOA->IDR & GPIO_IDR_IDR12) {    
        while(GPIOA->IDR & GPIO_IDR_IDR12) {__NOP();}
        /*DelayMicro(45);*/
        return 1;    
    }
    else {
        return 0;
    }    
}

uint8_t DHT22_ReadByte() {
    uint8_t data = 0;
    
    int8_t i;
    for (i = 7; i >= 0; i--) {
        data |= DHT22_ReadBit() << i;
    }
    
    return data;
}
