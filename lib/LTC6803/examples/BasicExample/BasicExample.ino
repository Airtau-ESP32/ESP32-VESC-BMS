#include <LTC6803.h>

LTC6803 bms(7); // CS pin 7

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("LTC6803 Basic Example");
    Serial.println("====================");
    
    if (bms.begin()) {
        Serial.println("LTC6803 initialized successfully");
    } else {
        Serial.println("Failed to initialize LTC6803");
        while(1);
    }
}

void loop() {
    static uint32_t last_read = 0;
    
    if (millis() - last_read >= 2000) {
        last_read = millis();
        
        // Запускаем преобразование напряжений ячеек
        if (bms.startCellVoltageADC()) {
            Serial.println("Cell voltage ADC conversion started");
        }
        
        delay(15); // Ждем завершения преобразования
        
        // Читаем напряжения ячеек
        uint16_t cell_voltages[12];
        if (bms.readCellVoltages(cell_voltages)) {
            Serial.println("Cell Voltages:");
            for (int i = 0; i < 12; i++) {
                float voltage = bms.rawCellVoltageToFloat(cell_voltages[i]);
                Serial.printf("  Cell %2d: %6.3fV", i + 1, voltage);
                if ((i + 1) % 3 == 0) Serial.println();
            }
            Serial.println();
        } else {
            Serial.println("Failed to read cell voltages");
        }
        
        // Читаем статус
        uint8_t status_data[6];
        if (bms.readStatus(status_data)) {
            Serial.printf("Status Register A: 0x%02X\n", status_data[0]);
            
            // Анализ статуса
            if (status_data[0] & 0x80) Serial.println("  - MUX fail detected");
            if (status_data[0] & 0x40) Serial.println("  - Thermal shutdown");
            if (status_data[0] & 0x20) Serial.println("  - ADC conversion in progress");
            if (status_data[0] & 0x10) Serial.println("  - Undervoltage fault");
            if (status_data[0] & 0x08) Serial.println("  - Overvoltage fault");
            if (status_data[0] & 0x04) Serial.println("  - Reference not ready");
            if (status_data[0] & 0x02) Serial.println("  - Cell voltage ADC busy");
            if (status_data[0] & 0x01) Serial.println("  - GPIO ADC busy");
        }
        
        Serial.println("----------------------------------------");
    }
    
    delay(100);
}