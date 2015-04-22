#define __18F23K22

#include <xc.h>         /*generic file for xc8 compiler*/
#include<plib/usart.h> /*for usart function*/
#include<plib/EEP.h>   /*for eeprom function*/
#include"Includes.h"

void peripheral_disable();
void interrupt_config();
void update_eeprom();
void Break_Timer_init(void);
void sdi_usart_init(void);
void sdi_usart_TXEN(bool);
void sdi_usart_RXEN(bool);
void sdi_usart_RXIE(bool);
bool sdi_usart_putchar(unsigned char);
void sensor_usart_init(void);
void sensor_usart_TXEN(bool);
void sensor_usart_RXEN(bool);
bool sensor_usart_putchar(unsigned char);
unsigned char sensor_usart_getchar();

void intialize(void) {
    // disable all the unused peripherals
    peripheral_disable();
    // all ports -> Digital i/o
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;
    //disable all interrupt on change  bits of port b
    IOCB = 0x00;
    SLRCON &= ~(0x07);
    LATA = 0x00;
    PORTB = 0xff;
    BREAK_PIN_DIR = 1; //as input
    TXBUFFER_PIN_DIR = 0; // as output
    TXBUFFER_ENABLE = 0;
    update_eeprom();
    Break_Timer_init();
    MarkingTimeout_Timer_init();
    CompareCapture_init();
    sdi_usart_init();
    sensor_usart_init();
    interrupt_config();

}

/* Function to configure Interrupts and priority
 */
void interrupt_config() {
    BREAK_IOC_ENABLE = 1; //Enabling the PORTB interrupt-on-change
    BREAK_IOCPIN_ENABLE = 1; //Enabling the PORTB pin for interrupt on change
    BREAK_IP = 1; //setting the PORTB interrupt-on-change high priority
    BREAK_IF = 0; //clearing the Break pinchange interrupt flag
    BREAK_TIMER_OVF_IF = 0; //Clearing Timer0 overflow interrupt flag
    BREAK_TIMER_OVF_IE = 1; //Enabling the Timer0 overflow interrupt for break
    BREAK_TIMER_OVF_IP = 1; //setting the Timer0 OVF interrupt high priority

    MARKING_TIMEOUT_COMPARE_IP = 0; //Setting the priority for CCP1 interrupt high

    MARKING_TIMEOUT_COMPARE_IE = 0; //Disabling the compare capture1 interrupt for break and marking

    SDIUSART_RX_IP = 0; //Setting the sdi_uart_rx interrupt low priority
    sdi_usart_RXIE(0); //Disabling the sdi uart receiver interrupt
    /* Enabling the interrupt priority*/
    INTERRUPT_PRI_EN(1);
    /* Enabling the global interrupt enable bit for high and low priority*/
    INTERRUPT_ENABLE(1);
}

void update_eeprom() {
    uint8_t i = 0,temp;
    /* Reading the SDI12 Address*/
    Busy_eep();
    if (Read_b_eep(EEPROM_SENNO) != SENNO) {
        Busy_eep();
        Write_b_eep(EEPROM_SENNO, (SENNO));
        while (i < SENNO) {
            Busy_eep();
            Write_b_eep(EEPROM_ADDBYTE + i, AddressSet[i]);
            i++;
        }
    } else {
        while (i < SENNO) {
            Busy_eep();
            temp = Read_b_eep(EEPROM_ADDBYTE + i);
            if (temp > 96 && temp < 123) {
                AddressSet[i] = temp;
            }
            i++;
        }
    }
}

