/*  Project :  SDI-12 Header for sensors using PIC18F23k22
 *
 */
#include "Includes.h"

/* Macro for parity bit generation:
 * returns 0 for even parity and 1 for odd parity
 */
#define PARITY(b)   (((((((b)^(((b)<<4)|((b)>>4))) + 0x41) | 0x7C ) +2 ) & 0x80)>>7)

/******************************************************************************/
volatile char Address; //holds the sdi-12 address
volatile char cmd; //current executing command
volatile uint8_t breakflag, abortflag, commandflag, addressflag;
volatile char commandBuffer[COMMAND_MAX], valueBuffer[SEN_DATA_VALUE_MAX], responseBuffer[SEN_RESPONSE_MAX];
volatile uint8_t bufferLength, valueLength;
volatile uint8_t i, j, index; //count variables
volatile char* send; //points to the transmitting buffer
volatile uint8_t retValue;

/******************************************************************************
 * High priority interrupt handler
 * Handles Break signal
 */
void interrupt high_isr(void)
{
    if (BREAK_IOC_ENABLE && BREAK_IF) {
        uint8_t break_port;
        BREAK_TIMER_ENABLE = 0; //resetting the break timer
        BREAK_TIMER_REG = BREAK_TIMER_REGVAL; //reloading the timer register with intial value for 12ms timeout
        MARKING_TIMEOUT_TIMER_EN = 0; //disabling the marking and timeout timer
        MARKING_TIMEOUT_TIMER_REG = 0;
        break_port = BREAK_PORT;
        /* Start the break detection if the pin has gone low
         */
        if ((BREAK_PIN_VAL) == 0) {
            BREAK_TIMER_ENABLE = 1; //starting the break timer for break detection
        }
        /* Start the marking or timeout detection if the pin has gone high
         */
        if ((BREAK_PIN_VAL == 1) && (breakflag == 1)) {
            MARKING_TIMEOUT_TIMER_EN = 1; //starting the marking and timeout timer for marking or timeout detection
        }
        BREAK_IF = 0; //resetting the interrupt flag.

    }
    if ((BREAK_TIMER_OVF_IE == 1) && (BREAK_TIMER_OVF_IF == 1)) {
        BREAK_TIMER_OVF_IF = 0; //interrupt flag =zero
        BreakFn();
    }
}

/*Low priority interrupt handler
 *Handles :  Break Timer overflow,
 *        :  Marking and TimeOut Timer overflow,
 *        :  SDI-12 uart reception
 */
void interrupt low_priority low_isr(void) {
    INTERRUPT_ENABLE(0);
    if ((MARKING_TIMEOUT_COMPARE_IE == 1) && (MARKING_TIMEOUT_COMPARE_IF == 1)) {
        MARKING_TIMEOUT_TIMER_EN = 0;
        MARKING_TIMEOUT_TIMER_REG = 0;
        MARKING_TIMEOUT_COMPARE_IF = 0; //clearing the CompareCapture interrupt flag
        /* marking
         */
        if (MARKING_TIMEOUT_COMPARE_REG == MARKING_VALUE_D) {
            INTERRUPT_ENABLE(1)
            Waiting();
        }
        /* timeout
         */
        StandbyFn();
    }
    if ((SDIUSART_RX_IE == 1) && (SDIUSART_RX_IF == 1)) {
        char temp;
        /* Disable the marking and timeout detection
         */
        MARKING_TIMEOUT_TIMER_EN = 0;
        MARKING_TIMEOUT_TIMER_REG = 0;
        temp = SDIUSART_RXREG & 0x7f; //To discard the parity bit; In pic18f23k22 parity bit is not handled in hardware
        if (addressflag == 0) {
            /* invalid address -> standby mode
             */
            if ((!(temp > 96 && temp < 123)) && temp != 0x3f) {
                // INTERRUPT_ENABLE(1)
                StandbyFn();
            }
            /* valid address
             */
            addressflag = 1;
            if (temp != 0x3f) {
                Address = temp;
            }
            MARKING_TIMEOUT_COMPARE_REG = MARKING_VALUE_D;
            MARKING_TIMEOUT_TIMER_EN = 1;
            index = 0;
            INTERRUPT_ENABLE(1)
            return;
        }
        commandBuffer[index] = temp;
        /* check for terminating character'!'
         * if true -> command
         */
        if (temp == 0x21) {
            addressflag = 0;
            sdi_usart_RXEN(0);
            INTERRUPT_ENABLE(1)
            command();
        }
        index++;
        MARKING_TIMEOUT_COMPARE_REG = MARKING_VALUE_D;
        MARKING_TIMEOUT_TIMER_EN = 1; //enabling the marking & timeout detection
        INTERRUPT_ENABLE(1)
    }

}

/*******************************Main function ********************************/
void main() {
    OSCCON = 0x60;
    int temp;
    intialize();
    temp = PORTB;
    StandbyFn();
    while (1) {
    }

}

