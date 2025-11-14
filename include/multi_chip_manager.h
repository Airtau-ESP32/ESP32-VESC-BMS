#ifndef MULTI_CHIP_MANAGER_H
#define MULTI_CHIP_MANAGER_H

#include "chip_detector.h"
#include "globals.h"
#include <vector>

struct ChipInfo {
    uint8_t cs_pin;
    ChipType type;
    bool is_present;
    uint8_t status;
    uint32_t last_communication;
    String error_message;
};

class MultiChipManager {
private:
    std::vector<ChipInfo> _chips;
    std::vector<uint8_t> _cs_pins;
    
    void initializeChip(ChipInfo& chip);
    bool communicateWithChip(ChipInfo& chip);
    
public:
    MultiChipManager();
    
    void addChipCS(uint8_t cs_pin);
    void setChipPins(const std::vector<uint8_t>& cs_pins);
    void scanAllChips();
    void detectChipTypes();
    bool initializeAllChips();
    
    // Получение информации
    std::vector<ChipInfo> getChipInfo() const;
    ChipInfo getChipInfo(uint8_t cs_pin) const;
    size_t getDetectedChipCount() const;
    size_t getTotalChipCount() const;
    
    // Статистика
    void printChipSummary() const;
    std::vector<uint8_t> getWorkingChips() const;
    std::vector<uint8_t> getFaultyChips() const;
    
    // Управление
    bool startConversionAll();
    bool readAllCellVoltages();
};

#endif