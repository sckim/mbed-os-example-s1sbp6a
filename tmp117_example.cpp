/****************************************************************************
 *
 * Copyright 2020 Samsung Electronics All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
#include "mbed.h"
#include "bp6a_example.h"

#define TMP117_ADDRESS   0x48

#define TMP117_ID_VALUE 0x0117			// Value found in the device ID register on reset (page 24 Table 3 of datasheet)
#define TMP117_RESOLUTION 0.0078125f	// Resolution of the device, found on (page 1 of datasheet)
#define CONTINUOUS_CONVERSION_MODE 0b00 // Continuous Conversion Mode
#define ONE_SHOT_MODE 0b11				// One Shot Conversion Mode
#define SHUTDOWN_MODE 0b01				// Shutdown Conversion Mode

#define TMP117_TEMP_RESULT      0X00
#define TMP117_CONFIGURATION    0x01
#define TMP117_T_HIGH_LIMIT     0X02
#define TMP117_T_LOW_LIMIT      0X03
#define TMP117_EEPROM_UL        0X04
#define TMP117_EEPROM1          0X05
#define TMP117_EEPROM2          0X06
#define TMP117_TEMP_OFFSET      0X07
#define TMP117_EEPROM3          0X08
#define TMP117_DEVICE_ID        0X0F

// Configuration register found on page 25 Figure 26 and Table 6
typedef union {
	struct
	{
		uint8_t EMPTY : 1;			// Empty bit in register
		uint8_t TMP_SOFT_RESET : 1; // Software reset bit
		uint8_t DR_ALERT : 1;		// ALERT pin select bit
		uint8_t POL : 1;			// ALERT pin polarity bit
		uint8_t T_NA : 1;			// Therm/alert mode select
		uint8_t AVG : 2;			// Conversion averaging modes
		uint8_t CONV : 3;			// Conversion cycle bit
		uint8_t MOD : 2;			// Set conversion mode
		uint8_t EEPROM_BUSY : 1;	// EEPROM busy flag
		uint8_t DATA_READY : 1;		// Data ready flag
		uint8_t LOW_ALERT : 1;		// Low Alert flag
		uint8_t HIGH_ALERT : 1;		// High Alert flag
	} CONFIGURATION_FIELDS;
	uint16_t CONFIGURATION_COMBINED;
} CONFIGURATION_REG;

// Device ID Register used for checking if the device ID is the same as declared
// This register is found on Page 30 of the datasheet in Table 15 and Figure 34
typedef union {
	struct
	{
		uint16_t DID : 12; // Indicates the device ID
		uint8_t REV : 4;   // Indicates the revision number
	} DEVICE_ID_FIELDS;
	uint16_t DEVICE_ID_COMBINED;
} DEVICE_ID_REG;

I2C *tmp117;

void tmp117_write_reg(uint8_t addr, uint8_t reg, uint8_t data)
{
    char buf[2];

    buf[0] = reg;
    buf[1] = data;
    tmp117->write(addr, (char *)buf, 2, false);
}

void tmp117_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, int len)
{
    tmp117->write(addr, (char *)&reg, 1, false);
    tmp117->read(addr, (char *)data, len, false);
}

bool tmp117_dataReady()
{
     uint8_t Buf[2] = {0,};

    tmp117_read_reg(TMP117_ADDRESS, TMP117_CONFIGURATION, Buf, 2);
    
    uint16_t response = (Buf[0] << 8 | Buf[1]);

	// If statement to see if the 13th bit of the register is 1 or not
	if (response & 1 << 13)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int tmp117_example(void)
{
    printf("i2c TMP117 example\r\n");

    ThisThread::sleep_for(1000ms);

    tmp117 = new I2C(I2C2_SDA, I2C2_SCL);

    ThisThread::sleep_for(500ms);

    uint8_t Buf[15] = {0,};
    
    tmp117_read_reg(TMP117_ADDRESS, TMP117_DEVICE_ID,  Buf, 2); // reads registers into rawData
    uint16_t deviceID=(Buf[0] << 8 | Buf[1]);

	//make sure the device ID reported by the TMP is correct
	//should always be 0x0117
	if (deviceID != TMP117_ID_VALUE)
	{
        printf("TMP117 is not installed.\r\n");
		return false;
	} else {
        printf("TMP117 is found.\r\n");
    }

    ThisThread::sleep_for(500ms);

    while (running) {
        if( tmp117_dataReady() ) {
            tmp117_read_reg(TMP117_ADDRESS, TMP117_TEMP_RESULT, Buf, 2);

            // TMP117
            int16_t digitalTempC = (Buf[0] << 8 | Buf[1]);
            float finalTempC = digitalTempC * TMP117_RESOLUTION; 

            printf("%d, %d\r\n", Buf[0], Buf[1]);
            // printf("%lf\r\n", finalTempC);
        }
        ThisThread::sleep_for(500ms);
    }
    printf("i2c TMP117 stop\r\n");

    return 0;

}
