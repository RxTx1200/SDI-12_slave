#include"Includes.h"
#define SET_BIT(x,n)  x |= 1<<n
#define CLR_BIT(x,n)  x  &= ~(1<<n)
#define DQPIN_MAKEINPUT     TRISB0 =1
#define DQPIN_MAKEOUTPUT    TRISB0 =0
#define DQPIN_CLR           LATBbits.LB0 =0
#define DQPIN_SET           LATBbits.LB0 =1
#define DQPIN_GET           PORTB & 0x01
#define INIT_STATE          0
#define ROM_STATE           1
#define FUNCTION_STATE      2
#define NO_ERROR            1
#define NO_PRESENCE_PULSE  10
#define TEMPERATURE_ERROR  11
#define NO_DS18B20         12
#define DEFAULT_ERROR      13
#define CRC_ERROR          14
#define STATE_ERROR        15
#define FUNC_ERROR         16
#define NO_DEVICE          17
//Rom commands
#define SEARCH_ROM         0xF0 //allows the master to determine multiple slave devices
#define READ_ROM           0x33 //useful when there is only one slave on the bus
#define MATCH_ROM          0x55 //useful for addressing a particular slave
#define SKIP_ROM           0xCC //Can address all devices on the bus without sending out any ROM code information
#define ALARM_SEARCH       0xEC //only the devices with alarm flag set will respond
//Function commands
#define CONVERTT           0x44 //Initiates the temperature conversion
#define READ_SCRATCHPAD    0xBE //Reads the scratchpad register
#define WRITE_SCRATCHPAD   0x4E //write into the scratchpad register
#define COPY_SCRATCHPAD    0x48 //copies scratchpad data into the eeprom
#define RECALL_E2          0xB8 //Copies EEPROM data into the scratchpad
#define READ_POWER_SUPPLY  0xB4 //Read the power supply status
//Default ScratchPad Byte Values
#define TEMPERATURE_LSB    0x50
#define TEMPERATURE_MSB    0x05
#define BYTE_5             0xff
#define BYTE_7             0x10
#define DELAY_1us()        Delay1TCY()
#define DELAY_13us()       Delay1TCYx(6);

void Timer3_init(void);
void Timer3_usdelay(unsigned int);
void DS18B20_Exit();
uint8_t DS18B20readTemperature(int16_t*);

unsigned char ROMAddr[8];
unsigned char ScratchPad[9];
uint8_t state;

uint8_t DS18B20_read(uint16_t* Data) {
    Data[0] = 0xff;
    TRISB0 = 0;
    LATB0 = 1;
    Timer3_init();
    Timer3_usdelay(65535);
    while (1){
        retValue = DS18B20readTemperature(Data);
        if (retValue == 0){
            break;
        }
        else if (retValue == CRC_ERROR ){
            Data[0] = 0xffee;
            break;
        }
    }
    return 1;
}

void Timer3_usdelay(unsigned int x) {
    TMR3H = (65535 - x) >> 8;
    TMR3L = (65535 - x);
    T3CONbits.TMR3ON = 1;
    while (!(PIR2bits.TMR3IF));
    T3CONbits.TMR3ON = 0;
    PIR2bits.TMR3IF = 0;
}

uint8_t verifyCRC(unsigned char *buffer, unsigned int length) {
    uint8_t i, j;
    uint8_t crc = 0x00;
    for (i = 0; i < length - 1; i++) {
        crc = crc ^ buffer[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x01) {
                crc = (crc >> 1);
                crc ^= 0x8C;
            } else {
                crc >>= 1;
            }
        }
    }
    if (crc == buffer[length - 1])
        return 0;
    return CRC_ERROR;
}

void write(uint8_t cmd) {
    /*
    All the data and commands should be transmitted least significant bit first over the 1-wire system
     */
    uint8_t bitMask;
    DQPIN_MAKEOUTPUT;
    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        DQPIN_CLR;
        DELAY_1us();
        DELAY_13us();
        if (bitMask & cmd) {
            DQPIN_SET;
            Timer3_usdelay(40); //20us while switching
        } else {
            Timer3_usdelay(45); //60us
            DQPIN_SET;
        }
    }
}
/*
void readAddress(unsigned char *ROM_ADDR) {
    uint8_t i, j, value;
    memset(ROM_ADDR, 0, sizeof (ROM_ADDR));
    for (j = 0; j < 8; j++) {
        value = 0;
        for (i = 0; i < 8; i++) {
            DQPIN_MAKEOUTPUT;
            DQPIN_CLR;
            DELAY_1us();
            DQPIN_MAKEINPUT;
            DELAY_13us();
            if (DQPIN_GET)
                SET_BIT(value, i);
            else
                CLR_BIT(value, i);
            Timer3_usdelay(45);
        }
        ROM_ADDR[j] = value;
    }
}
*/
/*
 * Return is useful only verify is Set
 * Return 0 on default value match else
 * Return corresponding error
 */
