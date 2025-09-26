#include <Arduino.h>
#include <bms.h>

// Чтени данных о напряжениях ячеек
void readBatteryCellVoltages()
{
  // Периодически считывает напряжение элементов питания
//  ltc.getData();
  float totalVoltage = 0.0;

  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Printing the internal teperature value of the LCT IC
  Serial.print("-    IC Temp = ");
//  Serial.print(ltc.IC_tmp);
  Serial.println(" degrees celsius   -");


    for (int z = 0; z < cellNumber; z++)
    {
      Serial.print("Cell ");
      Serial.print(z + 1);

      Serial.print(" voltage: ");
//      Serial.print(ltc.cell_voltages[z]);
//      totalVoltage = totalVoltage + ltc.cell_voltages[z];
      Serial.print("v -- ");
      Serial.print("Flag: ");
//      Serial.println(ltc.flag_cell[z]);
    }

  Serial.print("Total voltage = ");
  Serial.println(totalVoltage);

  delay(1000);
}

// Чтени данных о температуре и нагрузке
void readBatteryTemperatureAndCurrent()
{
  Serial.print("Temperature A: ");
//  Serial.println(ltc.tmp_cell[0]);
  Serial.print("Temperature B: ");
//  Serial.println(ltc.tmp_cell[1]);
  Serial.println();
}
