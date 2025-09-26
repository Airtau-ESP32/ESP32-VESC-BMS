#include <Arduino.h>

#ifndef CONFIG_BT_BLE_50_FEATURES_SUPPORTED
#error "This SoC does not support BLE5. Try using ESP32-C3, or ESP32-S3"
#else

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEUUID UARTServiceUUID = BLEUUID("5d33a837-aeed-8009-800f-0085cb2b2a42");
BLEUUID UARTRxCharacteristicUUID = BLEUUID("8f2b21d0-8f5a-8005-8004-009218e444ec");
BLEUUID UARTTxCharacteristicUUID = BLEUUID("5a99867e-3958-8007-800e-001725352ba7");

BLEServer *pServer;
BLEService *uartService;
BLECharacteristic *uartRxCharacteristic;
BLECharacteristic *uartTxCharacteristic;

uint8_t MTU_SIZE = 128;
uint8_t PACKET_SIZE = MTU_SIZE - 3;

bool deviceConnected = false;
bool oldDeviceConnected = false;

int messageLength = 0;
uint8_t messageBuffer[1000];

void setMtu(uint16_t mtu)
{
  MTU_SIZE = mtu;
  PACKET_SIZE = MTU_SIZE - 3;
}

class BleCallbacks : public BLEServerCallbacks, public BLECharacteristicCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    uint16_t mtu = pServer->getPeerMTU(pServer->getConnId());
    setMtu(mtu);
    deviceConnected = true;
    log_i("Device connected with MTU: %d", mtu);
  }

  void onDisconnect(BLEServer *pServer)
  {
    log_i("Device disconnected");
    deviceConnected = false;
    // Optional: More aggressive cleanup
    pServer->getAdvertising()->stop();
    delay(100);
    pServer->getAdvertising()->start();
  }

  void onWrite(BLECharacteristic *characteristic)
  {
    if (characteristic->getUUID().equals(UARTRxCharacteristicUUID))
    {
      Serial.write(characteristic->getData(), characteristic->getLength());
      Serial.flush();
    }
  }

  void onNotify(BLECharacteristic *characteristic)
  {
    log_i("Notification sent");
  }

  void onMtuChanged(uint16_t mtu)
  {
    log_i("MTU Size changed: %d", mtu);
    setMtu(mtu);
  }
};

#endif