/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.0
 * Date: 25.01.2025
 */

#include "data.h"

SensorsManager::SensorsManager() {
	makeDefault();
}

void SensorsManager::begin() {
	oneWire.begin(DS18B20_PORT);
	ds18b20_sensor.setOneWire(&oneWire);
	
	ds18b20_sensor.begin();
	ds18b20_sensor.setResolution(12);
}


void SensorsManager::tick() {
	if (getReadDataTime()) {
		if (!read_data_timer || millis() - read_data_timer >= SEC_TO_MLS(getReadDataTime())) {
			read_data_timer = millis();
			updateSensorsData();
		}
	}
}

void SensorsManager::makeDefault() {
	memset(&am2320_data, 0, sizeof(am2320_data_t));
	ds18b20_data.clear();
	ds18b20_data.setMaxSize(DS_SENSORS_MAX_COUNT);

	system = NULL;
	am2320_data.status = UNSPECIFIED_STATUS;

	read_data_time = DEFAULT_READ_DATA_TIME;
	read_data_timer = 0;
}

void SensorsManager::writeSettings(char* buffer) {
	setParameter(buffer, "SSrdt", getReadDataTime());

	for (uint8_t i = 0;i < getDS18B20Count();i++) {
		setParameter(buffer, String("SSDSn") + i, (const char*) getDS18B20Name(i));
		setParameter(buffer, String("SSDSa") + i, getDS18B20Address(i), 8);
		setParameter(buffer, String("SSDSr") + i, getDS18B20Resolution(i));
		setParameter(buffer, String("SSDSc") + i, getDS18B20Correction(i));
  	}
}

void SensorsManager::readSettings(char* buffer) {
	uint8_t ds18b20_index = 0;
	char ds18b20_name[DS_NAME_SIZE];

	getParameter(buffer, "SSrdt", &read_data_time);
	
	while (getParameter(buffer, String("SSDSn") + ds18b20_index, ds18b20_name, DS_NAME_SIZE)) {
		if (addDS18B20()) {
			uint8_t ds18b20_address[8];
			uint8_t ds18b20_resolution;
			float ds18b20_correction;
			
			setDS18B20Name(ds18b20_index, ds18b20_name);

			if (getParameter(buffer, String("SSDSa") + ds18b20_index, ds18b20_address, 8)) {
				setDS18B20Address(ds18b20_index, ds18b20_address);
			}
			
			if (getParameter(buffer, String("SSDSr") + ds18b20_index, &ds18b20_resolution)) {
				setDS18B20Resolution(ds18b20_index, ds18b20_resolution);
			}

			if (getParameter(buffer, String("SSDSc") + ds18b20_index, &ds18b20_correction)) {
				setDS18B20Correction(ds18b20_index, ds18b20_correction);
			}
		}

		ds18b20_index++;
	}

	setReadDataTime(read_data_time);
}

#ifdef MODULE_MANAGER_BLYNK_SUPPORT
void SensorsManager::addBlynkElementCodes(DynamicArray<String>* array) {
	if (array == NULL) {
		return;
	}

	array->add(String("HSt"));
	array->add(String("HSh"));

	for (uint8_t i = 0; i < ds18b20_data.size();i++) {
		array->add(String("HSdst") + getDS18B20Name(i));
	}
}

bool SensorsManager::blynkElementSend(BlynkWifi* Blynk, blynk_link_t* link) {
	if (Blynk == NULL || link == NULL) {
		return false;
	}

	if (!strcmp(link->element_code, "HSt")) {
		Blynk->virtualWrite(link->port, getAM2320T());
		return true;
	}
	
	if (!strcmp(link->element_code, "HSh")) {
		Blynk->virtualWrite(link->port, getAM2320H());
		return true;
	}

	for (uint8_t i = 0;i < getDS18B20Count();i++) {
		if (!getDS18B20Status(i)) {
			char element_code[BLYNK_ELEMENT_CODE_SIZE] = "HSdst";
			strcat(element_code, getDS18B20Name(i));

			if (!strcmp(link->element_code, element_code)) {
				Blynk->virtualWrite(link->port, getDS18B20T(i));
				return true;
			}
		}
	}

	return false;
}

bool SensorsManager::blynkElementParse(String code, const BlynkParam& param) {
	return false;
}
#endif


bool SensorsManager::addDS18B20() {
	if (ds18b20_data.add()) {
		setDS18B20Name(ds18b20_data.size() - 1, DEFAULT_DS18B20_NAME);
		setDS18B20Resolution(ds18b20_data.size() - 1, DEFAULT_DS18B20_RESOLUTION);
		ds18b20_data[ds18b20_data.size() - 1].status = UNSPECIFIED_STATUS;

		return true;
	}

	return false;
}

bool SensorsManager::deleteDS18B20(uint8_t index) {
	if (!isCorrectDS18B20Index(index)) {
		return false;
	}

	if (ds18b20_data.del(index)) {
		#ifdef MODULE_MANAGER_BLYNK_SUPPORT
		system->deleteBlynkLink(String("HSdst") + getDS18B20Name(index));
		#endif

		return true;
	}

	return false;
}

void SensorsManager::updateSensorsData() {
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
			ds18b20_data[i].t += getDS18B20Correction(i);
		}
	}
}


