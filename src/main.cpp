/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.0 beta
 * Date: 14.01.2025
 */

#include "data.h"

SystemManager systemManager;

void setup() {
	systemManager.begin();
}

void loop() {
	systemManager.tick();
}