/******************************************************************************
 * Function name: BreakFn
 * parameters   : void
 * description  : Used to process a successful brake detected over the sdi-12 bus.
 */
void BreakFn(void) {
    /* Make sure the interrupt is disabled
     */
    if (breakflag == 1) {
        if (commandflag == 1) {
            if (cmd == 'M') {
                AbortFn(); // cmd='M' -> Abort
            }
        }
    }
    breakflag = 1; 
    sdi_usart_RXIE(1); 
    sdi_usart_RXEN(1); 
    sdi_usart_TXEN(0); 
    /* -> Marking
     */
    MarkingFn();
}

/*******************************************************************************
 * Function name: MarkingFn
 * parameters   : void
 * description  : used to wait till a successful marking on the sdi-12 bus
 */
void MarkingFn(void) {
    STKPTR = 0x00;
    /* enable Marking detection
     */
    MARKING_TIMEOUT_TIMER_EN = 0;
    MARKING_TIMEOUT_TIMER_REG = 0x00;
    MARKING_TIMEOUT_COMPARE_REG = MARKING_VALUE_D;
    MARKING_TIMEOUT_COMPARE_IE = 1;
    //sdi_usart_RXIE(1);
    //sdi_usart_RXEN(1);
    //sdi_usart_TXEN(0);
    //MARKING_TIMEOUT_TIMER_EN = 0;
    INTERRUPT_ENABLE(1)   
    while (1) {
        asm("NOP");
    }
}

/*******************************************************************************
 * Function name: Waiting
 * parameters   : void
 * description  : used for waiting for address, timeout, or break
 */
void Waiting(void)
{
    STKPTR = 0x00;
    /* Enable the sdi_uart receiver module and RX interrupt
     */
    sdi_usart_RXIE(1);
    sdi_usart_RXEN(1);
    /* enage timeout detection
     */
    MARKING_TIMEOUT_TIMER_EN = 0;
    MARKING_TIMEOUT_TIMER_REG = 0x00;
    MARKING_TIMEOUT_COMPARE_REG = TIMEOUT_VALUE;
    MARKING_TIMEOUT_COMPARE_IE = 1;
    MARKING_TIMEOUT_TIMER_EN = 1;
    INTERRUPT_ENABLE(1)
    /* wait for address or timeout or break
     */
    while (1) {
        asm("NOP");
    }
}

/*******************************************************************************
 * Function name: AbortFn
 * parameters   : void
 * description  : used to set the response value for D command after detecting abort signal from the recorder
 */
void AbortFn()
{
    i = 0;
    /* Setting the value buffer to send back for next D0 command
     */
    //sprintf(valueBuffer, "%c%s%c%c", Address, "+0+0", 0x0D, 0x0A);
    valueBuffer[i++] = Address;
    valueBuffer[i++] = '+';
    valueBuffer[i++] = '0';
    valueBuffer[i++] = '+';
    valueBuffer[i++] = '0';
    valueBuffer[i++] = 0x0D;
    valueBuffer[i++] = 0x0A;
    valueLength = 8;
    /* if CRC requested
     */
    if (commandBuffer[1] == 'C') {
        CRC_generation();
    }
    commandflag = 0;
    cmd = '\0';
    MarkingFn();
}

/*******************************************************************************
 * Function name: StandyFn
 * parameters   : void
 * description  : Function used instead of sleep mode; waits for a valid break signal
 */
void StandbyFn(void)
{
    /* disable the SDI12 uart receiver
     */
    STKPTR = 0x00;
    SDIUSART_RX_CON.SPEN = 0;
    asm("NOP");
    SDIUSART_RX_CON.SPEN = 1;
    sdi_usart_RXIE(0); 
    sdi_usart_RXEN(0); 
    sdi_usart_TXEN(0);
    breakflag = 0;
    LATB = 0x00;
    INTERRUPT_ENABLE(1)
    while (1) {
        asm("NOP");
    }

}

/*******************************************************************************
 * Function name: SDITransmitFn
 * parameters   : void
 * description  : Function handles the transmission side of  SDI-12
 */
void SDITransmitFn(void)
{
    INTERRUPT_ENABLE(0)
    i = 0;
    while (i < bufferLength) {
        /* adding the parity bit to data (even parity)*/
        send[i] = send[i] | (PARITY(send[i])^0) << 7;
        i++;
    }
    sdi_usart_RXEN(0);
    /* Enabling the TXBUFFER control pin
     */
    TXBUFFER_ENABLE = 1;
    /* Enabling the SDI12 uart transmitter
     */
    sdi_usart_TXEN(1);
    sdi_TX_Marking();
    i = 0;
    while (i < bufferLength) {
        sdi_usart_putchar(send[i]);
        i++;
    }
    /* disable sdi-12 transmission
     */
    sdi_usart_TXEN(0);
    TXBUFFER_ENABLE = 0;
    INTERRUPT_ENABLE(1)
}