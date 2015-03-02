#include "Includes.h"

void GYPSUM_Timer3_init(void);
void GYPSUM_Capture2_init(void);
void GYPSUM_Exit(void);

uint8_t GYPSUM_read(uint16_t* Data)
{
    uint16_t captureValue;
    Data[0] = 0xff;
    GYPSUM_Timer3_init();
    GYPSUM_Capture2_init();
    T3CONbits.TMR3ON = 1;
    while (PIR2bits.CCP2IF != 1);
    PIR2bits.CCP2IF = 0;
    captureValue = CCPR2;
    while (PIR2bits.CCP2IF != 1);
    T3CONbits.TMR3ON = 0;
    PIR2bits.CCP2IF = 0;
    captureValue = CCPR2 - captureValue;
    Data[0] = captureValue;
    GYPSUM_Exit();
    return (1);

}

void GYPSUM_Timer3_init()
{
    PMD0bits.TMR3MD = 0; //enabling the Timer3
    T3CON = (T3CON & 0x3F) | (0 << 6); //clk-> FOSC/4
    T3CON = (T3CON & 0xCF) | (1 << 4); // 1:2 prescalar 1us resolution
    TMR3 = 0x00; //setting the timer1 register to zero
    //     TMR1H = 0;
    //     TMR1L = 0;
}

void GYPSUM_Capture2_init()
{
    PMD1bits.CCP2MD = 0; //enabling CCP2 module
    TRISB3 = 1;
    LATB3 = 0;
    PORTBbits.RB3 = 0;
    WPUBbits.WPUB3 = 1;
    INTCON2bits.nRBPU = 0;
    CCP2CON = (CCP2CON & 0xF0) | 0x04; // Every Falling Edge
    CCPTMRS0 = (CCPTMRS0 & 0xE7) | 0x08; // Timer3 as source

}

void GYPSUM_Exit()
{
    CCP2CON = (CCP2CON & 0xF0); //turning off CCP4
    PIR2bits.CCP2IF = 0;
    T3CONbits.TMR3ON = 0;
    TMR3 = 0;
    PMD1bits.CCP2MD = 1;
    PMD0bits.TMR3MD = 1;
    WPUBbits.WPUB3 = 0;
    INTCON2bits.nRBPU = 1;
}
