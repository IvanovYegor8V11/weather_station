#include "stm32f10x.h"
#include "GPIO_Init.h"
#include "SSD1306.h"
#include "ds18b20.h"
#include "DHT22.h"
#include <string.h>

#define BMP280_ADDRESS          0xEC

int16_t raw_temp;
float temp;

float hum = 0, temp_dht22 = 0;
uint8_t dht22_data[5] = {0, 0, 0, 0, 0};

float bmp280_temp;
float pressure;
int32_t adc_P;
int32_t adc_T;

void BMP280_Reset() {	
	GPIOA->BSRR = GPIO_BSRR_BS8;
	GPIOA->BSRR = GPIO_BSRR_BS9;
	DelayMicro(10000);
}

int32_t bmp280_compensate_T_int32(int32_t adc_T, uint16_t dig_T1, int16_t dig_T2, int16_t dig_T3, int32_t *t_fine) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;
    return T * 2;
}

uint32_t bmp280_compensate_P_int64(int32_t adc_P, uint16_t dig_P1, int16_t dig_P2, int16_t dig_P3, int16_t dig_P4,
                                   int16_t dig_P5, int16_t dig_P6, int16_t dig_P7, int16_t dig_P8, int16_t dig_P9, int32_t t_fine) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0;
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;
}


uint8_t count_digits(int32_t value) {
    if (value == 0) return 1;

    uint8_t count = 0;
    while (value > 0) {
        value /= 10;
        count++;
    }

    return count;
}

void float_to_string(char* str, float value, uint8_t precision) {
    int32_t int_part = (int32_t)value;
    float fl_part = value - int_part;
    uint8_t int_count = count_digits(int_part);    
    
    uint8_t index = int_count;
    while (int_part > 0) {
        str[index - 1] = '0' + (int_part % 10);
        int_part = int_part / 10;
        index--;
    }
    
    str[int_count] = '.';
    
    int_count += 1;
    
    uint8_t i;
    for (i = 0; i < precision; i++) {
        uint8_t fl_int_part = fl_part * 10;
        str[int_count + i] = '0' + fl_int_part;
        fl_part = fl_part * 10 - fl_int_part;
    }
}
SymbolType* char_to_symbol(char c) {
    switch (c) {
        case '0':
            return &HexSym[0];
        case '1':
            return &HexSym[1];
        case '2':
            return &HexSym[2];
        case '3':
            return &HexSym[3];
        case '4':
            return &HexSym[4];
        case '5':
            return &HexSym[5];
        case '6':
            return &HexSym[6];
        case '7':
            return &HexSym[7];
        case '8':
            return &HexSym[8];
        case '9':
            return &HexSym[9];
        case '.':
            return &HexSym[16];
    }
}

void PrintSymbols (float temp, float pressure, float humidity) {
    char temp_str[5] = {0, 0, 0, 0, 0};
    float_to_string(temp_str, temp, 2);
    
    for (int i = 0; i < 5; i++) {
        OLED_FillBuffer (10 + 7 * i, 20, char_to_symbol(temp_str[i] ));		
    }
    for (int i = 0; i < 7; i++) {
        OLED_FillBuffer (10 + 7 * i, 32, &HexSym[10 + i]);		
    }
                        
    OLED_SendBuffer ();
}

