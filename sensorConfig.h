
#ifndef SENSORCONFIG_H
#define	SENSORCONFIG_H

#define KDS042      0   //Soil mositure in VWC
#define SHT15       1   //Relative Humidity and Temperature
#define LW          1   //Leaf Wetness
#define DS18B20     1   //Soil Temperature
#define GYPSUM      1   //Soil mositure in pressure

#include <stdio.h>

#if KDS042
    #include "KDS042.h"
#endif

#if SHT15
    #include "SHT15.h"
#endif

#if LW
    #include "LeafWetness.h"
#endif

#if DS18B20
    #include "DS18B20.h"
#endif

#if GYPSUM
    #include "GYPSUM.h"
#endif

#define SENNO               2+KDS042+SHT15+LW+DS18B20+GYPSUM+1      //No of sensor supported +2 for default and address query.


//Adress index for different sensors: manually configured
volatile char AddressSet [SENNO]={ '0' //default ;no measurement
#if KDS042
                                   ,'g' //KDS-042
#endif
#if SHT15
                                  ,'j' //SHT-15
                                  //,'l' //SHT-15
#endif
#if LW
                                   ,'m' //LeafWetness
#endif
#if DS18B20
                                   ,'p' //DHT11
#endif
#if GYPSUM
                                   ,'s' //GYPSUM
                                 // ,'t' //GYPSUM
#endif
                                   ,'?' //address query
                                };


char * IdResponse [SENNO]       = { "\0",
                                     "C-DAC\0",
                                  };

char MeasureResponse [SENNO][5]={ "\0"
#if KDS042
                                  ,"0102\0" //KDS042
#endif
#if SHT15
                                  ,"0052\0" //SHT15
            //                    ,"0052\0" //SHT15
#endif
#if LW
                                  ,"0031\0"  //LeafWetness
#endif
#if DS18B20
                                  ,"0031\0"  //DHT11
#endif
#if GYPSUM
                                  ,"0051\0"  //GYPSUM
#endif
                                };

/* Macro for defining the maximum buffer size used to receive sensor data: depends on individual sensors*/
#define DATAMAX             3

/* defining the data type of the data buffer*/
typedef uint16_t     DATABUFFER_TYPE;

/*Function pointer to call the sensor specific function to read sensor value*/
typedef   uint8_t (*sensorFunction)(DATABUFFER_TYPE*);

sensorFunction  SENSOR_READ_FN [SENNO]={   NULL
#if KDS042
                                           ,KDS042_read
#endif
#if SHT15
                                           ,SHT15_read
             //                            ,SHT15_read_1
#endif
#if LW
                                           ,LeafWetness_read
#endif
#if DS18B20
                                           ,DS18B20_read
#endif
#if GYPSUM
                                           ,GYPSUM_read
#endif
                                         };
                                        
#endif	/* SENSORCONFIG_H */
