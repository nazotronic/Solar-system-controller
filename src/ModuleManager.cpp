/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.0 beta
 * Date: 14.01.2025
 */

#include "data.h"

ModuleManager::ModuleManager() {
	makeDefault();
}

void ModuleManager::begin() {
	oneWire.begin(DS18B20_PORT);
	ds18b20_sensor.setOneWire(&oneWire);
	
	ds18b20_sensor.begin();
	ds18b20_sensor.setResolution(12);
}


void ModuleManager::tick() {
	if (getReadDataTime()) {
		if (!read_data_timer || millis() - read_data_timer >= SEC_TO_MLS(getReadDataTime())) {
			read_data_timer = millis();
			updateModuleData();
		}
	}
}

void ModuleManager::makeDefault() {
	memset(&am2320_data, 0, sizeof(am2320_data_t));
	ds18b20_data.~DynamicArray();
	ds18b20_data.setMaxSize(DS_SENSORS_MAX_COUNT);

	system = NULL;
	am2320_data.status = UNSPECIFIED_STATUS;

	read_data_time = DEFAULT_READ_DATA_TIME;
	read_data_timer = 0;
}

void ModuleManager::writeSettings(char* buffer) {
	setParameter(buffer, "SMrdt", getReadDataTime());

	for (uint8_t i = 0;i < getDS18B20Count();i++) {
		setParameter(buffer, String("SMDSn") + i, (const char*) getDS18B20Name(i));
		setParameter(buffer, String("SMDSa") + i, getDS18B20Address(i), 8);
		setParameter(buffer, String("SMDSc") + i, getDS18B20Correction(i));
  	}
}

void ModuleManager::readSettings(char* buffer) {
	uint8_t ds18b20_index = 0;
	
	char ds18b20_name[DS_NAME_SIZE];
	uint8_t ds18b20_address[8];
	float ds18b20_correction;

	getParameter(buffer, "SMrdt", &read_data_time);
	
	while (getParameter(buffer, String("SMDSn") + ds18b20_index, ds18b20_name, DS_NAME_SIZE)) {
		if (addDS18B20()) {
			setDS18B20Name(ds18b20_index, ds18b20_name);

			if (getParameter(buffer, String("SMDSa") + ds18b20_index, ds18b20_address, 8)) {
				setDS18B20Address(ds18b20_index, ds18b20_address);
			}

			if (getParameter(buffer, String("SMDSc") + ds18b20_index, &ds18b20_correction)) {
				setDS18B20Correction(ds18b20_index, ds18b20_correction);
			}
		}

		ds18b20_index++;
	}
}

#ifdef MODULE_MANAGER_BLYNK_SUPPORT
void ModuleManager::addBlynkElements(DynamicArray<blynk_element_t>* array) {
	if (array == NULL) {
		return;
	}

	array->add(blynk_element_t("T home", "HSt", &am2320_data.t, BLYNK_TYPE_FLOAT));
	array->add(blynk_element_t("H home", "HSh", &am2320_data.h, BLYNK_TYPE_FLOAT));

	for (uint8_t i = 0; i < ds18b20_data.getSize();i++) {
		array->add(blynk_element_t(ds18b20_data[i].name, String("HSdst") + i, &ds18b20_data[i].t, BLYNK_TYPE_FLOAT));
	}

	array->add(blynk_element_t("P data time", "SMrdt", &read_data_time, BLYNK_TYPE_UINT8_T));
}
#endif


bool ModuleManager::addDS18B20() {
	if (ds18b20_data.add()) {
		strcpy(ds18b20_data[ds18b20_data.getSize() - 1].name, DEFAULT_DS18B20_NAME);
		ds18b20_data[ds18b20_data.getSize() - 1].status = UNSPECIFIED_STATUS;

		return true;
	}

	return false;
}

bool ModuleManager::deleteDS18B20(uint8_t index) {
#ifdef MODULE_MANAGER_BLYNK_SUPPORT
	system->deleteBlynkLink(String("HSdst") + index);

	if (index != ds18b20_data.getSize()) {
		for (uint8_t i = index;i < ds18b20_data.getSize();i++) {
			system->modifyBlynkLinkElementCode(String("HSdst") + i, String("HSdst") + (i - 1));
		}
	}
#endif

	if (ds18b20_data.del(index)) {
		return true;
	}

	return false;
}

uint8_t ModuleManager::scanDS18B20Count() {
	ds18b20_sensor.begin();
	return ds18b20_sensor.getDS18Count();
}

float ModuleManager::scanDS18B20TByAddress(uint8_t* address) {
	ds18b20_sensor.requestTemperaturesByAddress(address);
	return ds18b20_sensor.getTempC(address);
}


