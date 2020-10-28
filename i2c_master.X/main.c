/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.169.0
        Device            :  dsPIC33CK128MP206
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.50
        MPLAB 	          :  MPLAB X v5.40
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "mcc_generated_files/system.h"

#define FCY (_XTAL_FREQ/2)
#include "libpic30.h"

#include "mcc_generated_files/i2c2.h"
#include "mcc_generated_files/pin_manager.h"
#include "stdio.h"


#define SLAVE_I2C_GENERIC_RETRY_MAX           100
#define SLAVE_I2C_GENERIC_DEVICE_TIMEOUT      50   // define slave timeout 

#define TEMP_SENSOR_SLAVE_ADDR                  0x004B
#define TEMP_SENSOR_REG_ADDR                    0x00

uint16_t slaveDeviceAddress = TEMP_SENSOR_SLAVE_ADDR;
uint8_t i2cWrData = TEMP_SENSOR_REG_ADDR;
uint8_t i2cRdData[2] = {0};

uint8_t temperatureVal = 0;

static uint8_t getTemperature(uint8_t* rawTempValue) {
    int16_t temp;
    // Convert the temperature value read from sensor to readable format (Degree Celsius)
    // For demonstration purpose, temperature value is assumed to be positive.
    // The maximum positive temperature measured by sensor is +125 C
    temp = (rawTempValue[0] << 8) | rawTempValue[1];
    temp = (temp >> 7) * 0.5;
    //temp = (temp * 9 / 5) + 32;
    return (uint8_t) temp;
}

/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    while (1)
    {
        uint8_t     writeBuffer[3];
        uint16_t    timeOut, slaveTimeOut;                 
        I2C2_MESSAGE_STATUS status;
        I2C2_TRANSACTION_REQUEST_BLOCK readTRB[2];       
        
        // this initial value is important
        writeBuffer[0] = i2cWrData;
        
        I2C2_MasterWriteTRBBuild( &readTRB[0],
                                    writeBuffer,
                                    1,
                                    slaveDeviceAddress);
       
        // Build TRB for receiving data
        I2C2_MasterReadTRBBuild( &readTRB[1],
                                    i2cRdData,
                                    2,
                                    slaveDeviceAddress);
        
        timeOut = 0;
        slaveTimeOut = 0;        
        
        while(status != I2C2_MESSAGE_FAIL)
        {
            // now send the transactions
            I2C2_MasterTRBInsert(2, readTRB, &status); 

            // wait for the message to be sent or status has changed.
            // status will be pending in function I2C2_MasterTRBInsert().
            while(status == I2C2_MESSAGE_PENDING)
            {
                // add some delay here
                __delay_us(100);
                // timeout checking
                // check for max retry and skip this byte
                if (slaveTimeOut == SLAVE_I2C_GENERIC_DEVICE_TIMEOUT)
                    return (0);
                else
                    slaveTimeOut++;
            }

            if (status == I2C2_MESSAGE_COMPLETE)
            {
                temperatureVal = getTemperature(i2cRdData);

                printf("temperature = %d\n\r", temperatureVal);
                
                break;
            }                
                

            // if status is  I2C2_MESSAGE_ADDRESS_NO_ACK,
            //               or I2C2_DATA_NO_ACK,
            // The device may be busy and needs more time for the last
            // write so we can retry writing the data, this is why we
            // use a while loop here

            __delay_us(100);
            // check for max retry and skip this byte
            if (timeOut == SLAVE_I2C_GENERIC_RETRY_MAX)
                return (0);
            else
                timeOut++;

        }        
        
        __delay_ms(1000);

        IO_RA0_Toggle();
    
    }
        
    return 1; 
}
/**
 End of File
*/

