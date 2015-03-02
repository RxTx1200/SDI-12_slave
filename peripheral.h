/* 
 * File:   intialize.h
 * Author: Praveen James
 *
 * Created on September 11, 2013, 10:07 AM
 */

#ifndef INTIALIZE_H
#define	INTIALIZE_H


/* Macros for controlling the peripheral*/

/* Macros for controlling the Interrupt enable bits*/
#define INTERRUPT_PRI_EN(X)  RCONbits.IPEN = X;
#define INTERRUPT_ENABLE(X)  { INTCONbits.GIE_GIEH = X;  \
                              INTCONbits.PEIE_GIEL = X; \
                            }
#define INTERRUPT_LOWPRI_EN(X) INTCONbits.PEIE_GIEL =X;

/* Macros for  SDI-12 TX pin buffer control pin configuration */

#define TXBUFFER_PIN_DIR    TRISA0
#define TXBUFFER_ENABLE     LATA0




/*SDI-12 USART */

/* 16 bit is used and for 8 mhz the value of the registers for 1200 baud rate is 1666 */

#define SDIUSART_BAUD_SEL_VAL   1666                     //refer data sheet
#define SDIUSART_RX_CON         RCSTA2bits
#define SDIUSART_BAUDCON        BAUDCON2bits
#define SDIUSART_SPBRG(X)       { SPBRGH2 = (X)>>8; \
                                  SPBRG2 = X ;  \
                                }
#define SDIUSART_RXREG          RCREG2
#define SDIUSART_TX_CON         TXSTA2bits
#define SDIUSART_TXREG          TXREG2
#define SDIUSART_TX_FLAG        TXSTA2bits.TRMT2
#define SDIUSART_RX_IE          PIE3bits.RC2IE
#define SDIUSART_RX_IP          IPR3bits.RC2IP
#define SDIUSART_RX_IF          PIR3bits.RC2IF
#define SDIUSART_RXPIN_DIR      TRISB7
#define SDIUSART_TXPIN_DIR      TRISB6

/* SENSOR usart*/

/* 16 bit is used and for 8 mhz the value of the registers for 9600 baud rate is 207 */

#define SENSOR_USART_BAUD_SEL_VAL 207
#define SENSOR_USART_RX_CON     RCSTA1bits
#define SENSOR_USART_BAUDCON    BAUDCON1bits
#define SENSOR_USART_SPBRG(X)  { SPBRGH1 = (X)>>8; \
                                 SPBRG1 = X ;  \
                                }
#define SENSOR_USART_RXREG      RCREG1
#define SENSOR_USART_TX_CON     TXSTA1bits
#define SENSOR_USART_TXREG      TXREG1
#define SENSOR_USART_TX_FLAG    TXSTA1bits.TRMT1
#define SENSOR_USART_RX_FLAG    PIR1bits.RC1IF
#define SENSOR_USART_RXPIN_DIR  TRISC7
#define SENSOR_USART_TXPIN_DIR  TRISC6

/* PORTB for break*/

#define BREAK_PORT              PORTB
#define BREAK_PIN               4
#define BREAK_IOC_ENABLE        INTCONbits.RBIE
#define BREAK_IOCPIN_ENABLE     IOCBbits.IOCB4
#define BREAK_IP                INTCON2bits.RBIP
#define BREAK_PIN_DIR           TRISB4
#define BREAK_PIN_VAL           PORTBbits.RB4
#define BREAK_IF                INTCONbits.RBIF


/* TIMER0 for break detection*/
#define BREAK_TIMER_OVF_IE      INTCONbits.TMR0IE
#define BREAK_TIMER_OVF_IF      INTCONbits.TMR0IF
#define BREAK_TIMER_OVF_IP      INTCON2bits.TMR0IP
#define BREAK_TIMER_ENABLE      T0CONbits.TMR0ON
#define BREAK_TIMER_MOD         T0CONbits.T08BIT
#define BREAK_TIMER_CS          T0CONbits.T0CS
#define BREAK_TIMER_PRESCALAR(X)   { T0CONbits.PSA =0; \
                                     T0CON =(T0CON & 0xF8)|X ; \
                                   }
#define BREAK_TIMER_REG         TMR0


/* Marking and Timeout Detection using Timer1 and compare1 module*/

#define MARKING_TIMEOUT_COMPARE_MOD(X)  CCP1CON  = (CCP1CON  & 0xF0)|X
#define MARKING_TIMEOUT_COMPARE_TMRS(X) CCPTMRS0 = (CCPTMRS0 & 0xFC)|X
#define MARKING_TIMEOUT_COMPARE_REG     CCPR1
#define MARKING_TIMEOUT_COMPARE_IP      IPR1bits.CCP1IP
#define MARKING_TIMEOUT_COMPARE_IE      PIE1bits.CCP1IE
#define MARKING_TIMEOUT_COMPARE_IF      PIR1bits.CCP1IF
#define MARKING_TIMEOUT_TIMER_CON       T1CONbits
#define MARKING_TIMEOUT_TIMER_EN        T1CONbits.TMR1ON
#define MARKING_TIMEOUT_TIMER_CLKS(X)   T1CON = (T1CON & 0x3F)|(X<<6)
#define MARKING_TIMEOUT_TIMER_PRE(X)    T1CON = (T1CON & 0xCF)|(X<<4)
#define MARKING_TIMEOUT_TIMER_REG       TMR1
#define MARKING_TIMER_OVERFLOW_IF       PIR1bits.TMR1IF






/* System Intializing function  definition */
    void intialize (void);

    void Break_Timer_init();
    void MarkingTimeout_Timer_init();
    void CompareCapture_init();

    void sdi_usart_TXEN(bool);
    void sdi_usart_RXEN(bool);
    void sdi_usart_RXIE(bool);

    bool sdi_usart_putchar(unsigned char );
    void sdi_TX_Marking(void);

    void sensor_usart_TXEN(bool);
    void sensor_usart_RXEN(bool);

    bool sensor_usart_putchar(unsigned char);
    unsigned char sensor_usart_getchar();

    extern volatile char Address;



#endif	/* INTIALIZE_H */

