#ifndef LTC6802_H
#define LTC6802_H

#include <Arduino.h>
#include <SPI.h>

class LTC6802 {
private:
    SPIClass* _spi;
    uint8_t _cs_pin;
    uint8_t _config[6];
    
    void wakeup();
    uint16_t calculatePEC(uint8_t* data, int len);
    
public:
    LTC6802(uint8_t cs_pin = 7);
    
    bool begin();
    bool writeConfig();
    bool readConfig(uint8_t* config);
    bool startCellVoltageConversion();
    bool readCellVoltages(uint8_t* cell_voltages);
    bool readAuxiliary(uint8_t* aux_data);
    bool readStatus(uint8_t* status_data);
    bool startOpenWireConversion();
    bool startSelfTest();
    
    // Утилиты для работы с данными
    float rawVoltageToFloat(uint16_t raw_voltage);
    uint16_t floatVoltageToRaw(float voltage);
    bool validatePEC(uint8_t* data, int len, uint16_t received_pec);
    
    // Команды LTC6802
    static const uint16_t WRCFG = 0x0001;   // Write Configuration
    static const uint16_t RDCFG = 0x0002;   // Read Configuration
    static const uint16_t RDCV = 0x0004;    // Read Cell Voltages
    static const uint16_t RDAUX = 0x000C;   // Read Auxiliary
    static const uint16_t RDSTAT = 0x000E;  // Read Status
    static const uint16_t ADCV = 0x0260;    // Start Cell Voltage Conversion
    static const uint16_t ADAX = 0x0460;    // Start Auxiliary Conversion
    static const uint16_t ADOW = 0x0268;    // Start Open Wire Conversion
    static const uint16_t CVST = 0x0270;    // Start Self Test
    static const uint16_t AXST = 0x0470;    // Start Auxiliary Self Test
    static const uint16_t ADOL = 0x0268;    // Start Overlap Measurement
};

#endif