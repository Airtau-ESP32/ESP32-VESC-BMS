#ifndef CHIP_DETECTOR_H
#define CHIP_DETECTOR_H

#include <Arduino.h>
#include <SPI.h>

enum ChipType {
    CHIP_UNKNOWN = 0,
    CHIP_LTC6802 = 1,
    CHIP_LTC6803 = 2,
    CHIP_LTC6804 = 3  // для будущего расширения
};

struct ChipDetectionResult {
    ChipType type;
    bool is_responding;
    uint8_t status_register;
    String error_message;
};

class ChipDetector {
private:
    uint8_t _cs_pin;
    SPIClass* _spi;
    
    void wakeupChip();
    bool sendCommand(uint16_t command);
    bool readRegister(uint16_t command, uint8_t* data, uint8_t len, uint16_t* pec_error = nullptr);
    uint16_t calculatePEC14(uint8_t* data, int len);
    ChipDetectionResult testLTC6802();
    ChipDetectionResult testLTC6803();
    bool validatePEC(uint8_t* data, int len, uint16_t received_pec);
    
public:
    ChipDetector(uint8_t cs_pin);
    ~ChipDetector();
    
    bool begin();
    ChipDetectionResult detectChip();
    ChipDetectionResult detectChipAdvanced();
    bool testCommunication();
    void scanMultipleChips(uint8_t start_cs, uint8_t end_cs);
    String chipTypeToString(ChipType type);
};

#endif