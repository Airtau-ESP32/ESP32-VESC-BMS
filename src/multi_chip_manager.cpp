#include "multi_chip_manager.h"

MultiChipManager::MultiChipManager() {
    _cs_pins = {7, 8, 9, 10}; // Default CS pins
}

void MultiChipManager::addChipCS(uint8_t cs_pin) {
    _cs_pins.push_back(cs_pin);
}

void MultiChipManager::setChipPins(const std::vector<uint8_t>& cs_pins) {
    _cs_pins = cs_pins;
}

void MultiChipManager::scanAllChips() {
    Serial.println("\n🔄 Scanning all BMS chips...");
    _chips.clear();
    
    for (uint8_t cs_pin : _cs_pins) {
        ChipDetector detector(cs_pin);
        if (detector.begin()) {
            ChipDetectionResult result = detector.detectChip();
            
            ChipInfo info;
            info.cs_pin = cs_pin;
            info.type = result.type;
            info.is_present = result.is_responding;
            info.status = result.status_register;
            info.last_communication = millis();
            info.error_message = result.error_message;
            
            _chips.push_back(info);
            
            if (result.is_responding) {
                Serial.printf("✅ CS %d: %s detected\n", cs_pin, detector.chipTypeToString(result.type).c_str());
            } else {
                Serial.printf("❌ CS %d: No chip detected\n", cs_pin);
            }
        } else {
            Serial.printf("❌ CS %d: Detector initialization failed\n", cs_pin);
        }
        delay(10);
    }
}

void MultiChipManager::detectChipTypes() {
    Serial.println("\n🔍 Detecting chip types...");
    
    for (auto& chip : _chips) {
        if (chip.is_present) {
            ChipDetector detector(chip.cs_pin);
            if (detector.begin()) {
                ChipDetectionResult result = detector.detectChipAdvanced();
                chip.type = result.type;
                chip.status = result.status_register;
                chip.error_message = result.error_message;
                
                Serial.printf("CS %d: %s (Status: 0x%02X)\n", 
                             chip.cs_pin, detector.chipTypeToString(chip.type).c_str(), chip.status);
            }
        }
        delay(5);
    }
}

bool MultiChipManager::initializeAllChips() {
    Serial.println("\n🚀 Initializing all detected chips...");
    bool all_success = true;
    
    for (auto& chip : _chips) {
        if (chip.is_present) {
            if (initializeChip(chip)) {
                Serial.printf("✅ CS %d: %s initialized\n", 
                             chip.cs_pin, chipTypeToString(chip.type));
            } else {
                Serial.printf("❌ CS %d: Initialization failed\n", chip.cs_pin);
                all_success = false;
            }
        }
        delay(5);
    }
    
    return all_success;
}

void MultiChipManager::initializeChip(ChipInfo& chip) {
    // Здесь будет код инициализации конкретного чипа
    // Пока просто обновляем временную метку
    chip.last_communication = millis();
}

bool MultiChipManager::communicateWithChip(ChipInfo& chip) {
    // Тестовая коммуникация с чипом
    ChipDetector detector(chip.cs_pin);
    if (detector.begin()) {
        bool success = detector.testCommunication();
        chip.last_communication = millis();
        chip.is_present = success;
        return success;
    }
    return false;
}

std::vector<ChipInfo> MultiChipManager::getChipInfo() const {
    return _chips;
}

ChipInfo MultiChipManager::getChipInfo(uint8_t cs_pin) const {
    for (const auto& chip : _chips) {
        if (chip.cs_pin == cs_pin) {
            return chip;
        }
    }
    return ChipInfo{cs_pin, CHIP_UNKNOWN, false, 0, 0, "Chip not scanned"};
}

size_t MultiChipManager::getDetectedChipCount() const {
    size_t count = 0;
    for (const auto& chip : _chips) {
        if (chip.is_present) count++;
    }
    return count;
}

size_t MultiChipManager::getTotalChipCount() const {
    return _chips.size();
}

void MultiChipManager::printChipSummary() const {
    Serial.println("\n📊 BMS Chip Summary");
    Serial.println("===================");
    
    size_t detected = getDetectedChipCount();
    size_t total = getTotalChipCount();
    
    Serial.printf("Total chips: %zu\n", total);
    Serial.printf("Detected chips: %zu\n", detected);
    Serial.printf("Success rate: %.1f%%\n", (float)detected / total * 100.0f);
    
    Serial.println("\nDetected chips:");
    for (const auto& chip : _chips) {
        if (chip.is_present) {
            Serial.printf("  CS %d: %s (Status: 0x%02X)\n", 
                         chip.cs_pin, chipTypeToString(chip.type), chip.status);
        }
    }
    
    Serial.println("\nMissing chips:");
    for (const auto& chip : _chips) {
        if (!chip.is_present) {
            Serial.printf("  CS %d: Not detected", chip.cs_pin);
            if (!chip.error_message.isEmpty()) {
                Serial.printf(" - %s", chip.error_message.c_str());
            }
            Serial.println();
        }
    }
}

std::vector<uint8_t> MultiChipManager::getWorkingChips() const {
    std::vector<uint8_t> working;
    for (const auto& chip : _chips) {
        if (chip.is_present) {
            working.push_back(chip.cs_pin);
        }
    }
    return working;
}

std::vector<uint8_t> MultiChipManager::getFaultyChips() const {
    std::vector<uint8_t> faulty;
    for (const auto& chip : _chips) {
        if (!chip.is_present) {
            faulty.push_back(chip.cs_pin);
        }
    }
    return faulty;
}

bool MultiChipManager::startConversionAll() {
    bool all_success = true;
    for (const auto& chip : _chips) {
        if (chip.is_present) {
            ChipDetector detector(chip.cs_pin);
            if (detector.begin()) {
                detector.sendCommand(0x0260); // ADCV
            } else {
                all_success = false;
            }
        }
        delay(1);
    }
    delay(15); // Wait for conversion
    return all_success;
}

bool MultiChipManager::readAllCellVoltages() {
    // Здесь будет код чтения напряжений со всех чипов
    // Пока заглушка
    return true;
}

String chipTypeToString(ChipType type) {
    switch (type) {
        case CHIP_LTC6802: return "LTC6802";
        case CHIP_LTC6803: return "LTC6803";
        case CHIP_LTC6804: return "LTC6804";
        default: return "Unknown";
    }
}