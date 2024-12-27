/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.2.0
 * Date: 27.12.2024
 */

#include "data.h"

SolarSystemManager::SolarSystemManager() {
	makeDefault();
}

void SolarSystemManager::begin() {
	pinMode(RELE_PORT, OUTPUT);
	setReleFlag(false);
}


void SolarSystemManager::tick() {
	releTick();

	if (!work_flag) {
		return;
	}
	
	if (getStatus()) {
		if (getErrorOnFlag()) {
			setReleFlag(true);
		}
		
		return;
	}

	float delta_now = getBatteryT() - getBoilerT();

	if (delta_now >= delta) {
		setReleFlag(true);
	}
	else if (delta_now <= delta - 2) {
		setReleFlag(false);
	}
}

void SolarSystemManager::makeDefault() {
	system = NULL;

	work_flag = DEFAULT_SOLAR_WORK_FLAG;
	error_on_flag = DEFAULT_SOLAR_ERROR_ON_FLAG;
	rele_invert_flag = DEFAULT_SOLAR_RELE_INVERT_FLAG;
	delta = DEFAULT_SOLAR_DELTA;

	battery_sensor_index = -1;
	boiler_sensor_index = -1;
	exit_sensor_index = -1;

	rele_flag = false;
}

void SolarSystemManager::writeSettings(char* buffer) {
	setParameter(buffer, "SSSs", getWorkFlag());
	setParameter(buffer, "SSSeo", getErrorOnFlag());
	setParameter(buffer, "SSSri", getReleInvertFlag());// -----------
	setParameter(buffer, "SSSd", getDelta());
	setParameter(buffer, "SSSba", getBatterySensor());
	setParameter(buffer, "SSSbo", getBoilerSensor());
	setParameter(buffer, "SSSex", getExitSensor());
}

void SolarSystemManager::readSettings(char* buffer) {
	getParameter(buffer, "SSSs", &work_flag);
	getParameter(buffer, "SSSeo", &error_on_flag);
	getParameter(buffer, "SSSri", &rele_invert_flag);
	getParameter(buffer, "SSSd", &delta);
	getParameter(buffer, "SSSba", &battery_sensor_index);
	getParameter(buffer, "SSSbo", &boiler_sensor_index);
	getParameter(buffer, "SSSex", &exit_sensor_index);
}

void SolarSystemManager::addBlynkElements(BlynkManager* blynk) {
	blynk->addElement("PSL status", "SSSs", &work_flag, BLYNK_TYPE_BOOL);
	blynk->addElement("PSL rele", "HSSpu", &rele_flag, BLYNK_TYPE_BOOL);
	blynk->addElement("PSl error on", "SSSeo", &error_on_flag, BLYNK_TYPE_BOOL);
	blynk->addElement("PSl rele inv", "SSSri", &rele_invert_flag, BLYNK_TYPE_BOOL); // -------
	blynk->addElement("PSL delta", "SSSd", &delta, BLYNK_TYPE_UINT8_T);
}


void SolarSystemManager::setSystemManager(SystemManager* system) {
	this->system = system;
}


void SolarSystemManager::setReleFlag(bool rele_flag) {
	this->rele_flag = rele_flag;
	releTick();
}

void SolarSystemManager::setWorkFlag(bool work_flag) {
	this->work_flag = work_flag;
	tick();
}

void SolarSystemManager::setErrorOnFlag(bool error_on_flag) {
	this->error_on_flag = error_on_flag;
	tick();
}

void SolarSystemManager::setReleInvertFlag(bool rele_invert_flag) {
	this->rele_invert_flag = rele_invert_flag;
}

void SolarSystemManager::setDelta(uint8_t delta) {
	this->delta = constrain(delta, SOLAR_DELTA_MIN, SOLAR_DELTA_MAX);
	tick();
}


void SolarSystemManager::setSensor(uint8_t index, int8_t sensor_index) {
	switch (index) {
	case 0:
		setBatterySensor(sensor_index);
		break;
	case 1:
		setBoilerSensor(sensor_index);
		break;
	case 2:
		setExitSensor(sensor_index);
		break;
	}
}

