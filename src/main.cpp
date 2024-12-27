/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.2.0
 * Date: 27.12.2024
 */

#include "data.h"

SystemManager systemManager;
// LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
	systemManager.begin();
}

void loop() {
	systemManager.tick();
}