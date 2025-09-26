#ifndef __BMS_H
#define __BMS_H
#include <LTC6802.h>


int csPin = 10;
int cellNodes = 16;  // количество сборок
int cellNumber = 12; // количество ячеек в сборке
float uv_limit = 1.0;       // Предельное напряжение для элементов — не менее -0,3 В
float ov_limit = 5.0;       // Максимальное допустимое напряжение для элементов составляет 5 вольт

struct BMSData
{
  int totalVoltage;         // Общее напряжение батареи
  int current;              // Зарядный ток
  int soc;                  // Состояние заряда (%)
  int soh;                  // Здоровье батареи (%)
  int cycleCount;           // Общее количество циклов упаковки
  int protectionStatus[3];  // Primary, secondary, tertiary
  int cellVoltages[16][12]; // массив напряжений отдельных ячеек
  int busNodeaddress[16];   // массив адресов сборок
  int balancingStatus[16];  // 1-12S, 13-24S и т.д.
  int temperatures[16][2];  // массив температурных датчиков
};

int BMSChips[16] = {0x80,0x81,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x90};

class BMS
{
public:

  void readBatteryCellVoltages(); // Чтение данных о напряжениях ячеек

  void readBatteryTemperatureAndCurrent(); // Чтени данных о температуре и нагрузке
};
#endif