void SolarSystemManager::setBatterySensor(int8_t battery_sensor_index) {
	this->battery_sensor_index = constrain(battery_sensor_index, -1, DS_SENSORS_COUNT - 1);
	tick();
}

void SolarSystemManager::setBoilerSensor(int8_t boiler_sensor_index) {
	this->boiler_sensor_index = constrain(boiler_sensor_index, -1, DS_SENSORS_COUNT - 1);
	tick();
}

void SolarSystemManager::setExitSensor(int8_t exit_sensor_index) {
	this->exit_sensor_index = constrain(exit_sensor_index, -1, DS_SENSORS_COUNT - 1);
	tick();
}


SystemManager* SolarSystemManager::getSystemManager() {
	return system;
}

uint8_t SolarSystemManager::getStatus() {
	if (!getWorkFlag()) return 1;
	if (getBatterySensorStatus()) return 2;
	if (getBoilerSensorStatus()) return 3;
	if (getExitSensorStatus()) return 4;

	return 0;
}


bool SolarSystemManager::getReleFlag() {
	return rele_flag;
}

bool SolarSystemManager::getWorkFlag() {
  	return work_flag;
}

bool SolarSystemManager::getErrorOnFlag() {
  	return error_on_flag;
}

bool SolarSystemManager::getReleInvertFlag() {
	return rele_invert_flag;
}

uint8_t SolarSystemManager::getDelta() {
  	return delta;
}


uint8_t SolarSystemManager::getBatterySensorStatus() {
	ModuleManager* modules = system->getModuleManager();

	if (getBatterySensor() >= DS_SENSORS_COUNT || getBatterySensor() < 0) return 1;
	if (modules->getDS18B20Status(getBatterySensor()) ) return 2;

	return 0;
}

uint8_t SolarSystemManager::getBoilerSensorStatus() {
	ModuleManager* modules = system->getModuleManager();

	if (getBoilerSensor() >= DS_SENSORS_COUNT || getBoilerSensor() < 0) return 1;
	if (modules->getDS18B20Status(getBoilerSensor()) ) return 2;

	return 0;
}

uint8_t SolarSystemManager::getExitSensorStatus() {
	ModuleManager* modules = system->getModuleManager();

	if (getExitSensor() >= DS_SENSORS_COUNT || getExitSensor() < 0) return 1;
	if (modules->getDS18B20Status(getExitSensor()) ) return 2;

	return 0;
}


int8_t SolarSystemManager::getSensor(uint8_t index) {
	switch (index) {
	case 0:
		return getBatterySensor();
		break;
	case 1:
		return getBoilerSensor();
		break;
	case 2:
		return getExitSensor();
		break;
	default:
		return -1;
	}
}

int8_t SolarSystemManager::getBatterySensor() {
  	return battery_sensor_index;
}

int8_t SolarSystemManager::getBoilerSensor() {
  	return boiler_sensor_index;
}

int8_t SolarSystemManager::getExitSensor() {
  	return exit_sensor_index;
}


float SolarSystemManager::getBatteryT() {
	ModuleManager* modules = system->getModuleManager();

	if (getBatterySensorStatus()) {
		return 0;
	}

	return modules->getDS18B20T(getBatterySensor());
}

float SolarSystemManager::getBoilerT() {
	ModuleManager* modules = system->getModuleManager();

	if (getBoilerSensorStatus()) {
		return 0;
	}

	return modules->getDS18B20T(getBoilerSensor());
}

float SolarSystemManager::getExitT() {
	ModuleManager* modules = system->getModuleManager();

	if (getExitSensorStatus()) {
		return 0;
	}

	return modules->getDS18B20T(getExitSensor());
}


void SolarSystemManager::releTick() {
	digitalWrite(RELE_PORT, getReleInvertFlag() ? !getReleFlag() : getReleFlag());
}