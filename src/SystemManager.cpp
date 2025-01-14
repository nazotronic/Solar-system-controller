/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.0 beta
 * Date: 14.01.2025
 */

#include "data.h"

SystemManager::SystemManager() {
	makeDefault();
}

void SystemManager::begin() {
	Serial.begin(9600);
	
	time.setSystemManager(this);
	moduls.setSystemManager(this);
	solar.setSystemManager(this);
	display.setSystemManager(this);
	network.setSystemManager(this);
	blynk.setSystemManager(this);

	LittleFS.begin();
	time.begin();
	moduls.begin();
	solar.begin();
	display.begin();
	network.begin();  

	pinMode(BUZZER_PORT, OUTPUT);
	pinMode(SW_PORT, INPUT_PULLUP);
	enc.setType(TYPE2);

	attachInterrupt(CLK_PORT, encoderClkInterrupt, CHANGE);
	attachInterrupt(DT_PORT, encoderDtInterrupt, CHANGE);
	attachInterrupt(SW_PORT, encoderSwInterrupt, CHANGE);
	
	readSettings();
}


void SystemManager::tick() {
	// uint32_t tick = millis();

  	yield();
	enc.tick();
	
	if (enc.isTurn() || enc.isPress()) {
		if (display.action()) {
			enc.resetStates();
		}
	}

	time.tick();
	moduls.tick();
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


void SystemManager::makeBlynkElementsList(DynamicArray<blynk_element_t>* array) {
	if (array == NULL) {
		return;
	}

	array->clear();
	addBlynkElements(array);
	
#ifdef TIME_MANAGER_BLYNK_SUPPORT
	time.addBlynkElements(array);
#endif

#ifdef MODULE_MANAGER_BLYNK_SUPPORT
	moduls.addBlynkElements(array);
#endif

#ifdef SOLAR_SYSTEM_MANAGER_BLYNK_SUPPORT
	solar.addBlynkElements(array);
#endif

#ifdef DISPLAY_MANAGER_BLYNK_SUPPORT
	display.addBlynkElements(array);
#endif

#ifdef NETWORK_MANAGER_BLYNK_SUPPORT
	network.addBlynkElements(array);
#endif
}

int8_t SystemManager::scanBlynkElemetIndex(DynamicArray<blynk_element_t>* array, blynk_element_t* element) {
	if (array == NULL || element == NULL) {
		return -1;
	}

	for (uint8_t i = 0; i < array->getSize();i++) {
		if (!strcmp((*array)[i].code, element->code)) {
			return i;
		}
	}

	return -1;
}

int8_t SystemManager::scanBlynkElemetIndex(DynamicArray<blynk_element_t>* array, String element_code) {
	if (array == NULL) {
		return -1;
	}

	for (uint8_t i = 0; i < array->getSize();i++) {
		if (!strcmp((*array)[i].code, element_code.c_str())) {
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

ModuleManager* SystemManager::getModuleManager() {
	return &moduls;
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


void SystemManager::addBlynkElements(DynamicArray<blynk_element_t>* array) {
	if (array == NULL) {
		return;
	}

	array->add(blynk_element_t("P buzzer", "SSb", &buzzer_flag, BLYNK_TYPE_BOOL));
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
	moduls.writeSettings(buffer);
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

	time.readSettings(buffer);
	moduls.readSettings(buffer);
	solar.readSettings(buffer);
	display.readSettings(buffer);
	network.readSettings(buffer);
	blynk.readSettings(buffer);

	delete[] buffer;
	file.close();
}

Encoder SystemManager::enc = Encoder(CLK_PORT, DT_PORT, SW_PORT);