/* Function to disable unused peripheral modules*/
void peripheral_disable() {
    PMD0bits.SPI1MD = 1; //disabling the SPI1 module
    PMD0bits.SPI2MD = 1; //disabling the SPI2 module
    PMD0bits.TMR3MD = 1; //disabling the Timer3
    PMD0bits.TMR4MD = 1; //disabling the Timer4
    PMD0bits.TMR5MD = 1; //disabling the Timer5
    PMD0bits.TMR6MD = 1; //disabling the Timer6

    PMD1bits.CCP2MD = 1; //disabling the CCP2 module
    PMD1bits.CCP3MD = 1; //disabling the CCP3 module
    PMD1bits.CCP4MD = 1; //disabling the CCP4 module
    PMD1bits.CCP5MD = 1; //disabling the CCP5 module
    PMD1bits.MSSP1MD = 1; //disabling the MSSP1 module
    PMD1bits.MSSP2MD = 1; //disabling the MSSPI module

    PMD2bits.ADCMD = 1; //disabling the ADC module
    PMD2bits.CMP1MD = 1; //disabling the Comparator1 module
    PMD2bits.CMP2MD = 1; //disabling the Comparator2 module

}

/* Break Timer initialization function*/
void Break_Timer_init() {
    BREAK_TIMER_ENABLE = 0; //Disabling the break timer
    BREAK_TIMER_MOD = 0; //Setting the timer0 in 16 bit mode
    BREAK_TIMER_CS = 0; //Setting the timer0 with internal instruction cycle clk
    BREAK_TIMER_REG = BREAK_TIMER_REGVAL; //Setting timer0 registers to initial value

}

/* Marking and Timeout detection timer initialization function*/
void MarkingTimeout_Timer_init() {
    MARKING_TIMEOUT_TIMER_CLKS(0); //selecting the FOSC/4 as clk for timer1
    MARKING_TIMEOUT_TIMER_PRE(3); //Selecting 1:8 prescalar for timer1
    //MARKING_TIMEOUT_TIMER_CON.T1RD16 =1   //Enables register read/write of Timer1 in one 16-bit operation
    MARKING_TIMEOUT_TIMER_REG = 0x00; //setting the timer1 register to zero
    //     TMR1H = 0;
    //     TMR1L = 0;
}

/* Function to initialize the compare and capture */
void CompareCapture_init() {
    /* configuring the compare1 module for marking and timeout detection*/
    MARKING_TIMEOUT_COMPARE_MOD(0x0A); //selecting the software interrupt mode
    MARKING_TIMEOUT_COMPARE_TMRS(0); //selecting the timer as timer1
    MARKING_TIMEOUT_COMPARE_REG = MARKING_VALUE_D; //setting value for detecting marking first
}

/* Functions to intialize sdi_usart - usart2 */
void sdi_usart_init(void) {
    /*Setting Rx and Tx TRIS contol pins*/
    SDIUSART_RXPIN_DIR = 1;
    SDIUSART_TXPIN_DIR = 1;
    /*Setting the RCSTA2 and TXSTA2 registers
     * |SPEN|RX9|SREN|CREN|ADDEN|FERR|OERR|RX9D|
     *  1     0   x    0     0    x    x     x
     * serial port is enabled and 8 bit mode is selected to support the parity bit
     *
     * |CSRC|TX9|TXEN|SYNC|SENDB|BRGH|TRMT|TX9D|
     *   x     0    0   0   0       1   x   x
     * asynchronous mode with high speed baud rate is selected and trasmit is disabled
     *
     */

    SDIUSART_RX_CON.SPEN = 1;
    SDIUSART_TX_CON.BRGH = 1;
    /*Setting SPBRG|SPBRGH registers*/
    SDIUSART_SPBRG(SDIUSART_BAUD_SEL_VAL);

    /*Setting the Baud rate control register BAUDCON2
     *
     * |ABDOVF|RCIDL|DTRXP|CKTXP|BRG16|-|WUE|ABDEN|
     *  x       x       0   0     1        0    0
     *
     * Rx and Tx idle state is selected as high and 16 bit baud config is chosen
     */
    SDIUSART_BAUDCON.BRG16 = 1;
    SDIUSART_BAUDCON.DTRXP = 0;
    SDIUSART_BAUDCON.CKTXP = 0;

}

