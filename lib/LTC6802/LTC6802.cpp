#include "LTC6802.h"

LTC6802::LTC6802(uint8_t cs_pin) {
    _cs_pin = cs_pin;
    _spi = new SPIClass(HSPI);
    
    // Конфигурация по умолчанию
    _config[0] = 0xF0;  // REFON=1, ADCOPT=0, GPIO=0
    _config[1] = 0x00;  // VUV
    _config[2] = 0x00;  // VOV
    _config[3] = 0x00;  // DCC
    _config[4] = 0x00;  // DCTO
    _config[5] = 0x00;  // Reserved
}

bool LTC6802::begin() {
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
    
    // Инициализация SPI
    _spi->begin(4, 5, 6); // SCK, MISO, MOSI для ESP32-C3
    _spi->setFrequency(1000000); // 1 MHz
    _spi->setDataMode(SPI_MODE3);
    _spi->setBitOrder(MSBFIRST);
    
    // Записываем конфигурацию
    return writeConfig();
}

void LTC6802::wakeup() {
    digitalWrite(_cs_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(_cs_pin, HIGH);
    delayMicroseconds(300);
}

uint16_t LTC6802::calculatePEC(uint8_t* data, int len) {
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

bool LTC6802::validatePEC(uint8_t* data, int len, uint16_t received_pec) {
    uint16_t calculated_pec = calculatePEC(data, len);
    return (received_pec == calculated_pec);
}

bool LTC6802::writeConfig() {
    uint8_t tx_data[8];
    
    // Формируем пакет для записи конфигурации
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

bool LTC6802::readConfig(uint8_t* config) {
    uint8_t tx_data[2] = {(RDCFG >> 8) & 0xFF, RDCFG & 0xFF};
    uint8_t rx_data[8];
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    // Отправляем команду
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    // Читаем данные
    for (int i = 0; i < 8; i++) {
        rx_data[i] = _spi->transfer(0x00);
    }
    
    digitalWrite(_cs_pin, HIGH);
    
    // Проверяем PEC
    uint16_t received_pec = (rx_data[6] << 8) | rx_data[7];
    if (!validatePEC(rx_data, 6, received_pec)) {
        return false;
    }
    
    // Копируем конфигурацию
    for (int i = 0; i < 6; i++) {
        config[i] = rx_data[i];
    }
    
    return true;
}

bool LTC6802::startCellVoltageConversion() {
    uint8_t tx_data[2] = {(ADCV >> 8) & 0xFF, ADCV & 0xFF};
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    digitalWrite(_cs_pin, HIGH);
    
    delay(10); // Ждем завершения преобразования
    
    return true;
}

bool LTC6802::readCellVoltages(uint8_t* cell_voltages) {
    uint8_t tx_data[2] = {(RDCV >> 8) & 0xFF, RDCV & 0xFF};
    uint8_t rx_data[20]; // 18 байт данных + 2 байта PEC
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    // Отправляем команду
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    // Читаем данные (6 регистров по 3 байта + PEC)
    for (int i = 0; i < 20; i++) {
        rx_data[i] = _spi->transfer(0x00);
    }
    
    digitalWrite(_cs_pin, HIGH);
    
    // Проверяем PEC
    uint16_t received_pec = (rx_data[18] << 8) | rx_data[19];
    if (!validatePEC(rx_data, 18, received_pec)) {
        return false;
    }
    
    // Копируем данные напряжений
    for (int i = 0; i < 18; i++) {
        cell_voltages[i] = rx_data[i];
    }
    
    return true;
}

bool LTC6802::readAuxiliary(uint8_t* aux_data) {
    uint8_t tx_data[2] = {(RDAUX >> 8) & 0xFF, RDAUX & 0xFF};
    uint8_t rx_data[8];
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    // Отправляем команду
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    // Читаем данные (2 регистра по 3 байта + PEC)
    for (int i = 0; i < 8; i++) {
        rx_data[i] = _spi->transfer(0x00);
    }
    
    digitalWrite(_cs_pin, HIGH);
    
    // Проверяем PEC
    uint16_t received_pec = (rx_data[6] << 8) | rx_data[7];
    if (!validatePEC(rx_data, 6, received_pec)) {
        return false;
    }
    
    // Копируем данные
    for (int i = 0; i < 6; i++) {
        aux_data[i] = rx_data[i];
    }
    
    return true;
}

bool LTC6802::readStatus(uint8_t* status_data) {
    uint8_t tx_data[2] = {(RDSTAT >> 8) & 0xFF, RDSTAT & 0xFF};
    uint8_t rx_data[8];
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    // Отправляем команду
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    // Читаем данные (2 регистра по 3 байта + PEC)
    for (int i = 0; i < 8; i++) {
        rx_data[i] = _spi->transfer(0x00);
    }
    
    digitalWrite(_cs_pin, HIGH);
    
    // Проверяем PEC
    uint16_t received_pec = (rx_data[6] << 8) | rx_data[7];
    if (!validatePEC(rx_data, 6, received_pec)) {
        return false;
    }
    
    // Копируем данные статуса
    for (int i = 0; i < 6; i++) {
        status_data[i] = rx_data[i];
    }
    
    return true;
}

bool LTC6802::startOpenWireConversion() {
    uint8_t tx_data[2] = {(ADOW >> 8) & 0xFF, ADOW & 0xFF};
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    digitalWrite(_cs_pin, HIGH);
    
    delay(10);
    
    return true;
}

bool LTC6802::startSelfTest() {
    uint8_t tx_data[2] = {(CVST >> 8) & 0xFF, CVST & 0xFF};
    
    wakeup();
    digitalWrite(_cs_pin, LOW);
    
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    digitalWrite(_cs_pin, HIGH);
    
    delay(10);
    
    return true;
}

float LTC6802::rawVoltageToFloat(uint16_t raw_voltage) {
    // LTC6802: 16-bit ADC, 0-5V range
    return (raw_voltage * 5.0f) / 65535.0f;
}

uint16_t LTC6802::floatVoltageToRaw(float voltage) {
    return (uint16_t)((voltage * 65535.0f) / 5.0f);
}