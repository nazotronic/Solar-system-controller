/*
 * File: solarSystem.cpp
 * 
 * Description:
 * This file contains the code responsible for controlling the physical solar battery system. 
 * It ensures the proper operation of the components, such as the water tank, sensors, pump, and solar panel.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Date: 02.09.2024
 */

#include "data.h"

Solar::Solar() {
	makeDefault();
}

void Solar::makeDefault() {
	battery_sensor = -1;
	boiler_sensor = -1;
	exit_sensor = -1;

	work_status = DEFAULT_SOLAR_STATUS;
	on_if_problem = DEFAULT_SOLAR_ON_IF_PROBLEM;
	delta = DEFAULT_SOLAR_DELTA;
}

void Solar::saveSettings(char* buffer) {
	setParameter(buffer, "SSSs", work_status);
	setParameter(buffer, "SSSo", on_if_problem);
	setParameter(buffer, "SSSd", delta);
	setParameter(buffer, "SSSba", battery_sensor);
	setParameter(buffer, "SSSbo", boiler_sensor);
	setParameter(buffer, "SSSex", exit_sensor);
}

void Solar::readSettings(char* buffer) {
	getParameter(buffer, "SSSs", &work_status);
	getParameter(buffer, "SSSo", &on_if_problem);
	getParameter(buffer, "SSSd", &delta);
	getParameter(buffer, "SSSba", &battery_sensor);
	getParameter(buffer, "SSSbo", &boiler_sensor);
	getParameter(buffer, "SSSex", &exit_sensor);
}

void Solar::addBlynkElements() {
	blynk.addElement("PSL status", "SSSs", &work_status, BLYNK_TYPE_BOOL);
	blynk.addElement("PSL onifp", "SSSo", &delta, BLYNK_TYPE_BOOL);
	blynk.addElement("PSl delta","SSSd", &on_if_problem, BLYNK_TYPE_UINT8_T);
}

void Solar::tick() {
	if (!work_status) {
		return;
	}
	
	if (status()) {
		if (on_if_problem) {
			setRele(true);
		}
		
		return;
	}

	float delta_now = getBatteryT() - getBoilerT();
	if (delta_now >= delta) {
		setRele(true);
	}
	else if (delta_now <= delta - 2) {
		setRele(false);
	}
}


uint8_t Solar::status() {
	if (!work_status) return 1;
	
	if (battery_sensor >= DS_SENSORS_COUNT || battery_sensor < 0) return 2;
	if (ds_sensors[battery_sensor].problem) return 3;

	if (boiler_sensor >= DS_SENSORS_COUNT || boiler_sensor < 0) return 4;
	if (ds_sensors[boiler_sensor].problem) return 5;

	if (exit_sensor >= DS_SENSORS_COUNT || exit_sensor < 0) return 6;
	if (ds_sensors[exit_sensor].problem) return 7;

	return 0;
}


void Solar::setWorkStatus(bool work_status) {
	this->work_status = work_status;
	tick();
}

void Solar::setOnIfProblem(bool on_if_problem) {
	this->on_if_problem = on_if_problem;
	tick();
}

void Solar::setDelta(uint8_t delta) {
	this->delta = constrain(delta, SOLAR_DELTA_MIN, SOLAR_DELTA_MAX);
	tick();
}

void Solar::setSensor(uint8_t index, int8_t sensor_index) {
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
// sensor index!!!
void Solar::setBatterySensor(int8_t battery_sensor) {
	this->battery_sensor = constrain(battery_sensor, -1, DS_SENSORS_COUNT - 1);
	tick();
}

void Solar::setBoilerSensor(int8_t boiler_sensor) {
	this->boiler_sensor = constrain(boiler_sensor, -1, DS_SENSORS_COUNT - 1);
	tick();
}

void Solar::setExitSensor(int8_t exit_sensor) {
	this->exit_sensor = constrain(exit_sensor, -1, DS_SENSORS_COUNT - 1);
	tick();
}

bool Solar::getWorkStatus() {
  	return work_status;
}

bool Solar::getOnIfProblem() {
  	return on_if_problem;
}

uint8_t Solar::getDelta() {
  	return delta;
}

int8_t Solar::getSensor(uint8_t index) {
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

int8_t Solar::getBatterySensor() {
  	return battery_sensor;
}

int8_t Solar::getBoilerSensor() {
  	return boiler_sensor;
}

int8_t Solar::getExitSensor() {
  	return exit_sensor;
}


float Solar::getBatteryT() {
	if (batterySensor()) {
		return 0;
	}

	return ds_sensors[battery_sensor].t;
}

float Solar::getBoilerT() {
	if (boilerSensor()) {
		return 0;
	}

	return ds_sensors[boiler_sensor].t;
}

float Solar::getExitT() {
	if (exitSensor()) {
		return 0;
	}

	return ds_sensors[exit_sensor].t;
}


uint8_t Solar::batterySensor() {
	if (battery_sensor >= DS_SENSORS_COUNT || battery_sensor < 0) {
		return 1;
	}
	
	if (ds_sensors[battery_sensor].problem || battery_sensor < 0) {
		return 2;
	}

	return 0;
}

uint8_t Solar::boilerSensor() {
	if (boiler_sensor >= DS_SENSORS_COUNT || boiler_sensor < 0) {
		return 1;
	}
	
	if (ds_sensors[boiler_sensor].problem) {
		return 2;
	}

	return 0;
}

uint8_t Solar::exitSensor() {
	if (exit_sensor >= DS_SENSORS_COUNT || exit_sensor < 0) {
		return 1;
	}
	
	if (ds_sensors[exit_sensor].problem) {
		return 2;
	}

	return 0;
}