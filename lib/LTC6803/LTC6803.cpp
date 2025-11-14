#include "LTC6803.h"

LTC6803::LTC6803(uint8_t cs_pin) {
    _cs_pin = cs_pin;
    _spi = new SPIClass(HSPI);
    _is_initialized = false;
    
    // Конфигурация по умолчанию
    _config[0] = 0x00 | REFON;  // REFON=1, ADCOPT=0
    _config[1] = 0x00;          // Undervoltage Comparison Voltage 1
    _config[2] = 0x00;          // Undervoltage Comparison Voltage 2
    _config[3] = 0x00;          // Overvoltage Comparison Voltage 1
    _config[4] = 0x00;          // Overvoltage Comparison Voltage 2
    _config[5] = 0x00;          // Discharge Control
}

LTC6803::~LTC6803() {
    if (_spi) {
        delete _spi;
    }
}

bool LTC6803::begin() {
    if (_is_initialized) {
        return true;
    }
    
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
    
    // Инициализация SPI
    _spi->begin(4, 5, 6); // SCK, MISO, MOSI для ESP32-C3
    _spi->setFrequency(1000000); // 1 MHz
    _spi->setDataMode(SPI_MODE3);
    _spi->setBitOrder(MSBFIRST);
    
    _is_initialized = true;
    
    // Записываем конфигурацию
    return writeConfiguration();
}

void LTC6803::wakeup() {
    digitalWrite(_cs_pin, LOW);
    delayMicroseconds(1);
    digitalWrite(_cs_pin, HIGH);
    delayMicroseconds(300);
}

uint16_t LTC6803::calculatePEC14(uint8_t* data, int len) {
    uint16_t pec = 0x0010;
    uint16_t polynomial = 0x0015;
    
    for (int i = 0; i < len; i++) {
        pec ^= (data[i] << 7);
        for (int bit = 0; bit < 8; bit++) {
            if (pec & 0x4000) {
                pec = (pec << 1) ^ polynomial;
            } else {
                pec = pec << 1;
            }
        }
    }
    
    return pec & 0xFFFF;
}

void LTC6803::sendCommand(uint16_t command) {
    uint8_t cmd[2] = {(uint8_t)(command >> 8), (uint8_t)(command & 0xFF)};
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    _spi->transfer(cmd[0]);
    _spi->transfer(cmd[1]);
    digitalWrite(_cs_pin, HIGH);
}

bool LTC6803::readRegister(uint16_t command, uint8_t* rx_data, int data_len) {
    uint8_t tx_data[2] = {(uint8_t)(command >> 8), (uint8_t)(command & 0xFF)};
    uint8_t buffer[32];
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    // Отправляем команду чтения
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    // Читаем данные + PEC
    for (int i = 0; i < data_len + 2; i++) {
        buffer[i] = _spi->transfer(0x00);
    }
    
    digitalWrite(_cs_pin, HIGH);
    
    // Проверяем PEC
    uint16_t received_pec = (buffer[data_len] << 8) | buffer[data_len + 1];
    if (!validatePEC(buffer, data_len, received_pec)) {
        return false;
    }
    
    // Копируем данные
    memcpy(rx_data, buffer, data_len);
    return true;
}

bool LTC6803::validatePEC(uint8_t* data, int len, uint16_t received_pec) {
    uint16_t calculated_pec = calculatePEC14(data, len);
    return (received_pec == calculated_pec);
}

bool LTC6803::writeConfiguration() {
    uint8_t tx_data[8];
    
    tx_data[0] = (WRCFG >> 8) & 0xFF;
    tx_data[1] = WRCFG & 0xFF;
    
    // Копируем конфигурацию
    for (int i = 0; i < 6; i++) {
        tx_data[i + 2] = _config[i];
    }
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    for (int i = 0; i < 8; i++) {
        _spi->transfer(tx_data[i]);
    }
    
    digitalWrite(_cs_pin, HIGH);
    
    return true;
}

bool LTC6803::readConfiguration(uint8_t* config) {
    uint8_t rx_data[8];
    
    if (!readRegister(RDCFG, rx_data, 6)) {
        return false;
    }
    
    memcpy(config, rx_data, 6);
    return true;
}

bool LTC6803::startCellVoltageADC() {
    sendCommand(ADCV);
    delay(13); // Максимальное время преобразования ADC
    return true;
}

