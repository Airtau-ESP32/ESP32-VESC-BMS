#ifndef __GLOBALS_H
#define __GLOBALS_H

#include "bms.h"

#define I_ADC A0        // Пин, показаний датчика тока
#define INV_OUT A1      // Пин, по которому отправляем сигнал аварии 
#define KEY_ON_IGN A3   // Пин, к которому подключен сигнал зажигания HIGH - просыпаемся, LOW - засыпаем
#define KEY_ON_PIN_MASK (1ULL << KEY_ON_IGN) // маска события пробуждения


#define HW_UUID ESP.getEfuseMac()
#define HW_UUID_8 (uint8_t)HW_ID

#define HW_NAME "ESP32-BMS-Bridge"
#define FW_VERSION_MAJOR 0
#define FW_VERSION_MINOR 1

extern BMSData bmsData; 

#endif