uint8_t readScratchPadData(bool verify, unsigned char *Scratch_Pad) {
    uint8_t i, j, value;
    // memset(Scratch_Pad, 0, sizeof (Scratch_Pad));
    for (j = 0; j < 9; j++) {
        value = 0;
        for (i = 0; i < 8; i++) {
            DQPIN_MAKEOUTPUT;
            DQPIN_CLR;
            DELAY_1us();
            DQPIN_MAKEINPUT;
            DELAY_13us();
            if (DQPIN_GET)
                SET_BIT(value, i);
            else
                CLR_BIT(value, i);
            Timer3_usdelay(45);
        }
        Scratch_Pad[j] = value;
    }
    if (verifyCRC(Scratch_Pad, 9))
        return CRC_ERROR;
     if ((Scratch_Pad[0] == TEMPERATURE_LSB) && (Scratch_Pad[1] == TEMPERATURE_MSB) && (Scratch_Pad[5] == BYTE_5) && (Scratch_Pad[7] == BYTE_7))
         return DEFAULT_ERROR;
    return 0;
}

/*
 * Send Initialization  Signal
 * Initialization Signal : In the initialization sequence,the bus master transmits the reset pulse by pulling the line low for a minimum of 480us
 * Then it releases the bus and goes back to the receive mode, when the bus is released the pull-up resistor will pulls the 1-wire bus to high.
 * When the DS18B20 detects the rising edge,it waits for 15us to 20us and then transmits a presence pulse  by pulling the 1-wire bus low for 60-240us
 *
 * Returns 0 on presence of the device
 * Returns 1 on no device exist
 */
uint8_t sendInitSignal() {
    uint8_t presenceStatus = 0x00;
    state = INIT_STATE;
    DQPIN_MAKEOUTPUT;
    DQPIN_CLR;
    Timer3_usdelay(480);
    DQPIN_SET;
    DQPIN_MAKEINPUT;
    Timer3_usdelay(70);
    presenceStatus = DQPIN_GET;
    Timer3_usdelay(170);
    return presenceStatus;
}

/* Returns zero on Success
 * Else return Error Type
 */
uint8_t sendROMCommand(uint8_t cmd, unsigned char *data) {
    if (state != INIT_STATE)
        return STATE_ERROR;
    state = ROM_STATE;
    write(cmd);
    //  if (cmd == READ_ROM)
    //      readAddress(data);
    return 0;
}

/* Returns zero on success 0
 * Else return Error Type
 */
uint8_t sendFunctionCommand(uint8_t cmd) {
    uint8_t i;
    if (state != ROM_STATE)
        return STATE_ERROR;
    state = FUNCTION_STATE;
    write(cmd);
    if (cmd == CONVERTT) {
        for (i = 0; i < 24; i++) { //750ms
            if (DQPIN_GET)
                return 0;
            Timer3_usdelay(65535);
        }
        return (1);
    }
    return 0;
}

uint8_t DS18B20readTemperature(int16_t *temperature) {
    uint8_t retvalue, i;
    uint8_t cmd_array[2] = {CONVERTT, READ_SCRATCHPAD};
    for (i = 0; i < 2; i++) {
        if (sendInitSignal())
            return NO_PRESENCE_PULSE;
        if (sendROMCommand(SKIP_ROM, ROMAddr))
            return STATE_ERROR;
        retvalue = sendFunctionCommand(cmd_array[i]);
        if (retvalue)
            return retvalue;
    }
    retvalue = readScratchPadData(0, ScratchPad);
    if (retvalue)
        return retvalue;
    *temperature = ScratchPad[0] | ScratchPad[1] << 8;
    *temperature = *(temperature) >> 4; //neglecting the decimal part
    return 0;
}

void Timer3_init() {
    PMD0bits.TMR3MD = 0; //enabling the Timer3
    T3CON = (T3CON & 0x3F) | (0 << 6); //clk-> FOSC/4
    //T3CON = (T3CON & 0xCF) | (1 << 4); // 1:2 prescalar 1us resolution
    //TMR3 = 0x00; //setting the timer1 register to zero
    TMR3H = 0;
    TMR3L = 0;
}

void DS18B20_Exit() {
    T3CONbits.TMR3ON = 0;
    TMR3 = 0;
    PMD0bits.TMR3MD = 1;
}