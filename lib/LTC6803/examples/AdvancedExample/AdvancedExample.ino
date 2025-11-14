#include <LTC6803.h>

LTC6803 bms(7);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("LTC6803 Advanced Example");
    Serial.println("=======================");
    
    if (!bms.begin()) {
        Serial.println("Failed to initialize LTC6803");
        while(1);
    }
    
    Serial.println("LTC6803 ready");
}

void loop() {
    static bool discharge_enabled = false;
    static uint32_t last_operation = 0;
    
    if (millis() - last_operation >= 5000) {
        last_operation = millis();
        
        // Чтение всех данных
        readAllData();
        
        // Переключаем балансировку каждые 30 секунд
        static uint32_t last_discharge_toggle = 0;
        if (millis() - last_discharge_toggle >= 30000) {
            last_discharge_toggle = millis();
            toggleDischarge(discharge_enabled);
            discharge_enabled = !discharge_enabled;
        }
    }
    
    delay(100);
}

void readAllData() {
    // Напряжения ячеек
    bms.startCellVoltageADC();
    delay(15);
    
    uint16_t cell_voltages[12];
    if (bms.readCellVoltages(cell_voltages)) {
        Serial.println("=== Cell Voltages ===");
        float total_voltage = 0;
        float min_voltage = 5.0, max_voltage = 0;
        
        for (int i = 0; i < 12; i++) {
            float voltage = bms.rawCellVoltageToFloat(cell_voltages[i]);
            total_voltage += voltage;
            min_voltage = min(min_voltage, voltage);
            max_voltage = max(max_voltage, voltage);
            
            Serial.printf("C%d: %.3fV", i + 1, voltage);
            if ((i + 1) % 4 == 0) Serial.println();
            else Serial.print(" | ");
        }
        
        Serial.printf("\nTotal: %.2fV, Min: %.3fV, Max: %.3fV\n", 
                     total_voltage, min_voltage, max_voltage);
    }
    
    // Вспомогательные измерения
    bms.startAuxiliaryADC();
    delay(15);
    
    uint16_t aux_data[6];
    if (bms.readAuxiliary(aux_data)) {
        Serial.println("=== Auxiliary Measurements ===");
        for (int i = 0; i < 5; i++) { // GPIO1-GPIO5
            float voltage = bms.rawAuxVoltageToFloat(aux_data[i]);
            Serial.printf("GPIO%d: %.3fV", i + 1, voltage);
            if (i < 4) Serial.print(" | ");
        }
        Serial.println();
    }
    
    // Статус
    uint8_t status_data[6];
    if (bms.readStatus(status_data)) {
        Serial.println("=== Status ===");
        Serial.printf("Status A: 0x%02X, Status B: 0x%02X\n", 
                     status_data[0], status_data[1]);
    }
    
    Serial.println();
}

void toggleDischarge(bool enable) {
    if (enable) {
        // Включаем балансировку для ячеек с напряжением > 4.1V
        bms.startCellVoltageADC();
        delay(15);
        
        uint16_t cell_voltages[12];
        if (bms.readCellVoltages(cell_voltages)) {
            uint16_t discharge_mask = 0;
            
            for (int i = 0; i < 12; i++) {
                float voltage = bms.rawCellVoltageToFloat(cell_voltages[i]);
                if (voltage > 4.1f) {
                    discharge_mask |= (1 << i);
                    Serial.printf("Discharging cell %d (%.3fV)\n", i + 1, voltage);
                }
            }
            
            bms.setDischarge(discharge_mask);
            Serial.println("Discharge enabled for high voltage cells");
        }
    } else {
        bms.clearDischarge();
        Serial.println("Discharge disabled");
    }
}