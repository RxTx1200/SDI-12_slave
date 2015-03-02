#include "Includes.h"

void sendUart(char* cha);

uint8_t KDS042_read (uint16_t* Data)
{
    int temp;
    uint16_t count;
    unsigned char sen_char;
    unsigned char sencommand[]="*[SS00gsm]#\0";
    unsigned char sencommand1[]="*[SS00gdt]#\0";
    unsigned char sendata[25];
    TRISCbits.RC5 =0;
    LATCbits.LC5 =1;
    
    sensor_usart_TXEN(1);
    sensor_usart_RXEN(1);
    sen_char = NULL;
    SENSOR_USART_RX_CON.SPEN = 0;
    asm("NOP");
    SENSOR_USART_RX_CON.SPEN = 1;
    count=0;
    Data[0] =0; Data[1]=0;
    temp =0;

//soil moisture
    LATCbits.LC5  = 1;
    sendUart(sencommand);
    LATCbits.LC5 = 0;
    while(!SENSOR_USART_RX_FLAG);
//  {
//      count++;
//      if(count==32000)
//      break;
//  }
//  if(count == 32000)
//  {
//      Data[0]=0xff;
//      return (1);
//  }
    count =0;
    sen_char = SENSOR_USART_RXREG;
    do
    {
        while(!SENSOR_USART_RX_FLAG);
        sen_char = SENSOR_USART_RXREG;
        temp=temp+sen_char;
        sendata[count]=sen_char;
        count++;
    }while(sen_char!=0x0A);
    do
    {
        while(!SENSOR_USART_RX_FLAG);
        sen_char = SENSOR_USART_RXREG;
    }while(sen_char!=0x0A);
    count =0;
    do
    {
        if(sendata[count]=='+')
        {
            Data[0]=(sendata[count+1]-0x30)*100+
                    (sendata[count+2]-0x30)*10 +
                    (sendata[count+3]-0x30)*1  +
                    (sendata[count+5]-0x30)/10 +
                    (sendata[count+6]-0x30)/100;
            break;
        }
        count++;
    }while(count!=24);
    Data[0]=Data[0]*100;
 // soil temperature
    LATCbits.LC5= 1;
    sendUart(sencommand1);
    LATCbits.LC5 = 0;
    while(!SENSOR_USART_RX_FLAG);
    count =0;
    sen_char = SENSOR_USART_RXREG;
    do
    {
        while(!SENSOR_USART_RX_FLAG);
        sen_char = SENSOR_USART_RXREG;
        temp=temp+sen_char;
        sendata[count]=sen_char;
        count++;
    }while(sen_char!=0x0A);
    count =0;
    do
    {
        if(sendata[count]=='+')
        {
            Data[1]=(sendata[count+1]-0x30)*100+
                 (sendata[count+2]-0x30)*10 +
                 (sendata[count+3]-0x30)*1  +
                 (sendata[count+5]-0x30)/10 +
                 (sendata[count+6]-0x30)/100;
            break;
        }
        count++;
    }while(count!=24);
    Data[1]=Data[1]*100;
    sensor_usart_TXEN(0);
    sensor_usart_RXEN(0);
    return (2);
}

void sendUart(char* cha)
{
    int i =0,j;
    while ( cha[i]!= '\0')
    {
        j = sensor_usart_putchar(cha[i]);
        i++;
    }
}

