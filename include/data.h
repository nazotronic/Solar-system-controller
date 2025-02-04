/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.1
 * Date: 04.02.2025
 * 
 * Features:
 * 1. Supports reading up to 10 ds18b20 temperature sensors with calibration capability.
 * 2. Supports AM2320 sensor for temperature and humidity display.
 * 3. Easy and convenient functionality.
 * 4. Supports WiFi, access point, and can operate in automatic mode which enables the access point if no WiFi is available.
 * 5. Supports Blynk for remote monitoring from anywhere in the world.
 * 6. Supports a web interface for configuration and information display.
 * 7. Supports signal reversal on relay.
 */

#pragma once
#include <Arduino.h>
#include <DallasTemperature.h>
#include <AM2320.h>
#include <Clock.h>
#include <settings.h>
#include <DynamicArray.h>
#include <LittleFS.h>
#include <GyverEncoder.h>
#include <LiquidCrystal_I2C.h> 
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverPortal.h>

#define NO_GLOBAL_BLYNK
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

/* --- Ports --- */
#define DS18B20_PORT D4
#define BUZZER_PORT D8
#define CLK_PORT D5
#define DT_PORT D6
#define SW_PORT D7
#define RELE_PORT D0

/* --- Defaults --- */
/* SystemManager */
#define DEFAULT_BUZZER_FLAG true

/* TimeManager */
#define DEFAULT_NTP_FLAG true
#define DEFAULT_GMT 0

/* SensorsManager */
#define DEFAULT_READ_DATA_TIME 5 // sec
#define DEFAULT_DS18B20_NAME "Tn"
#define DEFAULT_DS18B20_RESOLUTION 12

/* SolarSystemManager */
#define DEFAULT_SOLAR_WORK_FLAG true
#define DEFAULT_SOLAR_ERROR_ON_FLAG true
#define DEFAULT_SOLAR_RELE_INVERT_FLAG true
#define DEFAULT_SOLAR_DELTA 5

/* DisplayManager */
#define DEFAULT_DISPLAY_WORK_FLAG true
#define DEFAULT_DISPLAY_AUTO_RESET_FLAG true
#define DEFAULT_DISPLAY_BACKLIGHT_OFF_TIME 10 // sec
#define DEFAULT_DISPLAY_FPS 10

/* NetworkManager */
#define DEFAULT_NETWORK_MODE NETWORK_AUTO
#define DEFAULT_NETWORK_SSID_AP "nztr_solar"
#define DEFAULT_NETWORK_PASS_AP "nazotronic"

/* BlynkManager */
#define DEFAULT_BLYNK_WORK_STATUS true
#define DEFAULT_BLYNK_SEND_DATA_TIME DEFAULT_READ_DATA_TIME // sec

/* --- Macroces --- */
/* SystemManager */
#define SAVE_SETTINGS_TIME 5 // sec
#define SETTINGS_BUFFER_SIZE 1100

/* TimeManager */
#define NTP_SYNC_TIME 1 // min

/* SensorsManager */
#define MODULE_MANAGER_BLYNK_SUPPORT
#define UNSPECIFIED_STATUS 255
#define DS_SENSORS_MAX_COUNT 10
#define DS_NAME_SIZE 3

/* SolarSystemManager */
#define SOLAR_SYSTEM_MANAGER_BLYNK_SUPPORT
#define SOLAR_DELTA_MIN 3
#define SOLAR_DELTA_MAX 10

/* NetworkManager */
#define NETWORK_OFF 0
#define NETWORK_STA 1
#define NETWORK_AP_STA 2
#define NETWORK_AUTO 3
#define NETWORK_SSID_PASS_SIZE 15
#define NETWORK_RECONNECT_TIME 20 // sec

#define UDP_RESEND_TIME 5 //sec
#define NTP_SERVER "time.nist.gov"
#define NTP_PORT 123

#define WEB_UPDATE_TIME 10 // sec

/* BlynkManager */
#define BLYNK_TYPE_UINT8_T 0
#define BLYNK_TYPE_INT8_T 1
#define BLYNK_TYPE_UINT32_T 2
#define BLYNK_TYPE_INT32_T 3
#define BLYNK_TYPE_BOOL 4
#define BLYNK_TYPE_FLOAT 5
#define BLYNK_LINKS_MAX 20
#define BLYNK_AUTH_SIZE 35
#define BLYNK_ELEMENT_CODE_SIZE 10
#define BLYNK_RECONNECT_TIME 20 // sec

/* --- Macro functions --- */
#define SEC_TO_MLS(TIME) ((TIME) * 1000)
#define MIN_TO_MLS(TIME) ((TIME) * 60000)
#define IS_EVEN_SECOND(MLS) ((MLS / 1000) % 2)