int main (void) {
    SysTick_Config(SystemCoreClock / 1000000); 
    GPIO_Init();

    I2C_Init();	
    OLED_Init();	
        
    //PrintSymbols();
    
//------------------------------------------------------------------------    
    uint8_t scratchpad_data[9] = {0};
    uint8_t rom_data [8] = {0};
    uint8_t ds18b20_addr [8] = {0x28,0xA0,0x3F,0xC1,0x0B,0x00,0x00,0xDE};
    //uint8_t ds18b20_addr [8] = {0x28,0xEB,0xBB,0x08,0x0C,0x00,0x00,0x57};
    uint8_t crc8_rom = 0x0,  crc8_rom_error = 1;
    uint8_t crc8_data = 0x0, crc8_data_error = 0;

    

    ds18b20_PortInit();
    while (ds18b20_Reset());

    while (crc8_rom_error) {
        ds18b20_ReadROM(rom_data);
        crc8_rom = Compute_CRC8(rom_data, 7);
        crc8_rom_error = Compute_CRC8(rom_data, 8) == 0 ? 0 : 1;
    }

    ds18b20_Init_R12(1, rom_data);
//------------------------------------------------------------------------    
    
    
    uint8_t power_on_reset[1] = {0xB6};
    I2C_WriteData(0xE0, power_on_reset, 1, BMP280_ADDRESS);
    DelayMicro(10000);
    
    uint8_t ctrl_meas[1] = {0xFF};
    I2C_WriteData(0xF4, ctrl_meas, 1, BMP280_ADDRESS);
    DelayMicro(10000);
    
    uint8_t calib[24] = {0}; 
    I2C_ReadData(0x88, calib, 24, BMP280_ADDRESS);
    DelayMicro(10000);
    
    uint16_t dig_T1 = ((uint16_t)calib[0] << 8) + (uint16_t)calib[1];
    int16_t dig_T2 = ((int16_t)calib[2] << 8) + (int16_t)calib[3];
    int16_t dig_T3 = ((int16_t)calib[4] << 8) + (int16_t)calib[5];
    
    uint16_t dig_P1 = ((uint16_t)calib[6] << 8) + (uint16_t)calib[7];
    int16_t dig_P2 = ((int16_t)calib[8] << 8) + (int16_t)calib[9];
    int16_t dig_P3 = ((int16_t)calib[10] << 8) + (int16_t)calib[11];
    int16_t dig_P4 = ((int16_t)calib[12] << 8) + (int16_t)calib[13];
    int16_t dig_P5 = ((int16_t)calib[14] << 8) + (int16_t)calib[15];
    int16_t dig_P6 = ((int16_t)calib[16] << 8) + (int16_t)calib[17];
    int16_t dig_P7 = ((int16_t)calib[18] << 8) + (int16_t)calib[19];
    int16_t dig_P8 = ((int16_t)calib[20] << 8) + (int16_t)calib[21];
    int16_t dig_P9 = ((int16_t)calib[22] << 8) + (int16_t)calib[23];

    while (1) {
        //ds18b20
        if (!crc8_data_error) {
            ds18b20_ConvertTemp(1, 0, rom_data);
        }
        DelayMicro(750000);
        ds18b20_ReadStratchpad(1, scratchpad_data, 0, rom_data);
        crc8_data = Compute_CRC8(scratchpad_data, 8);
        crc8_data_error = Compute_CRC8(scratchpad_data, 9) == 0 ? 0 : 1;
        if (!crc8_data_error) {
            raw_temp = ((uint16_t)scratchpad_data[1] << 8) | scratchpad_data[0];
            temp = raw_temp * 0.0625;
        }
        
        //dht22
        DHT22_Start();
        DelayMicro(165);
        
        uint8_t i;
        for (i = 0; i < 5; i++) {
            dht22_data[i] = DHT22_ReadByte();
        }
        
        uint8_t check_sum = dht22_data[0] + dht22_data[1] + dht22_data[2] + dht22_data[3];        
        if (check_sum == dht22_data[4]) {
            float hum_buffer = 0, temp_buffer = 0;
            hum_buffer = ((int16_t)(dht22_data[0]) << 8) / 10.0f + dht22_data[1] / 10.0f;
            temp_buffer = ((int16_t)(dht22_data[2]) << 8) / 10.0f + dht22_data[3] / 10.0f;
            
            hum = hum_buffer;
            temp_dht22 = temp_buffer;            
        } 
        
        //bmp280
        uint8_t buf[6] = {0}; 
        I2C_ReadData(0xF7, buf, 6, BMP280_ADDRESS);
        
        DelayMicro(100000);
        
        adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)(buf[2] >> 4));
        adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)(buf[5] >> 4));

        int32_t t_fine;
        bmp280_temp = bmp280_compensate_T_int32(adc_T, dig_T1, dig_T2, dig_T3, &t_fine) / 100.0;
        pressure = bmp280_compensate_P_int64(adc_P, dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, t_fine) / 100.0 - 33000;
        
        PrintSymbols(temp, pressure, hum);
    }
}