uint8_t ModuleManager::makeDS18B20AddressList(String* string_pointer) {
	if (string_pointer == NULL) {
		return 0;
	}

	uint8_t sensors_count = scanDS18B20Count();
	if (!sensors_count) {
		return 0;
	}

	for (uint8_t i = 0;i < sensors_count;i++) {
		uint8_t address[8];
		String string_address;

		ds18b20_sensor.getAddress(address, i);
		DS18B20AddressToString(address, &string_address);
		
		*string_pointer += string_address;

		if (i != sensors_count - 1) {
			*string_pointer += ",";
		}
	}

	return sensors_count;
}
uint8_t ModuleManager::makeDS18B20AddressList(DynamicArray<uint8_t[8]>* array, DynamicArray<float>* t_array) {
	if (array == NULL) {
		return 0;
	}

	uint8_t sensors_count = scanDS18B20Count();
	array->clear();

	if (t_array != NULL) {
		t_array->clear();
	}

	if (!sensors_count) {
		return 0;
	}

	for (uint8_t i = 0;i < sensors_count;i++) {
		uint8_t address[8];

		ds18b20_sensor.getAddress(address, i);
		array->add(&address); // &address - > uint8_t (*)[8] | address - > &address[0]

		if (t_array != NULL) {
			t_array->add(scanDS18B20TByAddress(address));
		}
	}

	return sensors_count;
}

int8_t ModuleManager::scanDS18B20AddressIndex(DynamicArray<uint8_t[8]>* array, uint8_t* address) {
	if (array == NULL || address == NULL) {
		return -1;
	}

	for (uint8_t i = 0; i < array->getSize();i++) {
		if (!memcmp((*array)[i], address, 8)) {
			return i;
		}
	}

	return -1;
}


void ModuleManager::setSystemManager(SystemManager* system) {
	this->system = system;
}

void ModuleManager::setReadDataTime(uint8_t time) {
	read_data_time = constrain(time, 0, 100);
}


void ModuleManager::setDS18B20(uint8_t index, ds18b20_data_t* ds18b20) {
	setDS18B20Name(index, ds18b20->name);
	setDS18B20Address(index, ds18b20->address);
	setDS18B20Correction(index, ds18b20->correction);
}

void ModuleManager::setDS18B20Name(uint8_t index, String name) {
	if (!isCorrectIndex(index)) {
		return;
	}
	
	name.toCharArray(ds18b20_data[index].name, 3);
}

void ModuleManager::setDS18B20Address(uint8_t index, uint8_t* address) {
	if (!isCorrectIndex(index)) {
		return;
	}

	if (!*address) {
		return;
	}

	memcpy(ds18b20_data[index].address, address, 8);
}

void ModuleManager::setDS18B20Correction(uint8_t index, float correction) {
	if (!isCorrectIndex(index)) {
		return;
	}

	ds18b20_data[index].correction = constrain(correction, -20.0, 20.0);
}


DallasTemperature* ModuleManager::getDallasTemperature() {
	return &ds18b20_sensor;
}

uint8_t ModuleManager::getReadDataTime() {
	return read_data_time;
}


float ModuleManager::getAM2320T() {
	return am2320_data.t;
}

float ModuleManager::getAM2320H() {
	return am2320_data.h;
}

uint8_t ModuleManager::getAM2320Status() {
	return am2320_data.status;
}


uint8_t ModuleManager::getDS18B20Count() {
	return ds18b20_data.getSize();
}

ds18b20_data_t* ModuleManager::getDS18B20(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return NULL;
	}

	return &ds18b20_data[index];
}

char* ModuleManager::getDS18B20Name(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return NULL;
	}

	return ds18b20_data[index].name;
}

uint8_t* ModuleManager::getDS18B20Address(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return NULL;
	}

	return ds18b20_data[index].address;
}

float ModuleManager::getDS18B20Correction(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return 0;
	}

	return ds18b20_data[index].correction;
}

float ModuleManager::getDS18B20T(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return 0;
	}

	return ds18b20_data[index].t + getDS18B20Correction(index);
}

uint8_t ModuleManager::getDS18B20Status(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return UNSPECIFIED_STATUS;
	}

	return ds18b20_data[index].status;
}


bool ModuleManager::isCorrectIndex(uint8_t index) {
	if (index >= getDS18B20Count()) {
		return false;
	}

	return true;
}

void ModuleManager::updateModuleData() {
	am2320_data.status = am2320_sensor.read(&am2320_data.t, &am2320_data.h);
	ds18b20_sensor.requestTemperatures();

	for (uint8_t i = 0;i < getDS18B20Count();i++) {
		ds18b20_data[i].t = ds18b20_sensor.getTempC(getDS18B20Address(i));

		if (getDS18B20T(i) < -100) {
			ds18b20_data[i].status = 1;
		}
		else if (getDS18B20T(i) == 85) {
			ds18b20_data[i].status = 2;
		}
		else {
			ds18b20_data[i].status = 0;
		}
	}
}

void ModuleManager::DS18B20AddressToString(uint8_t* address, String* string) {
	if (address == NULL || string == NULL) {
		return;
	}

	for (uint8_t i = 0;i < 8;i++) {
		*string += String(address[i], HEX);

		if (i != 7) {
			*string += "-";
		}
	}
}