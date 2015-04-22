#include "Includes.h"
#include <plib/delays.h>

enum {
    TEMP, HUMI
};

#define CRC 1
#define _SHT15_DAT (TRISCbits.TRISC3)
#define DATA (PORTCbits.RC3)
#define _SHT15_SCK (LATCbits.LATC4)
#define SET_SHT15_SCK (TRISCbits.TRISC4)

#define noACK 0
#define ACK 1

#define STATUS_REG_W     0x06 //000 0110 0
#define STATUS_REG_R     0x07 //000 0111 1
#define MEASURE_TEMP     0x03 //000 0011 1
#define MEASURE_HUMI     0x05 //000 0101 1
#define RESET            0x1E //000 1111 0

void delay(int x);
void s_transstart(void);
void s_connectionreset(void);
char s_write_byte(unsigned char value);
char s_read_byte(unsigned char ack);
char s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum);
char s_write_statusreg(unsigned char *p_value);
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode);
char s_softreset(void);
void init_Sht15(void);
void SHT15_Exit(void);
void calcCRC(uint8_t, uint8_t*);
uint8_t bitrev(uint8_t);

#if CRC
uint8_t crc;
#endif

uint8_t SHT15_read(uint16_t* Data) {
    unsigned char error, checksum;
    unsigned int temp_reading;
    unsigned int humi_reading;

    Data[0] = 0xff;
    Data[1] = 0xff;

    init_Sht15();
    s_connectionreset();
    error = 0;
    error += s_measure((unsigned char*) &temp_reading, &checksum, TEMP); //measure temperature
    error += s_measure((unsigned char*) &humi_reading, &checksum, HUMI); //measure humidity
    if (error != 0) {
        s_connectionreset(); //in case of an error: connection reset
    } else {
        Data[0] = temp_reading;
        Data[1] = humi_reading;
    }
    SHT15_Exit();
    return (2);

}

void delay(int x) {
    int i;
    for (i = 0; i < x; i++) {
        Delay100TCYx(20); //wait 1ms
    }
}
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______

void s_transstart(void) {
    _SHT15_DAT = 1;
    _SHT15_SCK = 0; //Initial state
    Nop();
    _SHT15_SCK = 1;
    Nop();
    _SHT15_DAT = 0;
    Nop();
    _SHT15_SCK = 0;
    Nop();
    Nop();
    Nop();
    _SHT15_SCK = 1;
    Nop();
    _SHT15_DAT = 1;
    Nop();
    _SHT15_SCK = 0;
}
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______

void s_connectionreset(void) {
    unsigned char i;
    _SHT15_DAT = 1;
    _SHT15_SCK = 0; //Initial state
    for (i = 0; i < 15; i++) {
        _SHT15_SCK = 1;
        _SHT15_SCK = 0;
    }
    s_transstart(); //transmission start
}

char s_write_byte(unsigned char value) {
    unsigned char i;
    unsigned char error = 0;

    for (i = 0x80; i > 0; i /= 2) {
        if (i & value) {
            _SHT15_DAT = 1;
        } else {
            _SHT15_DAT = 0;
        }
        _SHT15_SCK = 1; //clk for SENSI-BUS
        Nop();
        Nop();
        Nop(); // >.5uS //pulswith approx. 1.5 us
        _SHT15_SCK = 0;
    }
    _SHT15_DAT = 1; //release DATA-line
    _SHT15_SCK = 1; //clk #9 for ack
    error = DATA; //check ack (DATA will be pulled down by SHT11)
    Nop();
    _SHT15_SCK = 0;
    return (error); //error=1 in case of no acknowledge
}

char s_read_byte(unsigned char ack) {
    unsigned char i;
    unsigned char val = 0;
    _SHT15_DAT = 1; //release DATA-line

    for (i = 0x80; i > 0; i /= 2) {
        _SHT15_SCK = 1; //clk for SENSI-BUS
        delay(1);
        if (DATA) {
            val = (val | i);
        }
        _SHT15_SCK = 0;
    }
    _SHT15_DAT = !ack; //in case of "ack==1" pull down DATA-Line
    _SHT15_SCK = 1; //clk #9 for ack
    Nop();
    Nop();
    Nop(); // >.5uS //pulswith approx. 1.5 us
    _SHT15_SCK = 0;
    _SHT15_DAT = 1; //release DATA-line
    return val;
}

char s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum) {
    unsigned char error = 0;

    s_transstart(); //transmission start
    error = s_write_byte(STATUS_REG_R); //send command to sensor
    *p_value = s_read_byte(ACK); //read status register (8-bit)
    *p_checksum = s_read_byte(noACK); //read checksum (8-bit)
    return error; //error=1 in case of no response form the sensor
}

char s_write_statusreg(unsigned char *p_value) {
    unsigned char error = 0;
    s_transstart(); //transmission start
    error += s_write_byte(STATUS_REG_W); //send command to sensor
    error += s_write_byte(*p_value); //send value of status register
    return error; //error>=1 in case of no response form the sensor
}

char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode) {
    unsigned char error = 0;
    unsigned long i;

#if CRC
    crc = 0x00;
#endif

    s_transstart(); //transmission start
    switch (mode) {
            //send command to sensor
        case TEMP:
            error += s_write_byte(MEASURE_TEMP);
#if CRC
            calcCRC(MEASURE_TEMP, &crc);
#endif
            break;
        case HUMI:
            error += s_write_byte(MEASURE_HUMI);
#if CRC
            calcCRC(MEASURE_HUMI, &crc);
#endif
            break;
        default:
            break;
    }
    for (i = 0; i < 0xAC5FF;i++) {
        if (DATA == 0) {
            break; //wait until sensor has finished the measurement
        }
    }
    if (DATA) {
        error += 1; // or timeout (~2 sec.) is reached
        return error;
    }
    *(p_value + 1) = s_read_byte(ACK); //read the first byte (MSB)
    *(p_value) = s_read_byte(ACK); //read the second byte (LSB)
    *p_checksum = s_read_byte(noACK); //read checksum

#if CRC
    calcCRC(*(p_value + 1), &crc);
    calcCRC(*p_value, &crc);
    if (bitrev(crc) != *p_checksum){
        *(p_value + 1) = 0xff;
        *(p_value) = 0xee;
        error+=1;
    }
#endif
    return error;
}

char s_softreset(void) {
    unsigned char error = 0;
    s_connectionreset(); //reset communication
    error += s_write_byte(RESET); //send RESET-command to sensor
    Delay10KTCYx(200); //200mS
    return error; //error=1 in case of no response form the sensor
}

void init_Sht15(void) {
    DATA = 0;
    _SHT15_DAT = 1;
    SET_SHT15_SCK = 0;
    _SHT15_SCK = 0; // clock = low

    s_softreset();
    s_connectionreset();

}

void SHT15_Exit(void) {
    DATA = 0;
    _SHT15_DAT = 0;
    SET_SHT15_SCK = 0;
    _SHT15_SCK = 0;
}

#if CRC

uint8_t bitrev(uint8_t value) {
    uint8_t temp = 0, i;
    for (i = 8; i > 0; i--) {
        temp = (temp << 1) | (value & 0x01);
        value >>= 1;
    }
    return temp;
}

void calcCRC(uint8_t value, uint8_t* crc) {
    uint8_t i;
    *crc ^= value;
    for (i = 8; i > 0; i--) {
        if (*crc & 0x80) {
            *crc = (*crc << 1);
            *crc ^= 0x31;
        } else {
            *crc = (*crc << 1);
        }
    }
}
#endif