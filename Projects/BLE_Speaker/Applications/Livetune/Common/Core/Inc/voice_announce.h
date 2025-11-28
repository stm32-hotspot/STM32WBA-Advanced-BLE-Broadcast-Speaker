/**
******************************************************************************
* @file
* @author  MCD Application Team
* @brief
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

#include "stdint.h"

#define ADVERTISING_LEN   16000
#define CONNECTED_LEN     16000
#define DISCONNECTED_LEN  16000
#define STANDBY_LEN       16000
#define LOW_BATTERY_LEN   16000
#define SYNCHRONIZED_LEN  16000
#define SCANNING_LEN      12000
#define BIP_LEN           1600
#define EXAMPLE_LEN       94000

extern const int16_t Advertising_16kHz[];
extern const int16_t Connected_16kHz[];
extern const int16_t Disconnected_16kHz[];
extern const int16_t Standby_16kHz[];
extern const int16_t LowBattery_16kHz[];
extern const int16_t Synchronized_16kHz[];
extern const int16_t Scanning_16kHz[];
extern const int16_t Bip_16kHz[];
extern const int16_t Example_16kHz[];