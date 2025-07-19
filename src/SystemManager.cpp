/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.1
 * Date: 04.02.2025
 */

#include "data.h"

SystemManager::SystemManager() {
	makeDefault();
}

void SystemManager::begin() {
	Serial.begin(9600);

	time.setSystemManager(this);
	sensors.setSystemManager(this);
	solar.setSystemManager(this);
	display.setSystemManager(this);
	network.setSystemManager(this);
	blynk.setSystemManager(this);

	LittleFS.begin();
	time.begin();
	sensors.begin();
	solar.begin();
	display.begin();
	network.begin();  

	pinMode(BUZZER_PORT, OUTPUT);
	enc.setEncPortMode(ENC_PORT_INPUT_PULLUP);
	enc.setButPortMode(BUTTON_PORT_INPUT_PULLUP);
	enc.setButInvert(true);

	attachInterrupt(CLK_PORT, encoderClkInterrupt, CHANGE);
	attachInterrupt(DT_PORT, encoderDtInterrupt, CHANGE);
	attachInterrupt(SW_PORT, encoderSwInterrupt, CHANGE);
	
	readSettings();
	network.endBegin();
}


void SystemManager::tick() {
	// uint32_t tick = millis();

  	yield();
	// if (enc.isLeft()) {
	// 	Serial.println(String(millis()) + " left");
	// }
	// if (enc.isRight()) {
	// 	Serial.println(String(millis()) + " right");
	// }
	
	if (enc.isTurn() || enc.isPressed()) {
		if (display.action()) {
			enc.deleteTurns();
			enc.clearButFlags();
		}
	}

	time.tick();
	sensors.tick();
	solar.tick();
	display.tick();
	network.tick();
	blynk.tick();

	saveSettings();
	// Serial.println(millis() - tick);
}

void SystemManager::makeDefault() {
	buzzer_flag = DEFAULT_BUZZER_FLAG;
	save_settings_request = false;
	save_settings_timer = 0;
}

void SystemManager::reset() {
	ESP.reset();
}

void SystemManager::resetAll() {
	LittleFS.remove("/config.nztr");

  	ESP.reset();
}


void SystemManager::makeBlynkElementCodesList(DynamicArray<String>* array) {
	if (array == NULL) {
		return;
	}
	array->clear();
	
#ifdef TIME_MANAGER_BLYNK_SUPPORT
	time.addBlynkElementCodes(array);
#endif

#ifdef MODULE_MANAGER_BLYNK_SUPPORT
	sensors.addBlynkElementCodes(array);
#endif

#ifdef SOLAR_SYSTEM_MANAGER_BLYNK_SUPPORT
	solar.addBlynkElementCodes(array);
#endif

#ifdef DISPLAY_MANAGER_BLYNK_SUPPORT
	display.addBlynkElementCodes(array);
#endif

#ifdef NETWORK_MANAGER_BLYNK_SUPPORT
	network.addBlynkElementCodes(array);
#endif
}

void SystemManager::makeBlynkElementSend(BlynkWifi* Blynk, blynk_link_t* link) {
	if (Blynk == NULL || link == NULL) {
		return;
	}

#ifdef TIME_MANAGER_BLYNK_SUPPORT
	if (time.blynkElementSend(Blynk, link)) return;
#endif

#ifdef MODULE_MANAGER_BLYNK_SUPPORT
	if (sensors.blynkElementSend(Blynk, link)) return;
#endif

#ifdef SOLAR_SYSTEM_MANAGER_BLYNK_SUPPORT
	if (solar.blynkElementSend(Blynk, link)) return;
#endif

#ifdef DISPLAY_MANAGER_BLYNK_SUPPORT
	if (display.blynkElementSend(Blynk, link)) return;
#endif

#ifdef NETWORK_MANAGER_BLYNK_SUPPORT
	if (network.blynkElementSend(Blynk, link)) return;
#endif
}

