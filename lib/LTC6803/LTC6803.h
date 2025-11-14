#ifndef LTC6803_H
#define LTC6803_H

#include <Arduino.h>
#include <SPI.h>

class LTC6803 {
private:
    SPIClass* _spi;
    uint8_t _cs_pin;
    uint8_t _config[6];
    bool _is_initialized;
    
    void wakeup();
    uint16_t calculatePEC14(uint8_t* data, int len);
    void sendCommand(uint16_t command);
    bool readRegister(uint16_t command, uint8_t* rx_data, int data_len);
    
public:
    LTC6803(uint8_t cs_pin = 7);
    ~LTC6803();
    
    bool begin();
    bool writeConfiguration();
    bool readConfiguration(uint8_t* config);
    bool startCellVoltageADC();
    bool readCellVoltages(uint16_t* cell_voltages);
    bool startAuxiliaryADC();
    bool readAuxiliary(uint16_t* aux_data);
    bool readStatus(uint8_t* status_data);
    bool startSelfTest();
    bool startOpenWireTest();
    
    // Управление балансировкой
    bool setDischarge(uint16_t discharge_mask);
    bool clearDischarge();
    
    // Утилиты
    float rawCellVoltageToFloat(uint16_t raw_voltage);
    float rawAuxVoltageToFloat(uint16_t raw_voltage);
    bool validatePEC(uint8_t* data, int len, uint16_t received_pec);
    
    // Команды LTC6803
    static const uint16_t WRCFG = 0x0001;   // Write Configuration Register Group
    static const uint16_t RDCFG = 0x0002;   // Read Configuration Register Group
    static const uint16_t RDCVA = 0x0004;   // Read Cell Voltage Register Group A
    static const uint16_t RDCVB = 0x0006;   // Read Cell Voltage Register Group B
    static const uint16_t RDCVC = 0x0008;   // Read Cell Voltage Register Group C
    static const uint16_t RDCVD = 0x000A;   // Read Cell Voltage Register Group D
    static const uint16_t RDAUXA = 0x000C;  // Read Auxiliary Register Group A
    static const uint16_t RDAUXB = 0x000E;  // Read Auxiliary Register Group B
    static const uint16_t RDSTATA = 0x0010; // Read Status Register Group A
    static const uint16_t RDSTATB = 0x0012; // Read Status Register Group B
    static const uint16_t ADCV = 0x0260;    // Start Cell Voltage ADC Conversion
    static const uint16_t ADAX = 0x0460;    // Start GPIO ADC Conversion
    static const uint16_t ADOW = 0x0268;    // Start Open Wire ADC Conversion
    static const uint16_t CVST = 0x0270;    // Start Self Test Cell Voltage Conversion
    static const uint16_t AXST = 0x0470;    // Start Self Test GPIO Conversion
    static const uint16_t ADOL = 0x0268;    // Start Overlap Measurement
    static const uint16_t CLRCELL = 0x0711; // Clear Cell Voltage Register Groups
    static const uint16_t CLRAUX = 0x0712;  // Clear Auxiliary Register Groups
    static const uint16_t CLRSTAT = 0x0713; // Clear Status Register Groups
    static const uint16_t PLADC = 0x0714;   // Poll ADC Conversion Status
    static const uint16_t DIAGN = 0x0715;   // Diagnose MUX and Poll Status
    static const uint16_t WRCOMM = 0x0721;  // Write COMM Register Group
    static const uint16_t RDCOMM = 0x0722;  // Read COMM Register Group
    static const uint16_t STCOMM = 0x0723;  // Start I2C/SPI Communication
    
    // Конфигурационные биты
    static const uint8_t REFON = 0x04;      // Reference ON
    static const uint8_t ADCOPT = 0x01;     // ADC Mode Option
    static const uint8_t UNDERVOLTAGE_THRESHOLD = 0x00;
    static const uint8_t OVERVOLTAGE_THRESHOLD = 0x00;
};

#endif