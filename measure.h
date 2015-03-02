/* 
 * File:   measure.h
 * Author: Praveen James
 *
 * Created on October 8, 2013, 4:16 PM
 */

#ifndef MEASURE_H
#define	MEASURE_H


/* Macro for defining the function which is to be called to read from the sensor: depends on individual sensors*/


/* Macro for defining number of times the sensor function call before returning the erro*/
#define MAXCALL             3

/* Macro for enabling the service request to the master*/
#define SERVICE_REQUEST     0

#ifndef COMMAND_MAX
        #define COMMAND_MAX         5           //command buffer max size
        #define SEN_DATA_VALUE_MAX  39          //sensor value buffer max size 39 if M command alone i used
        #define SEN_RESPONSE_MAX    10          //sensor response buffer max size 35 if identification command is used
#endif
extern volatile bool breakflag, abortflag, commandflag;

extern volatile char commandBuffer[COMMAND_MAX], valueBuffer[SEN_DATA_VALUE_MAX], responseBuffer[SEN_RESPONSE_MAX];

extern volatile char Address, cmd;

extern volatile uint8_t bufferLength, valueLength,i,j,senindex;

extern volatile char* send;

/* Functions used for processing the measure command*/
void measure(void);
void SDI_conversion(void);
void CRC_generation(void);


#endif	/* MEASURE_H */

