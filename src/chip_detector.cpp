#include "chip_detector.h"

ChipDetector::ChipDetector(uint8_t cs_pin) {
    _cs_pin = cs_pin;
    _spi = new SPIClass(HSPI);
}

ChipDetector::~ChipDetector() {
    if (_spi) {
        delete _spi;
    }
}

bool ChipDetector::begin() {
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
    
    _spi->begin(4, 5, 6); // SCK, MISO, MOSI для ESP32-C3
    _spi->setFrequency(1000000); // 1 MHz
    _spi->setDataMode(SPI_MODE3);
    _spi->setBitOrder(MSBFIRST);
    
    return true;
}

void ChipDetector::wakeupChip() {
    digitalWrite(_cs_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(_cs_pin, HIGH);
    delayMicroseconds(300);
}

uint16_t ChipDetector::calculatePEC14(uint8_t* data, int len) {
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

bool ChipDetector::validatePEC(uint8_t* data, int len, uint16_t received_pec) {
    uint16_t calculated_pec = calculatePEC14(data, len);
    return (received_pec == calculated_pec);
}

bool ChipDetector::sendCommand(uint16_t command) {
    uint8_t cmd[2] = {(uint8_t)(command >> 8), (uint8_t)(command & 0xFF)};
    
    wakeupChip();
    digitalWrite(_cs_pin, LOW);
    _spi->transfer(cmd[0]);
    _spi->transfer(cmd[1]);
    digitalWrite(_cs_pin, HIGH);
    
    return true;
}

bool ChipDetector::readRegister(uint16_t command, uint8_t* data, uint8_t len, uint16_t* pec_error) {
    uint8_t tx_data[2] = {(uint8_t)(command >> 8), (uint8_t)(command & 0xFF)};
    uint8_t buffer[32];
    
    wakeupChip();
    digitalWrite(_cs_pin, LOW);
    
    // Send command
    _spi->transfer(tx_data[0]);
    _spi->transfer(tx_data[1]);
    
    // Read data + PEC
    for (int i = 0; i < len + 2; i++) {
        buffer[i] = _spi->transfer(0x00);
    }
    
    digitalWrite(_cs_pin, HIGH);
    
    // Check PEC
    uint16_t received_pec = (buffer[len] << 8) | buffer[len + 1];
    if (!validatePEC(buffer, len, received_pec)) {
        if (pec_error) *pec_error = received_pec;
        return false;
    }
    
    memcpy(data, buffer, len);
    return true;
}

ChipDetectionResult ChipDetector::testLTC6802() {
    ChipDetectionResult result;
    result.type = CHIP_LTC6802;
    result.is_responding = false;
    
    Serial.printf("🧪 Testing LTC6802 on CS pin %d...\n", _cs_pin);
    
    // Try to write configuration
    uint8_t config[6] = {0xF0, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t tx_data[8];
    
    tx_data[0] = 0x00;
    tx_data[1] = 0x01; // WRCFG
    
    for (int i = 0; i < 6; i++) {
        tx_data[i + 2] = config[i];
    }
    
    wakeupChip();
    digitalWrite(_cs_pin, LOW);
    for (int i = 0; i < 8; i++) {
        _spi->transfer(tx_data[i]);
    }
    digitalWrite(_cs_pin, HIGH);
    
    delay(1);
    
    // Try to read configuration back
    uint8_t rx_data[8];
    uint16_t pec_error;
    
    if (readRegister(0x0002, rx_data, 6, &pec_error)) {
        result.is_responding = true;
        result.status_register = rx_data[0];
        Serial.printf("✅ LTC6802 detected on CS %d, Status: 0x%02X\n", _cs_pin, rx_data[0]);
    } else {
        result.error_message = "PEC error: 0x" + String(pec_error, HEX);
        Serial.printf("❌ LTC6802 not responding on CS %d (PEC: 0x%04X)\n", _cs_pin, pec_error);
    }
    
    return result;
}

ChipDetectionResult ChipDetector::testLTC6803() {
    ChipDetectionResult result;
    result.type = CHIP_LTC6803;
    result.is_responding = false;
    
    Serial.printf("🧪 Testing LTC6803 on CS pin %d...\n", _cs_pin);
    
    // Try to write configuration
    uint8_t config[6] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00}; // REFON=1
    uint8_t tx_data[8];
    
    tx_data[0] = 0x00;
    tx_data[1] = 0x01; // WRCFG
    
    for (int i = 0; i < 6; i++) {
        tx_data[i + 2] = config[i];
    }
    
    wakeupChip();
    digitalWrite(_cs_pin, LOW);
    for (int i = 0; i < 8; i++) {
        _spi->transfer(tx_data[i]);
    }
    digitalWrite(_cs_pin, HIGH);
    
    delay(1);
    
    // Try to read configuration back
    uint8_t rx_data[8];
    uint16_t pec_error;
    
    if (readRegister(0x0002, rx_data, 6, &pec_error)) {
        result.is_responding = true;
        result.status_register = rx_data[0];
        Serial.printf("✅ LTC6803 detected on CS %d, Status: 0x%02X\n", _cs_pin, rx_data[0]);
        
        // Additional LTC6803 specific test - read status register group A
        uint8_t status_data[6];
        if (readRegister(0x0010, status_data, 6)) {
            Serial.printf("   Status Group A: 0x%02X\n", status_data[0]);
        }
    } else {
        result.error_message = "PEC error: 0x" + String(pec_error, HEX);
        Serial.printf("❌ LTC6803 not responding on CS %d (PEC: 0x%04X)\n", _cs_pin, pec_error);
    }
    
    return result;
}

ChipDetectionResult ChipDetector::detectChip() {
    ChipDetectionResult result;
    result.type = CHIP_UNKNOWN;
    result.is_responding = false;
    
    Serial.printf("\n🔍 Detecting chip on CS pin %d...\n", _cs_pin);
    
    // First try LTC6803 (more specific commands)
    ChipDetectionResult ltc6803_result = testLTC6803();
    if (ltc6803_result.is_responding) {
        return ltc6803_result;
    }
    
    // Then try LTC6802
    ChipDetectionResult ltc6802_result = testLTC6802();
    if (ltc6802_result.is_responding) {
        return ltc6802_result;
    }
    
    // If both fail, try advanced detection
    return detectChipAdvanced();
}

ChipDetectionResult ChipDetector::detectChipAdvanced() {
    ChipDetectionResult result;
    result.type = CHIP_UNKNOWN;
    result.is_responding = false;
    
    Serial.printf("🔧 Advanced detection on CS pin %d...\n", _cs_pin);
    
    // Test 1: Try to start voltage conversion
    sendCommand(0x0260); // ADCV command (common for both)
    delay(15);
    
    // Test 2: Try to read cell voltages in different formats
    uint8_t test_data[18];
    uint16_t pec_error;
    
    // Try LTC6802 format (18 bytes)
    if (readRegister(0x0004, test_data, 18, &pec_error)) {
        result.type = CHIP_LTC6802;
        result.is_responding = true;
        Serial.printf("✅ LTC6802 detected (via cell read) on CS %d\n", _cs_pin);
        return result;
    }
    
    // Try LTC6803 group format (6 bytes per group)
    if (readRegister(0x0004, test_data, 6, &pec_error)) {
        result.type = CHIP_LTC6803;
        result.is_responding = true;
        Serial.printf("✅ LTC6803 detected (via group read) on CS %d\n", _cs_pin);
        return result;
    }
    
    // Test 3: Try different register groups
    if (readRegister(0x000C, test_data, 6, &pec_error)) { // RDAUXA
        result.type = CHIP_LTC6803;
        result.is_responding = true;
        Serial.printf("✅ LTC6803 detected (via aux read) on CS %d\n", _cs_pin);
        return result;
    }
    
    result.error_message = "No response from any known chip type";
    Serial.printf("❌ No chip detected on CS %d\n", _cs_pin);
    return result;
}

bool ChipDetector::testCommunication() {
    Serial.printf("📡 Testing communication on CS pin %d...\n", _cs_pin);
    
    ChipDetectionResult result = detectChip();
    if (result.is_responding) {
        Serial.printf("✅ Communication OK - %s\n", chipTypeToString(result.type).c_str());
        return true;
    } else {
        Serial.printf("❌ Communication FAILED\n");
        return false;
    }
}

void ChipDetector::scanMultipleChips(uint8_t start_cs, uint8_t end_cs) {
    Serial.println("\n🔄 Scanning for BMS chips...");
    Serial.println("================================");
    
    for (uint8_t cs_pin = start_cs; cs_pin <= end_cs; cs_pin++) {
        // Temporarily change CS pin
        uint8_t original_cs = _cs_pin;
        _cs_pin = cs_pin;
        pinMode(_cs_pin, OUTPUT);
        digitalWrite(_cs_pin, HIGH);
        
        // Detect chip
        ChipDetectionResult result = detectChip();
        
        if (result.is_responding) {
            Serial.printf("CS %2d: ✅ %s (Status: 0x%02X)\n", 
                         cs_pin, chipTypeToString(result.type).c_str(), result.status_register);
        } else {
            Serial.printf("CS %2d: ❌ No chip detected\n", cs_pin);
        }
        
        // Restore original CS pin
        _cs_pin = original_cs;
        delay(10);
    }
    
    Serial.println("================================");
}

String ChipDetector::chipTypeToString(ChipType type) {
    switch (type) {
        case CHIP_LTC6802: return "LTC6802";
        case CHIP_LTC6803: return "LTC6803";
        case CHIP_LTC6804: return "LTC6804";
        default: return "Unknown";
    }
}