const uint8_t wifi[] = {0b00000, 0b01110, 0b10001, 0b00100, 0b01010, 0b00000, 0b00100, 0b00000};
const uint8_t down_symbol[] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b10001, 0b01010, 0b00100};

const uint8_t LT[] = {0b00111,  0b01111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
const uint8_t UB[] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};
const uint8_t RT[] = {0b11100,  0b11110,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
const uint8_t LL[] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b01111,  0b00111};
const uint8_t LB[] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
const uint8_t LR[] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11110,  0b11100};
const uint8_t UMB[] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};

const char keyboard1[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '.', '_', '-', '!', '?', ',', '@', '%', '/', '|', '#', '*', '<', 'E'};
const char keyboard2[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '<', '>', '<', 'E'};

struct ds18b20_data_t {
	void operator=(const ds18b20_data_t& other) {
		strcpy(name, other.name);
		memcpy(address, other.address, sizeof(DeviceAddress));
		resolution = other.resolution;
		correction = other.correction;

		t = other.t;
		status = other.status;
	}
	
	char name[DS_NAME_SIZE];
	DeviceAddress address;
	uint8_t resolution;
	float correction;
	
	float t;
	uint8_t status;
};

struct blynk_element_t {
	blynk_element_t(String code, void* pointer, uint8_t type) {
		this->pointer = pointer;
		this->type = type;
		
		strcpy(this->code, code.c_str());
	}

	void operator=(const blynk_element_t& other) {
		strcpy(code, other.code);
		pointer = other.pointer;
		type = other.type;
	}

	char code[BLYNK_ELEMENT_CODE_SIZE];
	void* pointer;
	uint8_t type;
};

struct blynk_link_t {
	void operator=(const blynk_link_t& other) {
		port = other.port;
		strcpy(element_code, other.element_code);
	}

	uint8_t port;
	char element_code[BLYNK_ELEMENT_CODE_SIZE];
};

class NetworkManager;
class SystemManager;
class BlynkManager;

class TimeManager {
public:
	TimeManager();
	void begin();

	void tick();
	void makeDefault();
	void writeSettings(char* buffer);
	void readSettings(char* buffer);
#ifdef TIME_MANAGER_BLYNK_SUPPORT
	void addBlynkElementCodes(DynamicArray<String>* array);
	bool blynkElementSend(BlynkWifi* Blynk, blynk_link_t* link);
	bool blynkElementParse(String code, const BlynkParam& param);
#endif

	uint8_t status();
	uint8_t hour();
	uint8_t minute();
	uint8_t second();
	uint8_t weekday();
	uint8_t day();
	uint8_t month();
	uint16_t year();

	void setSystemManager(SystemManager* system);

	void setNtpFlag(bool ntp_flag);
	void setGmt(int8_t gmt);
	void setTime(TimeT* time);
	void setTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint16_t year);
	void setUnix(uint32_t unix);

	SystemManager* getSystemManager();
	uint8_t getStatus();

	bool getNtpFlag();
	int8_t getGmt();
	TimeT getTime();
	uint32_t getUnix();

private:
	SystemManager* system;
	Clock clk;

	bool ntp_flag;
	int8_t gmt;

	uint32_t ntp_sync_timer;
};

class SensorsManager {
public:
	SensorsManager();
	void begin();

	void tick();
	void makeDefault();
	void writeSettings(char* buffer);
	void readSettings(char* buffer);
#ifdef MODULE_MANAGER_BLYNK_SUPPORT
	void addBlynkElementCodes(DynamicArray<String>* array);
	bool blynkElementSend(BlynkWifi* Blynk, blynk_link_t* link);
	bool blynkElementParse(String code, const BlynkParam& param);
#endif

	void updateSensorsData();
	bool addDS18B20();
	bool deleteDS18B20(uint8_t index);

	uint8_t makeDS18B20AddressList(DynamicArray<DeviceAddress>* array, DynamicArray<float>* t_array = NULL, DynamicArray<String>* string_array = NULL);
	int8_t scanDS18B20AddressIndex(DynamicArray<DeviceAddress>* array, uint8_t* address);
	
	void setSystemManager(SystemManager* system);
	void setReadDataTime(uint8_t time);

	void setDS18B20(uint8_t index, ds18b20_data_t* ds18b20);
	void setDS18B20Name(uint8_t index, String name);
	void setDS18B20Address(uint8_t index, uint8_t* address);
	void setDS18B20Resolution(uint8_t index, uint8_t resolution);
	void setDS18B20Correction(uint8_t index, float correction);

	DallasTemperature* getDallasTemperature();
	uint8_t getReadDataTime();

	float getAM2320T();
	float getAM2320H();
	uint8_t getAM2320Status();