uint8_t SensorsManager::makeDS18B20AddressList(DynamicArray<DeviceAddress>* array, DynamicArray<float>* t_array, DynamicArray<String>* string_array) {
	if (array == NULL) {
		return 0;
	}

	uint8_t sensors_count = getGlobalDS18B20Count();
	array->clear();

	if (t_array != NULL) {
		t_array->clear();
		ds18b20_sensor.requestTemperatures();
	}

	if (string_array != NULL) {
		string_array->clear();
	}

	if (!sensors_count) {
		return 0;
	}

	for (uint8_t i = 0;i < sensors_count;i++) {
		DeviceAddress address;

		ds18b20_sensor.getAddress(address, i);
		array->add(&address);

		if (t_array != NULL) {
			t_array->add(ds18b20_sensor.getTempC(address));
		}

		if (string_array != NULL) {
			String string_address;

			DS18B20AddressToString(address, &string_address);
			string_array->add(string_address);
		}
	}

	return sensors_count;
}

int8_t SensorsManager::scanDS18B20AddressIndex(DynamicArray<DeviceAddress>* array, uint8_t* address) {
	if (array == NULL || address == NULL) {
		return -1;
	}

	for (uint8_t i = 0; i < array->size();i++) {
		if (!memcmp((*array)[i], address, 8)) {
			return i;
		}
	}

	return -1;
}


void SensorsManager::setSystemManager(SystemManager* system) {
	this->system = system;
}

void SensorsManager::setReadDataTime(uint8_t time) {
	read_data_time = constrain(time, 0, 100);
}


void SensorsManager::setDS18B20(uint8_t index, ds18b20_data_t* ds18b20) {
	setDS18B20Name(index, ds18b20->name);
	setDS18B20Address(index, ds18b20->address);
	setDS18B20Resolution(index, ds18b20->resolution);
	setDS18B20Correction(index, ds18b20->correction);
}

void SensorsManager::setDS18B20Name(uint8_t index, String name) {
	if (!isCorrectDS18B20Index(index)) {
		return;
	}
	
	#ifdef MODULE_MANAGER_BLYNK_SUPPORT
	system->modifyBlynkLinkElementCode(String("HSdst") + getDS18B20Name(index), String("HSdst") + name);
	#endif

	name.toCharArray(ds18b20_data[index].name, 3);
}

void SensorsManager::setDS18B20Address(uint8_t index, uint8_t* address) {
	if (!isCorrectDS18B20Index(index) || !*address) {
		return;
	}

	memcpy(ds18b20_data[index].address, address, 8);
	setDS18B20Resolution(index, getDS18B20Resolution(index, false));
}

void SensorsManager::setDS18B20Resolution(uint8_t index, uint8_t resolution) {
	if (!isCorrectDS18B20Index(index)) {
		return;
	}

	if (*getDS18B20Address(index)) {
		ds18b20_sensor.setResolution(getDS18B20Address(index), resolution);
		ds18b20_data[index].resolution = ds18b20_sensor.getResolution(getDS18B20Address(index));
	}
	else {
		ds18b20_data[index].resolution = resolution;
	}
}

void SensorsManager::setDS18B20Correction(uint8_t index, float correction) {
	if (!isCorrectDS18B20Index(index)) {
		return;
	}

	ds18b20_data[index].correction = constrain(correction, -20.0, 20.0);
}


DallasTemperature* SensorsManager::getDallasTemperature() {
	return &ds18b20_sensor;
}

uint8_t SensorsManager::getReadDataTime() {
	return read_data_time;
}


float SensorsManager::getAM2320T() {
	return am2320_data.t;
}

float SensorsManager::getAM2320H() {
	return am2320_data.h;
}

uint8_t SensorsManager::getAM2320Status() {
	return am2320_data.status;
}


uint8_t SensorsManager::getGlobalDS18B20Count() {
	ds18b20_sensor.begin();
	return ds18b20_sensor.getDS18Count();
}

float SensorsManager::getDS18B20TByAddress(uint8_t* address) {
	ds18b20_sensor.requestTemperaturesByAddress(address);
	return ds18b20_sensor.getTempC(address);
}

uint8_t SensorsManager::getDS18B20Count() {
	return ds18b20_data.size();
}

ds18b20_data_t* SensorsManager::getDS18B20(uint8_t index) {
	if (!isCorrectDS18B20Index(index)) {
		return NULL;
	}

	return &ds18b20_data[index];
}

char* SensorsManager::getDS18B20Name(uint8_t index) {
	if (!isCorrectDS18B20Index(index)) {
		return NULL;
	}

	return ds18b20_data[index].name;
}

uint8_t* SensorsManager::getDS18B20Address(uint8_t index) {
	if (!isCorrectDS18B20Index(index)) {
		return NULL;
	}

	return ds18b20_data[index].address;
}

uint8_t SensorsManager::getDS18B20Resolution(uint8_t index, bool sync_flag) {
	if (!isCorrectDS18B20Index(index)) {
		return 0;
	}

	if (*getDS18B20Address(index) && sync_flag) {
		ds18b20_data[index].resolution = ds18b20_sensor.getResolution(getDS18B20Address(index));
	}

	return ds18b20_data[index].resolution;
}

float SensorsManager::getDS18B20Correction(uint8_t index) {
	if (!isCorrectDS18B20Index(index)) {
		return 0;
	}

	return ds18b20_data[index].correction;
}

float SensorsManager::getDS18B20T(uint8_t index) {
	if (!isCorrectDS18B20Index(index)) {
		return 0;
	}

	return ds18b20_data[index].t;
}

uint8_t SensorsManager::getDS18B20Status(uint8_t index) {
	if (!isCorrectDS18B20Index(index)) {
		return UNSPECIFIED_STATUS;
	}

	return ds18b20_data[index].status;
}


bool SensorsManager::isCorrectDS18B20Index(uint8_t index) {
	if (index >= getDS18B20Count()) {
		return false;
	}

	return true;
}

void SensorsManager::DS18B20AddressToString(uint8_t* address, String* string) {
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