bool LTC6803::readCellVoltages(uint16_t* cell_voltages) {
    uint8_t rx_data[8];
    bool success = true;
    
    // Читаем группу A (ячейки 1-3)
    if (readRegister(RDCVA, rx_data, 6)) {
        cell_voltages[0] = (rx_data[0] << 8) | rx_data[1];
        cell_voltages[1] = (rx_data[2] << 8) | rx_data[3];
        cell_voltages[2] = (rx_data[4] << 8) | rx_data[5];
    } else {
        success = false;
    }
    
    // Читаем группу B (ячейки 4-6)
    if (readRegister(RDCVB, rx_data, 6)) {
        cell_voltages[3] = (rx_data[0] << 8) | rx_data[1];
        cell_voltages[4] = (rx_data[2] << 8) | rx_data[3];
        cell_voltages[5] = (rx_data[4] << 8) | rx_data[5];
    } else {
        success = false;
    }
    
    // Читаем группу C (ячейки 7-9)
    if (readRegister(RDCVC, rx_data, 6)) {
        cell_voltages[6] = (rx_data[0] << 8) | rx_data[1];
        cell_voltages[7] = (rx_data[2] << 8) | rx_data[3];
        cell_voltages[8] = (rx_data[4] << 8) | rx_data[5];
    } else {
        success = false;
    }
    
    // Читаем группу D (ячейки 10-12)
    if (readRegister(RDCVD, rx_data, 6)) {
        cell_voltages[9] = (rx_data[0] << 8) | rx_data[1];
        cell_voltages[10] = (rx_data[2] << 8) | rx_data[3];
        cell_voltages[11] = (rx_data[4] << 8) | rx_data[5];
    } else {
        success = false;
    }
    
    return success;
}

bool LTC6803::startAuxiliaryADC() {
    sendCommand(ADAX);
    delay(13);
    return true;
}

bool LTC6803::readAuxiliary(uint16_t* aux_data) {
    uint8_t rx_data[8];
    bool success = true;
    
    // Читаем группу A (GPIO1-GPIO3)
    if (readRegister(RDAUXA, rx_data, 6)) {
        aux_data[0] = (rx_data[0] << 8) | rx_data[1];
        aux_data[1] = (rx_data[2] << 8) | rx_data[3];
        aux_data[2] = (rx_data[4] << 8) | rx_data[5];
    } else {
        success = false;
    }
    
    // Читаем группу B (GPIO4-GPIO5, Vref2)
    if (readRegister(RDAUXB, rx_data, 6)) {
        aux_data[3] = (rx_data[0] << 8) | rx_data[1];
        aux_data[4] = (rx_data[2] << 8) | rx_data[3];
        aux_data[5] = (rx_data[4] << 8) | rx_data[5];
    } else {
        success = false;
    }
    
    return success;
}

bool LTC6803::readStatus(uint8_t* status_data) {
    uint8_t rx_data[8];
    
    if (!readRegister(RDSTATA, rx_data, 6)) {
        return false;
    }
    
    // Копируем статусные регистры
    for (int i = 0; i < 6; i++) {
        status_data[i] = rx_data[i];
    }
    
    return true;
}

bool LTC6803::startSelfTest() {
    sendCommand(CVST);
    delay(13);
    return true;
}

bool LTC6803::startOpenWireTest() {
    sendCommand(ADOW);
    delay(13);
    return true;
}

bool LTC6803::setDischarge(uint16_t discharge_mask) {
    // Устанавливаем биты разряда в конфигурации
    _config[5] = (uint8_t)(discharge_mask & 0xFF);
    _config[4] = (uint8_t)((discharge_mask >> 8) & 0xFF);
    
    return writeConfiguration();
}

bool LTC6803::clearDischarge() {
    _config[4] = 0x00;
    _config[5] = 0x00;
    
    return writeConfiguration();
}

float LTC6803::rawCellVoltageToFloat(uint16_t raw_voltage) {
    // LTC6803: 16-bit ADC, 0-5V range
    return (raw_voltage * 5.0f) / 65535.0f;
}

float LTC6803::rawAuxVoltageToFloat(uint16_t raw_voltage) {
    // Для GPIO: 16-bit ADC, 0-3V range
    return (raw_voltage * 3.0f) / 65535.0f;
}