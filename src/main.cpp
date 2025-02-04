/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.1
 * Date: 04.02.2025
 */

#include "data.h"

SystemManager systemManager;

void setup() {
	systemManager.begin();
}

void loop() {
	systemManager.tick();
}