void sdi_usart_TXEN(bool state) {
    /*Setting the USART2 transmit enable bit*/
    SDIUSART_TX_CON.TXEN = state;
}

void sdi_usart_RXEN(bool state) {
    /*Setting the USART2 receiver enable bit*/
    SDIUSART_RX_CON.CREN = state;
}

void sdi_usart_RXIE(bool state) {
    /*Setting the USART2 Interrupt enable bit*/
    SDIUSART_RX_IE = state;
}

bool sdi_usart_putchar(unsigned char c) {
    while (SDIUSART_TX_FLAG == 0);
    SDIUSART_TXREG = c;
    while (SDIUSART_TX_FLAG == 0);
    return 0;

}

void sdi_TX_Marking(void) {
    MARKING_TIMEOUT_COMPARE_MOD(0x00); //setting the compare mode off
    MARKING_TIMEOUT_TIMER_EN = 0; //Disabling the timer1
    MARKING_TIMEOUT_TIMER_REG = MARKING_VALUE_G; //Setting the timer1 register for marking transmission
    MARKING_TIMER_OVERFLOW_IF = 0;
    MARKING_TIMEOUT_TIMER_EN = 1; //Enabling the timer1
    while (!(MARKING_TIMER_OVERFLOW_IF)) //waiting for the timer1 overflow
    {
        asm("NOP");
    }
    MARKING_TIMER_OVERFLOW_IF = 0; // Clearing the timer1 overflow interrupt
    MARKING_TIMEOUT_TIMER_EN = 0; //Disabling the timer1
    MARKING_TIMEOUT_COMPARE_MOD(0x0A); //selecting the software interrupt mode

}

void sensor_usart_init(void) {
    /*Setting Rx and Tx TRIS contol pins*/
    SENSOR_USART_RXPIN_DIR = 1;
    SENSOR_USART_TXPIN_DIR = 1;
    /*Setting the RCSTA1 and TXSTA1 registers
     * |SPEN|RX9|SREN|CREN|ADDEN|FERR|OERR|RX9D|
     *  1     0   x    0     0    x    x     x
     * serial port is enabled and eight bit mode is selected
     *
     * |CSRC|TX9|TXEN|SYNC|SENDB|BRGH|TRMT|TX9D|
     *   x     0    0   0   0       1   x   x
     * asynchronous mode with high speed baud rate is selected and trasmit is disabled
     *
     */
    SENSOR_USART_RX_CON.SPEN = 1;
    SENSOR_USART_TX_CON.BRGH = 1;
    /*Setting SPBRG1|SPBRGH1 registers */
    SENSOR_USART_SPBRG(SENSOR_USART_BAUD_SEL_VAL);
    /*Setting the Baud rate control register BAUDCON2
     *
     * |ABDOVF|RCIDL|DTRXP|CKTXP|BRG16|-|WUE|ABDEN|
     *  x       x       0   0     1        0    0
     *
     * Rx and Tx idle state is selected as high and 16 bit baud config is chosen
     */
    SENSOR_USART_BAUDCON.BRG16 = 1;
    SENSOR_USART_BAUDCON.DTRXP = 0;
    SENSOR_USART_BAUDCON.CKTXP = 0;

}

void sensor_usart_TXEN(bool state) {
    /*Setting the USART1 transmit enable bit*/
    SENSOR_USART_TX_CON.TXEN = state;
}

void sensor_usart_RXEN(bool state) {
    /*Setting the USART1 receiver enable bit*/
    SENSOR_USART_RX_CON.CREN = state;
}

bool sensor_usart_putchar(unsigned char c) {
    while (SENSOR_USART_TX_FLAG == 0) {
    }
    SENSOR_USART_TXREG = c;
    while (SENSOR_USART_TX_FLAG == 0) {
    }
    return 0;
}

unsigned char sensor_usart_getchar() {
    while (SENSOR_USART_RX_FLAG == 0) {
    }
    return SENSOR_USART_RXREG;
}
