/* 
 * File:   command.h
 * Author: Praveen James
 *
 * Created on October 8, 2013, 10:15 AM
 */

#ifndef COMMAND_H
#define	COMMAND_H

//#define TTTN "0052"

#ifndef COMMAND_MAX
        #define COMMAND_MAX         5         //command buffer max size
        #define SEN_DATA_VALUE_MAX  39        //sensor value buffer max size 39 if M command alone i used
        #define SEN_RESPONSE_MAX   10         //sensor response buffer max size 35 if identification command is used
#endif
extern volatile bool breakflag, abortflag, commandflag;

extern volatile char commandBuffer[COMMAND_MAX], valueBuffer[SEN_DATA_VALUE_MAX], responseBuffer[SEN_RESPONSE_MAX],AddressSet[SENNO];

extern volatile char Address, cmd;

extern volatile uint8_t bufferLength, valueLength,i;

extern volatile char* send;

/*Functions related to command processing*/

uint8_t validCommandCheck(void);
void    commandResponse(void);
void    command(void);



#endif	/* COMMAND_H */

