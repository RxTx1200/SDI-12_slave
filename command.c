/*  Project :  SDI-12 Header for sensors using PIC18F23k22
 */
#include "Includes.h"

/* command types
 */
enum {
    ADDRESS_QUERY = 1, ADDRESS_CHANGE, MEASURE, DATA, ID
};

char* TTTN = MeasureResponse[0];
uint8_t cmdtype; //holds command type
volatile uint8_t senindex = 0;

/*******************************************************************************
 * Function name: command
 * parameters   : void
 * description  : Used to process the received command and send back the appropriate response
 */
void command(void)
{
    STKPTR = 0x00;
    /* disabling timeout detection
     */
    MARKING_TIMEOUT_COMPARE_IE = 0;
    MARKING_TIMEOUT_TIMER_EN = 0;
    cmdtype = validCommandCheck();
   /* invalid command -> marking
    */
    if (!cmdtype) 
    {
        MarkingFn();
    }
    for (i = 0; i < SENNO; i++) {
        if (Address == AddressSet[i]) {
            senindex = i;
            break;
        }
    }
    /* address not found -> default condition
     */
    if (i == SENNO) {
        Waiting();
    }
    commandflag = 1; 
    cmd = commandBuffer[0]; 
    commandResponse(); 
    SDITransmitFn(); 
    if (cmd == 'M') {
        measure();
    }
    commandflag = 0; 
    cmd = '\0'; 
    Waiting();
    
}

/*******************************************************************************
 * Function name: validCommandCheck
 * parameters   : void
 * return type  : uint8_t
 * description  : Used to check the validity of received command word and return the corresponding command type
 */
uint8_t validCommandCheck(void)
{
    if (commandBuffer[0] == '!') {
        return 1;
    }
    switch (commandBuffer[0]) {
            /* Address change command
             */
        case 'A':
            if ((commandBuffer[1] > 47 && commandBuffer[1] < 58) ||
                    (commandBuffer[1] > 64 && commandBuffer[1] < 91) ||
                    (commandBuffer[1] > 96 && commandBuffer[1] < 123)) {
                if (commandBuffer[2] == '!')
                    return 2;
            }
            return 0;
            /* Measurement command
             */
        case 'M':
            if (commandBuffer[1] == '!')
                return 3;
            if (commandBuffer[1] == 'C' && commandBuffer[2] == '!')
                return 3;
            return 0;
            /* Send Data command
             */
        case 'D':
            if (commandBuffer[1] == '0' && commandBuffer [2] == '!')
                return 4;
            return 0;
            /* Identification
             */
        case 'I':
            if (commandBuffer[1] == '!')
                return 5;           
            return 0;
        default:
            return 0;
    }
}

/*******************************************************************************
 * Function name: commandResponse
 * parameters   : void
 * return type  : void
 * description  : Used to set the appropriate response to received command
 */
void commandResponse(void) {
    i = 0;
    switch (cmdtype) {
        case ADDRESS_QUERY:
            // responseBuffer[i++]=Address;
            strncpy(responseBuffer, AddressSet, ((SENNO) - 1));
            i = (SENNO) - 1;
            break;
        case ADDRESS_CHANGE:
            //updating the SDI12 Address
            AddressSet[senindex] = commandBuffer[1];
            Write_b_eep(EEPROM_ADDBYTE + senindex, commandBuffer[1]);
            Busy_eep();
            Address = Read_b_eep(EEPROM_ADDBYTE);
            responseBuffer[i++] = Address;
            break;
        case MEASURE:
            TTTN = MeasureResponse[senindex];
            sprintf(responseBuffer, "%c%s", Address, TTTN);
            i = strlen(responseBuffer);
            break;
        case DATA:
            bufferLength = valueLength;
            send = valueBuffer;
            return;
        case ID:
            sprintf(responseBuffer, "%c%s", Address, IdResponse + senindex);
            i = strlen(responseBuffer);
            return;
    }
    /* Append the terminating characters
     */
    responseBuffer[i++] = 0x0D;
    responseBuffer[i] = 0x0A;
    bufferLength = (i + 1);
    send = responseBuffer;
    return;

}
