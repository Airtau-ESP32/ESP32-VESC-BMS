#include <Arduino.h>
#include "globals.h"
#include "driver/rtc_io.h"

#include <EEPROM.h>
#include <Metro.h>
#include <ble.h>
#include "bms.h"

byte chipBaseAddress = 0x80;

Metro DeepSllepTimer = Metro(86400000); // таймер задержки сна после отключения зажигания
Metro ledBlinkTimer = Metro(250);
BMS bms;
BleCallbacks callbacks;

RTC_DATA_ATTR int bootCount = 0;
bool state = LOW;

void ledIndicator() //
{
  if (digitalRead(KEY_ON_IGN) == HIGH)
  {
    if (ledBlinkTimer.check() == true)
    {
      if (state == HIGH)
      {
        state = LOW;
      }
      else
      {
        state = HIGH;
      }
    }
  }
  else if (digitalRead(KEY_ON_IGN) == LOW)
  {
    state = !deviceConnected;
  }
  digitalWrite(PIN_RGB_LED, state);
  delay(100);

  Serial.print("LED state ");
  Serial.println(digitalRead(PIN_RGB_LED));
}

void setDeepSllepTimer(String &stringDuration)
{
  String durationPeriod = stringDuration.substring(stringDuration.length() - 1, stringDuration.length());
  durationPeriod.toUpperCase();
  int durationValue = stringDuration.substring(0, stringDuration.length() - 1).toInt();
  int timeDay = 86400000; // дни
  int timeHour = 3600000; // часы
  int timeMin = 60000;    // минуты
  int timeSec = 1000;     // секунды
  Serial.print("Duration: Value ");
  Serial.print(durationValue);
  Serial.print(" Period ");
  Serial.print(durationPeriod);

  if (durationPeriod == "D")
  {
    durationValue = durationValue * timeDay;
  }
  else if (durationPeriod == "H")
  {
    durationValue = durationValue * timeHour;
  }
  else if (durationPeriod == "M")
  {
    durationValue = durationValue * timeMin;
  }
  else if (durationPeriod == "S")
  {
    durationValue = durationValue * timeSec;
  }

  DeepSllepTimer.interval(durationValue);
  Serial.print(" Timer ");
  Serial.print(durationValue);
  Serial.println("mS");
}

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    // scanBeacons();

    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
  ledIndicator();
}

void initBLE()
{
  BLEDevice::init("LTC BMS V1");
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  BLEDevice::setMTU(MTU_SIZE);

  // создадим BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(&callbacks);

  // создадим и запустим uart BLE сервис
  uartService = pServer->createService(UARTServiceUUID);
  uartRxCharacteristic = uartService->createCharacteristic(UARTRxCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  uartRxCharacteristic->setCallbacks(&callbacks);
  uartTxCharacteristic = uartService->createCharacteristic(UARTTxCharacteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);
  uartTxCharacteristic->addDescriptor(new BLE2902());
  uartService->start();
}

void setup()
{

  Serial.begin();
  LTC6802::initSPI();

  for (byte address : BMSChips) {
    static LTC6802 chip = LTC6802(address, csPin);
    chip.cfgRead();         // Считываем конфигурацию чипа
    chip.cfgSetCDC(1);      // Длительность измерения 13 мс
    chip.cfgSetMCI(0x0fff); // отключаем прерываения
    chip.cfgWrite(false);   // Записываем конфигурацию в чип
    Serial.println("Initialized chip");
  }

  initBLE();
}

void readBmsData()
{
  for (int a = 0; a < cellNodes; a++)
  {
    // bms.readBatteryCellVoltages();
    // bms.readBatteryTemperatureAndCurrent();
  }
}

void sendBmsDataToVesc()
{
  log_d("Sending BMS data to VESC");
  // Создание сообщений VESC CAN на основе данных BMS
  // Отправка VESC CAN сообщений
  // ...
}

void receiveVescCommands()
{
  log_d("Receiving VESC commands");
  // Проверка входящих сообщений CAN шины
  // Если получена команда VESC, обрабатываем её соответствующим образом
  // ...
}

void sendBmsDataToBLE()
{
  int remaining = messageLength;
  int packetSize = PACKET_SIZE;
  int packetIndex = 0;

  while (remaining > 0)
  {
    packetSize = remaining < PACKET_SIZE ? remaining : PACKET_SIZE;
    log_i("Packet size: %d", packetSize);
    packetIndex = messageLength - remaining;
    u_int8_t payload[packetSize];
    memcpy(payload, &messageBuffer[packetIndex], packetSize * sizeof(u_int8_t));
    uartTxCharacteristic->setValue(payload, packetSize);
    remaining -= packetSize;
    uartTxCharacteristic->notify();
    delay(10); // Если будет отправлено слишком много пакетов, стек Bluetooth перегрузится

    memset(messageBuffer, 0, messageLength);
  }
}

void receiveBLECommands()
{
  log_d("Receiving BLE commands");
  // Проверка входящих сообщений со смартфона
  // Если получена команда BLT, обрабатываем её соответствующим образом
  // ...
}

void loop()
{

  // Чтение данных BMS из SPI шины
  readBmsData();
  // Отправка данных BMS по шине CAN с использованием протокола VESC
  sendBmsDataToVesc();
  // Получение и обработка команд VESC по шине CAN
  receiveVescCommands();

  // Если отключились от смартфона
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // дадим стеку Bluetooth возможность закончить обмен данными
    pServer->startAdvertising(); // перезапустим обнаружение
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Если подключились к смартфону
  if (deviceConnected && !oldDeviceConnected)
  {
    sendBmsDataToBLE();
    oldDeviceConnected = deviceConnected;
  }
  delay(100);
}
