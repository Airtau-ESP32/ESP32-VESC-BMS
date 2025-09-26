#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>


// Настройка CAN шины (при необходимости измените контакты и скорость)
#define CAN_RX_PIN GPIO_NUM_8 
#define CAN_TX_PIN GPIO_NUM_9
#define CAN_SPEED 500 // kbps

void initCAN() {
	// устанавливаем пины кан шины
    ESP32Can.setPins(CAN_TX_PIN, CAN_RX_PIN);
	
    // Вы можете установить собственный размер очередей — они используются по умолчанию
    ESP32Can.setRxQueueSize(5);
	ESP32Can.setTxQueueSize(5);

    // Инициализируем CAN шину
    if(ESP32Can.begin()) {
        Serial.println("CAN bus started!");
    } else {
        Serial.println("CAN bus failed!");
    }
    
}