/*  Project :  SDI-12 Header for sensors using PIC18F23k22
 */
#include "Includes.h"

DATABUFFER_TYPE dataBuffer[DATAMAX];
char serviceRequestBuffer[3]; //service request
uint8_t DATA_return, Data;

/******************************************************************************
 * Function name: measure
 * parameters   : void
 * return type  : void
 * description  : Used to process the M command, will call the sensor specific functions to read from it
 *              : value buffer is set to +9+9 if failed to read from sensor
 */
void measure(void)
{
    STKPTR = 0x00;
    i = 0;
    /* call the sensor function MAXCALL times if return with success then break
     */
    while (i != MAXCALL) {
        DATA_return = SENSOR_READ_FN[senindex](dataBuffer);
        /* The sensorFucntion will return non zero value for success (no of parameters)
         */
        if (DATA_return) {
            break;
        }
        i++;
    }
    Data = DATA_return;
    /*if error in communication with the sensor: response to D0 command is set to +9+9
     */
    if (i == MAXCALL) {
        sprintf(valueBuffer, "%c%s%c%c", Address, "+9+9", 0x0D, 0x0A);
        valueLength = 8;//strlen(valueBuffer) + 1; //8;
        commandflag = 0;
        cmd = '\0';
        Waiting();
    }
    SDI_conversion();
    /* if CRC requested
     */
    if (commandBuffer[1] == 'C') {
        CRC_generation();
    }
    /* if service request is enabled
     */
    if (SERVICE_REQUEST == 1) {
        serviceRequestBuffer[0] = Address;
        serviceRequestBuffer[1] = 0x0D;
        serviceRequestBuffer[2] = 0x0A;
        bufferLength = 3;
        send = serviceRequestBuffer;
        SDITransmitFn();
    }
    commandflag = 0;
    cmd = '\0';
    Waiting();

}

/******************************************************************************
 * Function name: SDI_conversion
 * parameters   : void
 * return type  : void
 * description  : Used to convert the data received from the sensor to SDI-12 standard: depends on individual sensors
 */
void SDI_conversion(void)
{
   /* uint16_t k = 10000;
    i = 0;
    j = 0;
    valueBuffer[j++] = Address;
    while (i != Data) {
        k = 10000;
        if (dataBuffer[i] >= 0) {
            valueBuffer[j++] = '+';
        } else {
            valueBuffer[j++] = '-';
        }
        while (k > dataBuffer[i]) {
            k = k / 10;
        }
        while (k > 0) {
            valueBuffer[j++] = dataBuffer[i] / k + 48;
            dataBuffer[i] = dataBuffer[i] % k;
            k = k / 10;
        }
        i++;
    }
    valueBuffer[j++] = 0x0D; //adding <CR>
    valueBuffer[j] = 0x0A; //adding <LF>
    valueLength = j + 1; //setting bufferlength for transmission
    */
    i=0;
    j=0;
    valueBuffer[j++] = Address;
    while (i != Data){
        sprintf(valueBuffer+j,"%+d",dataBuffer[i]);
        j=strlen(valueBuffer);
        i++;
    }
    valueBuffer[j++] = 0x0D; //adding <CR>
    valueBuffer[j] = 0x0A; //adding <LF>
    valueLength = j + 1; //setting bufferlength for transmission
}

/*******************************************************************************
 * Function name: CRC_generation
 * parameters   : void
 * return type  : void
 * description  : Used to generate CRC characters if CRC is requested by the command
 */
void CRC_generation(void)
{
    i = 0;
    uint16_t CRC = 0; 
    while (valueBuffer[i] != 0x0D) {
        CRC ^= valueBuffer[i];
        for (j = 0; j != 8; j++) {
            if (CRC & 1) {
                CRC = CRC >> 1;
                CRC ^= 0xA001;
            } else {
                CRC = CRC >> 1;
            }
        }
        i++;
    }
    //Encoding the CRC as ASCII characters
    valueBuffer[i++] = (0x40 | (CRC >> 12));
    valueBuffer[i++] = (0x40 | ((CRC >> 6)&0x3F));
    valueBuffer[i++] = (0x40 | (CRC & 0x3F));
    valueBuffer[i++] = 0x0D;
    valueBuffer[i] = 0x0A;
    //updating the buffer length for transmission
    valueLength = i + 1;

}