void SystemManager::makeBlynkElementParse(String element_code, const BlynkParam& param) {
#ifdef TIME_MANAGER_BLYNK_SUPPORT
	if (time.blynkElementParse(element_code, param)) return;
#endif

#ifdef MODULE_MANAGER_BLYNK_SUPPORT
	if (sensors.blynkElementParse(element_code, param)) return;
#endif

#ifdef SOLAR_SYSTEM_MANAGER_BLYNK_SUPPORT
	if (solar.blynkElementParse(element_code, param)) return;
#endif

#ifdef DISPLAY_MANAGER_BLYNK_SUPPORT
	if (display.blynkElementParse(element_code, param)) return;
#endif

#ifdef NETWORK_MANAGER_BLYNK_SUPPORT
	if (network.blynkElementParse(element_code, param)) return;
#endif
}

int8_t SystemManager::scanBlynkElemetCodeIndex(DynamicArray<String>* array, String element_code) {
	if (array == NULL) {
		return -1;
	}

	for (uint8_t i = 0; i < array->size();i++) {
		if (!strcmp((*array)[i].c_str(), element_code.c_str()) ) {
			return i;
		}
	}

	return -1;
}


bool SystemManager::deleteBlynkLink(String element_code) {
	return blynk.deleteLink(element_code);
}

bool SystemManager::modifyBlynkLinkElementCode(String previous_code, String new_code) {
	return blynk.modifyLinkElementCode(previous_code, new_code);
}


void SystemManager::saveSettingsRequest() {
	save_settings_request = true;
}

void SystemManager::buzzer(uint16_t freq, uint16_t duration) {
	if (!buzzer_flag) {
		return;
	}

	tone(BUZZER_PORT, freq, duration);
}


ICACHE_RAM_ATTR void SystemManager::encoderClkInterrupt() {
	enc.tick();
}

ICACHE_RAM_ATTR void SystemManager::encoderDtInterrupt() {
	enc.tick();
}

ICACHE_RAM_ATTR void SystemManager::encoderSwInterrupt() {
	enc.tick();
}


void SystemManager::setBuzzerFlag(bool buzzer_flag){
	this->buzzer_flag = buzzer_flag;
}


bool SystemManager::getBuzzerFlag() {
	return buzzer_flag;
}

TimeManager* SystemManager::getTimeManager() {
	return &time;
}

SensorsManager* SystemManager::getSensorsManager() {
	return &sensors;
}

SolarSystemManager* SystemManager::getSolarSystemManager() {
	return &solar;
}

DisplayManager* SystemManager::getDisplayManager() {
	return &display;
}

NetworkManager* SystemManager::getNetworkManager() {
	return &network;
}

BlynkManager* SystemManager::getBlynkManager() {
	return &blynk;
}

Encoder* SystemManager::getEncoder() {
	return &enc;
}


void SystemManager::saveSettings(bool ignore_flag) {
	if (!ignore_flag) {
		if (!save_settings_request) {
			return;
		}

		if (millis() - save_settings_timer < SEC_TO_MLS(SAVE_SETTINGS_TIME)) {
			return;
		}
	}
	Serial.println("save");

	File file = LittleFS.open("/config.nztr", "w");
	char buffer[SETTINGS_BUFFER_SIZE + 1] = "";

	setParameter(buffer, "SSb", getBuzzerFlag());

	time.writeSettings(buffer);
	sensors.writeSettings(buffer);
	solar.writeSettings(buffer);
	display.writeSettings(buffer);
	network.writeSettings(buffer);
	blynk.writeSettings(buffer);

	file.write(buffer, strlen(buffer));
  	file.close();

	save_settings_request = false;
	save_settings_timer = millis();
}

void SystemManager::readSettings() {
	File file = LittleFS.open("/config.nztr", "r");

	if (!file) {
		saveSettings(true);
		return;
	}

	uint16_t file_size = file.size();
	char* buffer = new char[file_size + 1];

	file.read((uint8_t*) buffer, file_size);
	buffer[file_size] = 0;
	
	getParameter(buffer, "SSb", &buzzer_flag);
	setBuzzerFlag(buzzer_flag);

	time.readSettings(buffer);
	sensors.readSettings(buffer);
	solar.readSettings(buffer);
	display.readSettings(buffer);
	network.readSettings(buffer);
	blynk.readSettings(buffer);

	delete[] buffer;
	file.close();
}

Encoder SystemManager::enc = Encoder(CLK_PORT, DT_PORT, SW_PORT);