	uint8_t getGlobalDS18B20Count();
	float getDS18B20TByAddress(uint8_t* address);
	uint8_t getDS18B20Count();
	ds18b20_data_t* getDS18B20(uint8_t index);
	char* getDS18B20Name(uint8_t index);
	uint8_t* getDS18B20Address(uint8_t index);
	uint8_t getDS18B20Resolution(uint8_t index, bool sync_flag = true);
	float getDS18B20Correction(uint8_t index);
	float getDS18B20T(uint8_t index);
	uint8_t getDS18B20Status(uint8_t index);

private:
	bool isCorrectDS18B20Index(uint8_t index);
	void DS18B20AddressToString(uint8_t* address, String* string);

	AM2320 am2320_sensor;
	OneWire oneWire;
	DallasTemperature ds18b20_sensor;
	SystemManager* system;

	uint8_t read_data_time;

	struct am2320_data_t {
		float t;
		float h;
		uint8_t status;

	} am2320_data;
	DynamicArray<ds18b20_data_t> ds18b20_data;

	uint32_t read_data_timer;
};

class SolarSystemManager {
public:
	SolarSystemManager();
	void begin();

	void tick();
	void makeDefault();
	void writeSettings(char* buffer);
	void readSettings(char* buffer);
#ifdef SOLAR_SYSTEM_MANAGER_BLYNK_SUPPORT
	void addBlynkElementCodes(DynamicArray<String>* array);
	bool blynkElementSend(BlynkWifi* Blynk, blynk_link_t* link);
	bool blynkElementParse(String code, const BlynkParam& param);
#endif

	void setSystemManager(SystemManager* system);

	void setReleFlag(bool rele_flag);
	void setWorkFlag(bool work_flag);
	void setErrorOnFlag(bool error_on_flag);
	void setReleInvertFlag(bool rele_invert_flag);
	void setDelta(uint8_t delta);

	void setSensor(uint8_t solar_sensor, int8_t ds18b20_index);
	void setBatterySensor(int8_t ds18b20_index);
	void setBoilerSensor(int8_t ds18b20_index);
	void setExitSensor(int8_t ds18b20_index);

	SystemManager* getSystemManager();
	uint8_t getStatus();

	bool getReleFlag();
	bool getWorkFlag();
	bool getErrorOnFlag();
	bool getReleInvertFlag();
	uint8_t getDelta();
	
	uint8_t getBatterySensorStatus();
	uint8_t getBoilerSensorStatus();
	uint8_t getExitSensorStatus();

	int8_t getSensor(uint8_t solar_sensor);
	int8_t getBatterySensor();
	int8_t getBoilerSensor();
	int8_t getExitSensor();

	float getBatteryT();
	float getBoilerT();
	float getExitT();

private:
	void releTick();

	SystemManager* system;

	bool work_flag;
	bool error_on_flag;
	bool rele_invert_flag;
	uint8_t delta;

	int8_t battery_sensor_index;
	int8_t boiler_sensor_index;
	int8_t exit_sensor_index;

	bool rele_flag;
	
};

#include "display.h"

class NetworkManager {
public:
	NetworkManager();
	void begin();
	void endBegin();

	void tick();
	void makeDefault();
	void writeSettings(char* buffer);
	void readSettings(char* buffer);
#ifdef NETWORK_MANAGER_BLYNK_SUPPORT
	void addBlynkElementCodes(DynamicArray<String>* array);
	bool blynkElementSend(BlynkWifi* Blynk, blynk_link_t* link);
	bool blynkElementParse(String code, const BlynkParam& param);
#endif
	
	bool connect(String ssid = String(""), String pass = String(""), uint8_t connect_time = 0, bool auto_save = false);
	void off();
	bool isWifiOn();
	bool isApOn();

	static void uiBuild();
	static void uiAction();
	bool ntpSync(TimeManager* time);

	void setSystemManager(SystemManager* system);

	void setMode(uint8_t mode);
	void setWifi(String* ssid, String* pass);
	void setWifi(const char* ssid, const char* pass);
	void setAp(String* ssid, String* pass);
	void setAp(const char* ssid, const char* pass);

	SystemManager* getSystemManager();
	wl_status_t getStatus();

	uint8_t getMode();
	char* getWifiSsid();
	char* getWifiPass();
	char* getApSsid();
	char* getApPass();

private:
	/* --- functions --- */
	static void updateWebSensorsBlock();
	static void updateWebBlynkBlock();

	/* --- settings --- */
	uint8_t mode;

	char ssid_sta[NETWORK_SSID_PASS_SIZE];
	char pass_sta[NETWORK_SSID_PASS_SIZE];
	char ssid_ap[NETWORK_SSID_PASS_SIZE];
	char pass_ap[NETWORK_SSID_PASS_SIZE];

