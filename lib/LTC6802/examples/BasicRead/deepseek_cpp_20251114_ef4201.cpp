#include <LTC6802.h>

LTC6802 bms(7); // CS pin 7

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("LTC6802 Basic Read Example");
    
    if (bms.begin()) {
        Serial.println("LTC6802 initialized successfully");
    } else {
        Serial.println("Failed to initialize LTC6802");
        while(1);
    }
}

void loop() {
    // Запускаем преобразование напряжений
    if (bms.startCellVoltageConversion()) {
        Serial.println("Cell voltage conversion started");
    }
    
    delay(15); // Ждем завершения преобразования
    
    // Читаем напряжения
    uint8_t cell_data[18];
    if (bms.readCellVoltages(cell_data)) {
        Serial.println("Cell voltages read successfully");
        
        // Парсим данные ячеек
        for (int cell = 0; cell < 12; cell++) {
            uint16_t raw_voltage;
            int reg_index = cell / 2;
            int byte_offset = reg_index * 3;
            
            if (cell % 2 == 0) {
                // Четная ячейка
                raw_voltage = (cell_data[byte_offset] << 4) | (cell_data[byte_offset + 1] >> 4);
            } else {
                // Нечетная ячейка
                raw_voltage = ((cell_data[byte_offset + 1] & 0x0F) << 8) | cell_data[byte_offset + 2];
            }
            
            float voltage = bms.rawVoltageToFloat(raw_voltage);
            Serial.printf("Cell %d: %.3fV\n", cell + 1, voltage);
        }
    } else {
        Serial.println("Failed to read cell voltages");
    }
    
    // Читаем статус
    uint8_t status_data[6];
    if (bms.readStatus(status_data)) {
        Serial.printf("Status: 0x%02X\n", status_data[0]);
        
        if (status_data[0] & 0x08) Serial.println("MUX failure");
        if (status_data[0] & 0x04) Serial.println("Thermal shutdown");
        if (status_data[0] & 0x02) Serial.println("ADC error");
    }
    
    Serial.println("---");
    delay(5000);
}