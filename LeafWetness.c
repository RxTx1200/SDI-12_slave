/* File:   main.c
 * Author: Praveen James
 *
 * Created on July 14, 2014, 10:41 AM
 */
#include "Includes.h"

void delay(int);
void adc_config(void);
void adc_exit(void);

uint8_t LeafWetness_read (uint16_t* Data)
{
    uint16_t count=0,captureValue=0;
    adc_config();
    ADCON0bits.ADON =1;
    captureValue = 0;
    for(count=0;count<16;count++)
    {
        ADCON0bits.GO = 1;
        while (ADCON0bits.GO == 1);
        captureValue = captureValue+(ADRESH<<8|ADRESL);
        delay(100);
    }
    Data[0] = captureValue>>4;
    ADCON0bits.ADON =0;
    adc_exit();
    return 1;

}

void adc_config()
{
    ANSELAbits.ANSA2 = 1;
    TRISAbits.RA2    = 1;
    PMD2bits.ADCMD   = 0;
    VREFCON0         = 0x90;           // 1.024 internal reference voltage
    while(VREFCON0bits.FVRS == 0 );
    ADCON0           = 0x02<<2;        //channel = 2
    // for testing  internal fixed voltage reference
    // ADCON0       = 0x1F<<2;
    ADCON1          = 0x02<<2;    //positive reference = internal fixed voltage reference
    ADCON2          = 0xB5;         //Right Adjusted: Aquistion time = 20TAD : ADC clock = FOSC/16
}

void adc_exit(void)
{
    //ANSELAbits.ANSA2 = 0;
    //TRISAbits.RA2    = 0;
    PMD2bits.ADCMD   = 1;
    VREFCON0bits.FVREN =0;
}
