/*
 * File: functions.cpp
 * 
 * Description:
 * This file contains additional functions that support the operation of the system. 
 * It includes the systemTick() function, which is called within the loop() function. 
 * The systemTick() function sequentially calls class methods to ensure orderly operation.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Date: 02.09.2024
 */


#include "data.h"

ICACHE_RAM_ATTR void clkInterrupt() {
	enc.tick();
}

ICACHE_RAM_ATTR void dtInterrupt() {
  	enc.tick();
}

ICACHE_RAM_ATTR void swInterrupt() {
  	enc.tick();
}


void getNextSensor(uint8_t* address, int8_t step) {
	uint8_t read_address[8];
	uint8_t count = sensor.getDeviceCount();
	int8_t index = -1;

	if (*address) {
		for (uint8_t i = 0;i < count;i++) {
			sensor.getAddress(read_address, i);
			
			if (!memcmp(address, read_address, 8)) {
				index = i;
			}
		}
	}

	smartIncr(index, step, -1, count - 1);

	if (index < 0) {
		memset(address, 0x00, 8);
	}
	else {
		sensor.getAddress(address, index);
	}
}


void makeDefault() {
	read_data_time = DEFAULT_READ_DATA_TIME;
	buzzer_status = DEFAULT_BUZZER_STATUS;
	rele_invert = DEFAULT_RELE_INVERT;
	gmt = DEFAULT_GMT;
	ntp_sync = DEFAULT_NTP_SYNC;

	memset(ds_sensors, 0, sizeof(ds_sensors));
	for (uint8_t i = 0;i < DS_SENSORS_COUNT;i++) {
		sprintf(ds_sensors[i].name, "T%d", i + 1);
		ds_sensors[i].problem = true;
	}

	memset(problem, true, PROBLEMS_COUNT * sizeof(bool));

	solar.makeDefault();
	network.makeDefault();
	blynk.makeDefault();
}

void resetAll() {
	LittleFS.remove("/config.nztr");

  	ESP.reset();
}

void saveSettings() {
	File file = LittleFS.open("/config.nztr", "w");
	char buffer[SETTINGS_BUFFER_SIZE + 1] = "";

	setParameter(buffer, "SStd", read_data_time);
	setParameter(buffer, "SSb", buzzer_status);
	setParameter(buffer, "SSri", rele_invert);
	setParameter(buffer, "SSTg", gmt);
	setParameter(buffer, "SSTns", ntp_sync);

	for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
		setParameter(buffer, String("SSDtn") + i, (const char*) ds_sensors[i].name);
		setParameter(buffer, String("SSDta") + i, ds_sensors[i].address, 8);
		setParameter(buffer, String("SSDtc") + i, ds_sensors[i].correction);
  	}

	solar.saveSettings(buffer);
	network.saveSettings(buffer);
	blynk.saveSettings(buffer);

	file.write(buffer, strlen(buffer));
  	file.close();
}

void readSettings() {
	File file = LittleFS.open("/config.nztr", "r");

	if (!file) {
		saveSettings();
		return;
	}

	uint16_t file_size = file.size();
	char* buffer = new char[file_size + 1];

	file.read((uint8_t*) buffer, file_size);
	buffer[file_size] = 0;
	
	getParameter(buffer, "SStd", &read_data_time);
	getParameter(buffer, "SSb", &buzzer_status);
	getParameter(buffer, "SSri", &rele_invert);
	getParameter(buffer, "SSTg", &gmt);
	getParameter(buffer, "SSTns", &ntp_sync);

	for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
		getParameter(buffer, String("SSDtn") + i, ds_sensors[i].name, 3);
		getParameter(buffer, String("SSDta") + i, ds_sensors[i].address, 8);
		getParameter(buffer, String("SSDtc") + i, &ds_sensors[i].correction);
  	}

	solar.readSettings(buffer);
	network.readSettings(buffer);
	blynk.readSettings(buffer);

	delete[] buffer;
	file.close();
}


void systemTick() {
	// static uint32_t last_time;
	// Serial.println(millis() - last_time);
	// last_time = millis();
  	yield();
  
	if (enc.isTurn() || enc.isPress()) {
		if (display.action()) {
			enc.resetStates();
		}
	}

	timers();
	solar.tick();
	releTick();
	enc.tick();
	display.tick();
	network.tick();
	ui.tick();
	Blynk.run();
}

void timers() {
	if (millis() - blink_timer >= BLINK_TIME) {
		blink_timer = millis();
		blink_flag = !blink_flag;
	}

	if (read_data_time && (millis() - read_data_timer >= read_data_time * 1000 || !read_data_timer) ) {
		read_data_timer = millis();
		
		readSensors();
		blynk.tick();
	}

	if (ntp_sync && (millis() - ntp_sync_timer >= NTP_SYNC_TIME || !ntp_sync_timer)) {
		if (ntpSync()) {
			ntp_sync_timer = millis();
		}
	}

	if (save_settings_flag && millis() - save_settings_timer >= SAVE_SETTINGS_TIME) {
		save_settings_timer = millis();
		save_settings_flag = false;

		saveSettings();
	}
}

void readSensors() {
	problem[0] = (bool)clk.status();
	problem[1] = am.read(&t_home, &h_home);

	sensor.requestTemperatures();
	for (uint8_t i = 0;i < DS_SENSORS_COUNT;i++) {
		ds_sensors[i].t = sensor.getTempC(ds_sensors[i].address) + ds_sensors[i].correction;
		ds_sensors[i].problem = (ds_sensors[i].t < -100 || ds_sensors[i].t == 85) ? true : false;
	}
}

void setRele(bool status) {
  	rele_flag = status;
 	releTick();
}

bool getRele() {
	return rele_flag;
}

void releTick() {
  	digitalWrite(RELE_PORT, (rele_invert) ? !rele_flag : rele_flag);
}
