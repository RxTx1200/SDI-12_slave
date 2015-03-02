/* 
 * File:   SDI_12.h
 * Author: Praveen James
 *
 * Created on September 13, 2013, 2:11 PM
 */

#ifndef SDI_12_H
#define	SDI_12_H

    #define SYS_FOSC            8000000
    #define FOSC                8000000/4


    /* Max Buffer size*/

    #define COMMAND_MAX         5       //command buffer max size
    #define SEN_DATA_VALUE_MAX  39      //sensor value buffer max size 39 if M command alone i used
    #define SEN_RESPONSE_MAX    10      //sensor response buffer max size 35 if identification command is used


    /* Macros for  EEPROM Addressing*/
    #define EEPROM_SENNO        0x0A    //sensor no location in EEPROM
    #define EEPROM_ADDBYTE      0x0B    //Sensor address location in EEPROM



    /* Macros for defining Break*/								
    #define BREAK_TIMER_CLK     FOSC	//Clock for Break detection timer
    #define BREAK_TIMER_PRE     1       //Prescale for Break Timer
    #define BREAK               12      //Break in ms
    #define BREAK_TIMER_REGVAL  ( 65535-BREAK*BREAK_TIMER_CLK/BREAK_TIMER_PRE/1000 ) //

     /* Macros for Marking and Timeout*/
    #define MARKING_TIMEOUT_TIMER_CLK FOSC

    /* Macros for defining Marking and Timeout*/
    #define MARKING_TIME_D      8      //marking time in ms
    #define MARKING_TIME_G      8.35      //marking time for transmission in ms
    #define TIMEOUT_TIME        100    //timeout time in ms
    #define MARKING_TIMEOUT_PRESCALAR   8

    #define MARKING_VALUE_D     MARKING_TIME_D*MARKING_TIMEOUT_TIMER_CLK/MARKING_TIMEOUT_PRESCALAR/1000
    #define TIMEOUT_VALUE       TIMEOUT_TIME*MARKING_TIMEOUT_TIMER_CLK/MARKING_TIMEOUT_PRESCALAR/1000
    #define MARKING_VALUE_G     65535-MARKING_TIME_G*FOSC/MARKING_TIMEOUT_PRESCALAR/1000

/* Function Declaration*/

    void AbortFn(void);
    void MarkingFn(void);
    void BreakFn(void);
    void Waiting(void);
    void StandbyFn(void);
    void SDITransmitFn(void);



#endif	/* SDI_12_H */