	/* --- classes --- */
	static SystemManager* system;
	WiFiUDP udp;
	static GyverPortal ui;

	/* --- structures --- */
	struct web_blynk_block_t {
		String element_codes_string;
		DynamicArray<String> element_codes;
	};

	struct web_sensors_block_t {
		String ds18b20_addresses_string;
		DynamicArray<DeviceAddress> ds18b20_addresses;
	};

	/* --- variables --- */
	bool reset_request;
	bool tick_allow;
	uint32_t wifi_reconnect_timer;

	/* --- web variables --- */
	static String web_update_codes;
	static web_blynk_block_t web_blynk;
	static web_sensors_block_t web_sensors;

};

class BlynkManager {
public:
	BlynkManager();
	
	void tick();
	void makeDefault();
	void writeSettings(char* buffer);
	void readSettings(char* buffer);

	bool addLink();
	bool deleteLink(uint8_t index);
	bool deleteLink(String element_code);
	bool modifyLinkElementCode(String previous_code, String new_code);

	void setSystemManager(SystemManager* system);

	void setWorkFlag(bool work_flag);
	void setSendDataTime(uint8_t time);
	void setAuth(String auth);
	
	void setLinkPort(uint8_t index, uint8_t port);
	void setLinkElementCode(uint8_t index, String code);
	
	SystemManager* getSystemManager();
	bool getStatus();

	bool getWorkFlag();
	uint8_t getSendDataTime();
	char* getAuth();

	uint8_t getLinksCount();
	uint8_t getLinkPort(uint8_t index);
	char* getLinkElementCode(uint8_t index);

private:
	bool isCorrectLinkIndex(uint8_t index);
	int8_t scanLinkIndex(String element_code);

	void connectBlynk();
	void disconnectBlynk();

	void sendData();
	friend BLYNK_WRITE_DEFAULT();

	SystemManager* system;
	static WiFiClient _blynkWifiClient;
  	static BlynkArduinoClient _blynkTransport;
  	static BlynkWifi Blynk;
	
	bool work_flag;
	uint8_t send_data_time;
	char auth[BLYNK_AUTH_SIZE];

	DynamicArray<blynk_link_t> links;
	uint32_t send_data_timer;
	uint32_t blynk_reconnect_timer;
};

class SystemManager {
public:
	SystemManager();
	void begin();

	void tick();
	void makeDefault();
	void reset();
	void resetAll();

	void makeBlynkElementCodesList(DynamicArray<String>* array);
	void makeBlynkElementSend(BlynkWifi* Blynk, blynk_link_t* link);
	void makeBlynkElementParse(String element_code, const BlynkParam& param);
	int8_t scanBlynkElemetCodeIndex(DynamicArray<String>* array, String element_code);

	bool deleteBlynkLink(String element_code);
	bool modifyBlynkLinkElementCode(String previous_code, String new_code);

	void saveSettingsRequest();
	void buzzer(uint16_t freq, uint16_t duration);

	static void encoderClkInterrupt();
	static void encoderDtInterrupt();
	static void encoderSwInterrupt();

	void setBuzzerFlag(bool buzzer_flag);

	bool getBuzzerFlag();
	TimeManager* getTimeManager();
	SensorsManager* getSensorsManager();
	SolarSystemManager* getSolarSystemManager();
	DisplayManager* getDisplayManager();
	NetworkManager* getNetworkManager();
	BlynkManager* getBlynkManager();
	Encoder* getEncoder();

private:
	// SystemManager does not support Blynk elements
	void saveSettings(bool ignore_flag = false);
	void readSettings();

	TimeManager time;
	SensorsManager sensors;
	SolarSystemManager solar;
	DisplayManager display;
	NetworkManager network;
	BlynkManager blynk;
	static Encoder enc;
	
	bool buzzer_flag;

	bool save_settings_request;
	uint32_t save_settings_timer;
};

template <class T1, class T2, class T3, class T4>
T1 smartIncr(T1& value, T2 incr_step, T3 min, T4 max) {
	if (!incr_step) {
		return value;
	}

	if ((value == min && incr_step < 0) || (value == max && incr_step > 0)) {
		return value;
	}
	
	value += (T1) incr_step;
	value = (T1) constrain(value, min, max);
	
	return value;
}

template <class T>
bool windowCursorTick(T& cursor, int8_t direct, uint8_t cursor_max) {
	if (direct < 0) {
		smartIncr(cursor, -1, 0, cursor_max);

		if (cursor % 4 == 3) {
			return true;
		}
	}

	else if (direct > 0) {
		smartIncr(cursor, 1, 0, cursor_max);

		if (!(cursor % 4)) {
			return true;
		}
	}

	return false;
}