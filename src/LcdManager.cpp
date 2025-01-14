/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.0 beta
 * Date: 14.01.2025
 */

#include "data.h"

LcdManager::LcdManager() : LiquidCrystal_I2C(0x27, 20, 4) {

}


void LcdManager::printTitle(uint8_t y, String title, uint16_t delay_time) {
	uint8_t x = 10 - title.length() / 2;

	clear();
	easyPrint(x, y, title);
	
	delay(delay_time);
	clear();
}

void LcdManager::easyPrint(uint8_t x, uint8_t y, String array) {
	setCursor(x, y);
	print(array);
}
void LcdManager::easyPrint(uint8_t x, uint8_t y, int32_t number) {
	setCursor(x, y);
	print(number);
}
void LcdManager::easyPrint(uint8_t x, uint8_t y, float number) {
	setCursor(x, y);
	print(number, 2);
}

void LcdManager::easyWrite(uint8_t x, uint8_t y, uint8_t code) {
	setCursor(x, y);
	write(code);
}

void LcdManager::clearLine(uint8_t line) {
	for (uint8_t i = 0;i < 20;i++) {
		easyPrint(i, line, " ");
	}
}

void LcdManager::clearColumn(uint8_t column) {
	for (uint8_t i = 0;i < 4;i++) {
		easyPrint(column, i, " ");
	}
}