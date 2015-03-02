#include <xc.h>         /* generic headerfile for xc8 compiler */
#include <stdio.h>
#include <stdint.h>     /* for uint8_t data type */
#include <stdbool.h>    /* for boolean data type */

void DHT11_Timer3_init(void);
void DHT11_Capture4_init(void);
void DHT11_Timer3_usdelay(unsigned int);
void DHT11_Exit(void);

uint8_t DHT11_read (uint16_t* Data)
{
    uint8_t k,j,result;
    uint8_t DHT11Data[5];

    Data[0]=0xff;
    Data[0]=0xff;
       
    TRISB0 = 0;
    LATB0 =1;
    DHT11_Timer3_init();
    for(k=0;k<70;k++)
      DHT11_Timer3_usdelay(65535);
    LATB0 =0;                 //setting the dataline low
    DHT11_Timer3_usdelay(18000);
    LATB0  =1;                 //setting the dataline to high
    TRISB0 =1;                 //dataline->input
    DHT11_Timer3_usdelay(20);

    while(( PORTB&0b00000001));
    DHT11_Timer3_usdelay(40);
    while(!( PORTB&0b00000001));
    DHT11_Timer3_usdelay(40);
    while(( PORTB&0b00000001));
    DHT11_Capture4_init();
    TMR3 = 0x00;
    /* loops to read the 40 bit data from DHT11 sensor */
    for(k=0;k<5;k++){
        result=0;
        for(j=0;j<8;j++)
        {
            while(!(PORTB&0b00000001));       //waiting for dataline to go high
            T3CONbits.TMR3ON = 1;
            while ( (!(PIR4bits.CCP4IF))); //&& (!(PIR2bits.TMR3IF) ));
            if ( (CCPR4)>50)                    //>35=1
                result |= (1<<(7-j));
            PIR4bits.CCP4IF =0;
            PIR2bits.TMR3IF =0;
            T3CONbits.TMR3ON = 0;
            TMR3 = 0x00;
            }
        DHT11Data[k]=  result;
        }

    if((DHT11Data[0]+DHT11Data[1]+DHT11Data[2]+DHT11Data[3])!=DHT11Data[4])
    {
        DHT11_Exit();
        return (0);
    }

    Data[0]=DHT11Data[0];
    Data[1]=DHT11Data[2];
    DHT11_Exit();
    return (2);
}
void DHT11_Timer3_init()
{
    PMD0bits.TMR3MD = 0;    //enabling the Timer3

    T3CON = (T3CON & 0x3F)|( 0 <<6);      //clk-> FOSC/4
    T3CON = (T3CON & 0xCF)|( 1 <<4);      // 1:2 prescalar 1us resolution

    //TMR3 = 0x00;      //setting the timer1 register to zero
           TMR3H = 0;
           TMR3L = 0;

}

void DHT11_Capture4_init()
{
    PMD1bits.CCP4MD = 0;    //enabling CCP4 module


    CCP4CON = (CCP4CON & 0xF0)| 0x04;       // Every Falling Edge
    CCPTMRS1 = (CCPTMRS1 & 0xFC)| 0x01;     // Timer3 as source

}
void DHT11_Timer3_usdelay( unsigned int x)
{
    TMR3H = (65535-x)>>8;
    TMR3L = (65535-x);
    T3CONbits.TMR3ON = 1;
    while(!(PIR2bits.TMR3IF));
    T3CONbits.TMR3ON =0;
    PIR2bits.TMR3IF  =0;
}
void DHT11_Exit()
{
    CCP4CON = (CCP4CON & 0xF0);              //turning off CCP4
    PIR4bits.CCP4IF =0;

    T3CONbits.TMR3ON =0;
    TMR3 =0;

    PMD1bits.CCP4MD = 1;
    PMD0bits.TMR3MD = 1;

}