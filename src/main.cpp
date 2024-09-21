/*
 * File: main.cpp
 * 
 * Description:
 * This file defines the main variables and objects used throughout the project.
 * It also contains the setup() and loop() functions, which initialize and run the core functionality of the system.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Date: 02.09.2024
 */

#include "data.h"

uint8_t read_data_time;
bool buzzer_status;
bool rele_invert;
int8_t gmt;
bool ntp_sync;

bool problem[PROBLEMS_COUNT];
sensor_t ds_sensors[DS_SENSORS_COUNT];
float t_home;
float h_home;

uint32_t read_data_timer;
uint32_t blink_timer;
uint32_t ntp_sync_timer;
uint32_t save_settings_timer;

bool rele_flag;
bool blink_flag;
bool save_settings_flag;

// настройки (час / інше) / змінні датчиків даних покажчиків / (Додаткове) тімери / флажки / всьо інше

Solar solar;
Display display;
Network network;
BlynkManage blynk;

DallasTemperature sensor(&oneWire);
OneWire oneWire(DS18B20_PORT);
AM2320 am;
Clock clk;
Encoder enc(CLK_PORT, DT_PORT, SW_PORT);
LiquidCrystal_I2C lcd(0x27, 20, 4);
WiFiUDP udp;
GyverPortal ui(&LittleFS);
static WiFiClient _blynkWifiClient;
static BlynkArduinoClient _blynkTransport(_blynkWifiClient);
BlynkWifi Blynk(_blynkTransport);

void setup() {
	Serial.begin(9600);
	LittleFS.begin();
	
	makeDefault();
	
	pinMode(RELE_PORT, OUTPUT);
	pinMode(BUZZER_PORT, OUTPUT);
	pinMode(SW_PORT, INPUT_PULLUP);
	setRele(false);
	
	attachInterrupt(CLK_PORT, clkInterrupt, CHANGE);
	attachInterrupt(DT_PORT, dtInterrupt, CHANGE);
	attachInterrupt(SW_PORT, swInterrupt, CHANGE);
	enc.setType(TYPE2);

	sensor.begin();
	sensor.setResolution(12);
	display.makeDefault();
	
	udp.begin(2390);
	ui.attachBuild(uiBuild);
	ui.attach(uiAction);
	ui.enableOTA();

	blynk.addElement("T home", "HSt", &t_home, BLYNK_TYPE_FLOAT);
	blynk.addElement("H home", "HSh", &h_home, BLYNK_TYPE_FLOAT);
	blynk.addElement("rele", "HSSpu", &rele_flag, BLYNK_TYPE_BOOL);
	blynk.addElement("P data time", "SStd", &read_data_time, BLYNK_TYPE_UINT8_T);
	blynk.addElement("P buzzer", "SSb", &buzzer_status, BLYNK_TYPE_BOOL);
	blynk.addElement("P rele invr", "SSri", &rele_invert, BLYNK_TYPE_BOOL);

	for (uint8_t i = 0;i < DS_SENSORS_COUNT;i++) {
		blynk.addElement(ds_sensors[i].name, String("SSDt") + i, &ds_sensors[i].t, BLYNK_TYPE_FLOAT);
	}
	
	blynk.addElement("P gmt", "SSTg", &gmt, BLYNK_TYPE_INT8_T);
	blynk.addElement("P ntp sync", "SSTns", &ntp_sync, BLYNK_TYPE_BOOL);

	display.addBlynkElements();
	solar.addBlynkElements();

	readSettings();
}

void loop() {
 	systemTick();
}