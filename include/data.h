/*
 * Project: Solar Battery Control System
 * 
 * Description:
 * This project is designed to manage a physical solar system which consists of a water tank, a solar battery, 
 * and a pipe connecting the solar battery and the tank, through which fluid circulates. 
 * The water temperature in the tank is measured by a sensor. Another sensor is attached to the pipe at the tank's outlet, 
 * measuring the fluid temperature after it exits the tank. A third sensor is attached to the pipe after the solar battery, 
 * measuring the fluid temperature after it passes through the solar battery. The system includes a pump that circulates the fluid through the pipe.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Version: 1.0.0
 * Date: 02.09.2024
 * 
 * Features:
 * 1. Supports reading up to 4 ds18b20 temperature sensors with calibration capability.
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
#include <LittleFS.h>
#include <GyverEncoder.h>
#include <LiquidCrystal_I2C.h> 
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverPortal.h>

#define NO_GLOBAL_BLYNK
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

// ports
#define DS18B20_PORT D3
#define BUZZER_PORT D8
#define CLK_PORT D6
#define DT_PORT D5
#define SW_PORT D4
#define RELE_PORT D7

// defaults
#define DEFAULT_READ_DATA_TIME 5
#define DEFAULT_DISPLAY_OFF_TIME 5
#define DEFAULT_DISPLAY_FPS 6
#define DEFAULT_BUZZER_STATUS true
#define DEFAULT_RELE_INVERT true

#define DEFAULT_GMT 0
#define DEFAULT_NTP_SYNC true

#define DEFAULT_SOLAR_STATUS true
#define DEFAULT_SOLAR_ON_IF_PROBLEM true
#define DEFAULT_SOLAR_DELTA 5

#define DEFAULT_NETWORK_MODE NETWORK_AUTO
#define DEFAULT_NETWORK_SSID_AP "nztr_solar"
#define DEFAULT_NETWORK_PASS_AP "nazotronic"
#define DEFAULT_BLYNK_WORK_STATUS true

// macroces
#define DS_SENSORS_COUNT 4
#define SOLAR_DELTA_MIN 3
#define SOLAR_DELTA_MAX 10
#define PROBLEMS_COUNT 2
#define BLINK_TIME 1000
#define EXIT_TONE_DURATION 300

#define SETTINGS_BUFFER_SIZE 900
#define SAVE_SETTINGS_TIME 5000

#define NETWORK_OFF 0
#define NETWORK_STA 1
#define NETWORK_AP_STA 2
#define NETWORK_AUTO 3
#define NETWORK_SSID_PASS_SIZE 15
#define NETWORK_RECONNECT_TIME 20000
#define WEB_REBUILD_TIME 500
#define WEB_UPDATE_TIME 3000

#define NTP_SYNC_TIME 60000
#define UDP_RESEND_TIME 5000

#define NTP_SERVER "time.nist.gov"
#define NTP_PORT 123

#define BLYNK_TYPE_UINT8_T 0
#define BLYNK_TYPE_INT8_T 1
#define BLYNK_TYPE_UINT32_T 2
#define BLYNK_TYPE_INT32_T 3
#define BLYNK_TYPE_BOOL 4
#define BLYNK_TYPE_FLOAT 5
#define BLYNK_LINKS_MAX 20
#define BLYNK_AUTH_SIZE 35
#define BLYNK_ELEMENT_CODE_SIZE 10

struct sensor_t {
	char name[3];
	uint8_t address[8];
	bool problem;
	
	float t;
	float correction;
};

struct blynk_element_t {
	const char* name;
	char code[BLYNK_ELEMENT_CODE_SIZE];
	void* pointer;
	uint8_t type;
};

struct blynk_link_t {
	uint8_t port;
	uint8_t element_index;
};

class Solar {
public:
	Solar();
	
	/**
	 * @brief Make settings default.
	 */
	void makeDefault();

	/**
	 * @brief Save settings of this class.
	 * 
	 * @param[out] buffer
	 * Buffer for saving settings.
	 */
	void saveSettings(char* buffer);

	/**
	 * @brief Read settings of this class.
	 * 
	 * @param[in] buffer
	 * Buffer for reading  settings.
	 */
	void readSettings(char* buffer);

	/**
	 * @brief Add elements of this class to Blynk.
	 */
	void addBlynkElements();

	/**
	 * @brief Solar control function.
	 */
	void tick();

	/**
	 * @brief Solar system status return.
	 * 
	 * @return Solar system status.
	 */
	uint8_t status();


	/**
	 * @brief Set work status flag.
	 * 
	 * @param[in] status
	 * The work status flag that will be set.
	 */
	void setWorkStatus(bool status);

	/**
	 * @brief Set on_if_problem flag.
	 * 
	 * @param[in] on_if_problem
	 * The on_if_problem flag that will be set.
	 */
	void setOnIfProblem(bool on_if_problem);

	/**
	 * @brief Set delta value.
	 * 
	 * @param[in] delta
	 * The delta value that will be set.
	 */
	void setDelta(uint8_t delta);

	/**
	 * @brief Set solar system sensors.
	 * 
	 * @param[in] solar_sensor
	 * Which sensor must be set.
	 * 
	 * @param[in] ds_sensor_index
	 * Index of the ds18b20 sensor that must be set.
	 */
	void setSensor(uint8_t solar_sensor, int8_t ds_sensor_index);
	
	/**
	 * @brief Set the sensor on the solar battery.
	 * 
	 * @param[in] ds_sensor_index
	 * Index of the ds18b20 sensor that must be set.
	 */
	void setBatterySensor(int8_t ds_sensor_index);

	/**
	 * @brief Set the sensor in the boiler.
	 * 
	 * @param[in] ds_sensor_index
	 * Index of the ds18b20 sensor that must be set.
	 */
	void setBoilerSensor(int8_t ds_sensor_index);

	/**
	 * @brief Set the sensor at the outlet of the boiler.
	 * 
	 * @param[in] ds_sensor_index
	 * Index of the ds18b20 sensor that must be set.
	 */
	void setExitSensor(int8_t ds_sensor_index);


	/**
	 * @brief Get work status flag.
	 * 
	 * @return Work status flag.
	 */
	bool getWorkStatus();

	/**
	 * @brief Get On_if_problem flag.
	 * 
	 * @return On_if_problem flag.
	 */
	bool getOnIfProblem();

	/**
	 * @brief Get Delta value.
	 * 
	 * @return Delta value.
	 */
	uint8_t getDelta();

	/**
	 * @brief Get index of the ds18b20 sensor for a certain sensor.
	 * 
	 * @param[in] solar_sensor
	 * Sensor whose index must be returned.
	 * 
	 * @return Index of the ds18b20 sensor.
	 */
	int8_t getSensor(uint8_t solar_sensor);

	/**
	 * @brief Get index of the ds18b20 sensor for a solar battery sensor.
	 * 
	 * @return Index of the ds18b20 sensor.
	 */
	int8_t getBatterySensor();

	/**
	 * @brief Get index of the ds18b20 sensor for a boiler sensor.
	 * 
	 * @return Index of the ds18b20 sensor.
	 */
	int8_t getBoilerSensor();

	/**
	 * @brief Get index of the ds18b20 sensor for at the outlet of the boiler sensor.
	 * 
	 * @return Index of the ds18b20 sensor.
	 */
	int8_t getExitSensor();
	

	/**
	 * @brief Get solar battery temperature.
	 * 
	 * @return Solar battery temperature.
	 */
	float getBatteryT();

	/**
	 * @brief Get boiler temperature.
	 * 
	 * @return Boiler temperature.
	 */
	float getBoilerT();

	/**
	 * @brief Get exit temperature.
	 * 
	 * @return Exit temperature.
	 */
	float getExitT();


	/**
	 * @brief Get solar battery sensor status.
	 * 
	 * @return Solar battery sensor status.
	 */
	uint8_t batterySensor();

	/**
	 * @brief Get boiler sensor status.
	 * 
	 * @return Boiler sensor status.
	 */
	uint8_t boilerSensor();

	/**
	 * @brief Get exit sensor status.
	 * 
	 * @return Exit sensor status.
	 */
	uint8_t exitSensor();

private:
	int8_t battery_sensor;
	int8_t boiler_sensor;
	int8_t exit_sensor;
	
	bool work_status;
	bool on_if_problem;
	uint8_t delta;
};

class Display {
public: 
	Display();

	/**
	 * @brief Make settings default.
	 */
	void makeDefault();

	/**
	 * @brief Save settings of this class.
	 * 
	 * @param[out] buffer
	 * Buffer for saving settings.
	 */
	void saveSettings(char* buffer);

	/**
	 * @brief Read settings of this class.
	 * 
	 * @param[in] buffer
	 * Buffer for reading settings.
	 */
	void readSettings(char* buffer);

	/**
	 * @brief Add elements of this class to Blynk.
	 */
	void addBlynkElements();

	/**
	 * @brief Display control function.
	 */
	void tick();

	/**
	 * @brief Call when an external action occurs.
	 */
	bool action();


	/**
	 * @brief Set display_off_time value.
	 * 
	 * @param[in] time
	 * The display_off_time value that will be set.
	 */
	void setDisplayOffTime(uint8_t time);

	/**
	 * @brief Set display_fps value.
	 * 
	 * @param[in] fps
	 * The display_fps value that will be set.
	 */
	void setDisplayFps(uint8_t fps);

	/**
	 * @brief Get display_off_time value.
	 * 
	 * @return Display_off_time value.
	 */
	uint8_t getDisplayOffTime();

	/**
	 * @brief Get display_fps value.
	 * 
	 * @return Display_fps value.
	 */
	uint8_t getDisplayFps();

private:
	/**
	 * @brief Displays the main interface of the system.
	 */
	void mainDisplay();

	/**
	 * @brief Displays the settings menu.
	 */
	void settingsDisplay();

	/**
	 * @brief Displays network settings.
	 */
	void networkSettingsDisplay();

	/**
	 * @brief Displays Wi-Fi settings.
	 */
	void wifiSettingsDisplay();

	/**
	 * @brief Displays Blynk settings.
	 */
	void blynkSettingsDisplay();

	/**
	 * @brief Displays Blynk links settings.
	 */
	void blynkLinksSettingsDisplay();

	/**
	 * @brief Displays solar system settings.
	 */
	void solarSettingsDisplay();

	/**
	 * @brief Displays system settings.
	 */
	void systemSettingsDisplay();

	/**
	 * @brief Displays time settings.
	 */
	void timeSettingsDisplay();

	/**
	 * @brief Displays ds18b20 settings.
	 */
	void dsSettingsDisplay();


	void (Display::*screen)();
	bool backlight_status;

	uint8_t display_off_time;
	uint8_t display_fps;

	uint32_t display_off_timer;
	uint32_t display_fps_timer;

	bool tick_allow;
};

class Network {
public:
	Network();

	/**
	 * @brief Make settings default.
	 */
	void makeDefault();

	/**
	 * @brief Save settings of this class.
	 * 
	 * @param[out] buffer
	 * Buffer for saving settings.
	 */
	void saveSettings(char* buffer);

	/**
	 * @brief Read settings of this class.
	 * 
	 * @param[in] buffer
	 * Buffer for reading  settings.
	 */
	void readSettings(char* buffer);

	/**
	 * @brief Network control function.
	 */
	void tick();

	/**
	 * @brief Turn off wifi and access point.
	 */
	void off();
	
	/**
	 * @brief Connect to Wi-Fi.
	 * If the (ssid) and/or (pass) parameters are set,
	 * an attempt will be made to connect to the (ssid) network for the duration of (connect_time),
	 * and if successful, this network will be saved (auto_save).
	 * 
	 * @param[in] ssid
	 * Name of the network to connect to.
	 * 
	 * @param[in] pass
	 * Pass to the network to connect to.
	 * 
	 * @param[in] connect_time
	 * Wi-Fi connection duration.
	 * 
	 * @param[in] auto_save
	 * Auto save network flag, if сonnection will be successful.
	 * 
	 * @return connection result.
	 */
	bool connect(String ssid = String(""), String pass = String(""), uint8_t connect_time = 0, bool auto_save = false);
	
	/**
	 * @brief WiFi status return.
	 * 
	 * @return WiFi status.
	 */
	wl_status_t status();


	/**
	 * @brief Set network mode.
	 * 
	 * @param[in] mode
	 * Network mode that will be set.
	 */
	void setMode(uint8_t mode);

	/**
	 * @brief Set Wi-Fi ssid & pass.
	 * 
	 * @param[in] ssid
	 * Wi-Fi ssid that will be set.
	 * @param[in] pass
	 * Wi-Fi pass that will be set.
	 */
	void setWifi(String* ssid, String* pass);

	/**
	 * @brief Set Wi-Fi ssid & pass.
	 * 
	 * @param[in] ssid
	 * Wi-Fi ssid that will be set.
	 * @param[in] pass
	 * Wi-Fi pass that will be set.
	 */
	void setWifi(const char* ssid, const char* pass);

	/**
	 * @brief Set AP ssid & pass.
	 * 
	 * @param[in] ssid
	 * AP ssid that will be set.
	 * @param[in] pass
	 * AP pass that will be set.
	 */
	void setAp(String* ssid, String* pass);

	/**
	 * @brief Set AP ssid & pass.
	 * 
	 * @param[in] ssid
	 * AP ssid that will be set.
	 * @param[in] pass
	 * AP pass that will be set.
	 */
	void setAp(const char* ssid, const char* pass);


	/**
	 * @brief Get network mode.
	 * 
	 * @return Network mode.
	 */
	uint8_t getMode();

	/**
	 * @brief Get Wi-Fi ssid.
	 * 
	 * @return Wi-Fi ssid.
	 */
	char* getWifiSsid();

	/**
	 * @brief Get Wi-Fi pass.
	 * 
	 * @return Wi-Fi pass.
	 */
	char* getWifiPass();

	/**
	 * @brief Get AP ssid.
	 * 
	 * @return AP ssid.
	 */
	char* getApSsid();

	/**
	 * @brief Get AP pass.
	 * 
	 * @return AP pass.
	 */
	char* getApPass();


	/**
	 * @brief Get working Wi-Fi status.
	 * 
	 * @return Working Wi-Fi status.
	 */
	bool isWifi();

	/**
	 * @brief Get working AP status.
	 * 
	 * @return Working AP status.
	 */
	bool isAp();

private:
	char ssid_sta[NETWORK_SSID_PASS_SIZE];
	char pass_sta[NETWORK_SSID_PASS_SIZE];

	char ssid_ap[NETWORK_SSID_PASS_SIZE];
	char pass_ap[NETWORK_SSID_PASS_SIZE];

	uint8_t mode;

	bool reset_request;
	bool tick_allow;
	uint32_t reconnect_timer;
};

class BlynkManage {
public:
	BlynkManage();
	~BlynkManage();
	/**
	 * @brief Make settings default.
	 */
	void makeDefault();

	/**
	 * @brief Save settings of this class.
	 * 
	 * @param[out] buffer
	 * Buffer for saving settings.
	 */
	void saveSettings(char* buffer);

	/**
	 * @brief Read settings of this class.
	 * 
	 * @param[in] buffer
	 * Buffer for reading  settings.
	 */
	void readSettings(char* buffer);

	/**
	 * @brief Network control function.
	 */
	void tick();

	friend BLYNK_WRITE_DEFAULT();

	/**
	 * @brief Return Blynk connection status.
	 * 
	 * @return Blynk connection status.
	 */
	bool status();

	/**
	 * @brief Add a variable for processing on virtual ports.
	 * 
	 * @param[in] name
	 * Name of this variable to display when selected.
	 * 
	 * @param[in] code
	 * Unique code of this variable.
	 * 
	 * @param[in] pointer
	 * Pointer to this variable.
	 * 
	 * @param[in] type
	 * Type of this variable.
	 * 
	 * @return adding success.
	 */
	bool addElement(const char* name, String code, void* pointer, uint8_t type);
	
	/**
	 * @brief Add a link connecting the virtual port and the variable.
	 * 
	 * @return adding success.
	 */
	bool addLink();

	/**
	 * @brief Delete a link.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @return delete success.
	 */
	bool delLink(uint8_t index);

	/**
	 * @brief Set work status flag.
	 * 
	 * @param[in] status
	 * The work status flag that will be set.
	 */
	void setWorkStatus(bool status);

	/**
	 * @brief Set authorization code.
	 * 
	 * @param[in] auth
	 * Authorization code to Blynk Device.
	 */
	void setAuth(String auth);

	/**
	 * @brief Set link virtual port.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @param[in] port
	 * Virtual Blynk port.
	 */
	void setLinkPort(uint8_t index, uint8_t port);

	/**
	 * @brief Set link element by it index.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @param[in] element_index
	 * Element index in this class.
	 */
	void setLinkElement(uint8_t index, uint8_t element_index);

	/**
	 * @brief Set link element by it unique code.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @param[in] element_index
	 * Unique element code in this class.
	 */
	void setLinkElement(uint8_t index, const char* element_code);

	/**
	 * @brief Get work status flag.
	 * 
	 * @return Work status flag.
	 */
	bool getWorkStatus();

	/**
	 * @brief Get authorization code.
	 * 
	 * @return Authorization code.
	 */
	char* getAuth();

	/**
	 * @brief Get links count in this class.
	 * 
	 * @return Links count.
	 */
	uint8_t getLinksCount();

	/**
	 * @brief Get elements count in this class.
	 * 
	 * @return Elements count.
	 */
	uint8_t getElementsCount();

	/**
	 * @brief Get link port.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @return Link port.
	 */
	uint8_t getLinkPort(uint8_t index);

	/**
	 * @brief Get link element index.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @return Element index.
	 */
	uint8_t getLinkElement(uint8_t index);

	/**
	 * @brief Get element name.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @return Element name.
	 */
	const char* getElementName(uint8_t index);

	/**
	 * @brief Get element code.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @return Element code.
	 */
	char* getElementCode(uint8_t index);

	/**
	 * @brief Get element pointer.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @return Element pointer.
	 */
	void* getElementPointer(uint8_t index);

	/**
	 * @brief Get element type.
	 * 
	 * @param[in] index
	 * Link index in this class.
	 * 
	 * @return Element type.
	 */
	uint8_t getElementType(uint8_t index);

private:
	/**
	 * @brief Send value to the Blynk virtual port.
	 * 
	 * @param[in] link_index
	 * Link index in this class.
	 */
	void send(uint8_t link_index);

	/**
	 * @brief Set element variable by it pointer.
	 * 
	 * @param[in] index
	 * Element index in this class.
	 * 
	 * @param[in] value
	 * Value that will be set to this variable.
	 */
	template <class T>
	void setElementValue(uint8_t index, T value);

	blynk_element_t* elements;
	blynk_link_t* links;

	bool work_status;
	char auth[BLYNK_AUTH_SIZE];

	uint8_t elements_count;
	uint8_t links_count;
};

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

void clkInterrupt();
void dtInterrupt();
void swInterrupt();

/**
 * @brief 
 * Increments a value within a specified range.
 * 
 * @param[in,out] value 
 * The value to be incremented.
 * 
 * @param[in] incr_step 
 * The increment step.
 * 
 * @param[in] min 
 * The minimum of the value.
 * 
 * @param[in] max 
 * The maximum of the value.
 * 
 * @return 
 * The incremented value constrained within the specified range.
 */
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

/**
 * @brief 
 * Gets the next DS18B20 sensor address.
 *
 * @param[in,out] address 
 * A pointer to the current sensor address.
 * 
 * @param[in] step 
 * The step value indicating how many positions to move from the current address.
 */
void getNextSensor(uint8_t* address, int8_t step);



/**
 * @brief 
 * Make settings default.
 */
void makeDefault();

/**
 * @brief 
 * Save settings to the memory.
 */
void saveSettings();

/**
 * @brief 
 * Read settings from the memory.
 */
void readSettings();

/**
 * @brief 
 * Resets all settings.
 */
void resetAll();


/**
 * @brief 
 * Main system function.
 */
void systemTick();

/**
 * @brief 
 * Manages timer events.
 */
void timers();

/**
 * @brief 
 * Reads sensor data.
 */
void readSensors();

/**
 * @brief 
 * Sets the status of the relay.
 * 
 * @param[in] status
 * Set status of the relay (true for on, false for off).
 */
void setRele(bool status);

/**
 * @brief 
 * Gets the current status of the relay.
 *
 * @return 
 * The current status of the relay (true if on, false if off).
 */
bool getRele();

/**
 * @brief 
 * Handles the relay tick event.
 */
void releTick();


/**
 * @brief 
 * Displays a keyboard interface.
 *
 * @param[in,out] string
 * String to fill.
 * 
 * @param[in] size
 * Size of the string.
 */
void displayKeyboard(char* string, byte size);

/**
 * @brief 
 * Displays set time interface.
 */
void displaySetTime();

/**
 * @brief 
 * Creates custom display symbols.
 */
void displayMakeSymbols();

/**
 * @brief 
 * Updates cursor position on the display.
 *
 * @param[in] direct
 * The direction to move the cursor (-1 for left, 1 for right).
 * 
 * @param[in,out] cursor
 * Pointer to the cursor position variable.
 * 
 * @param[in] cursor_max
 * The maximum value for the cursor.
 * 
 * @param[in,out] page
 * Optional pointer to the page variable.
 * 
 * @param[in] page_max
 * Optional maximum value for the page.
 * 
 * @return 
 * true if the cursor sets to 0 or 3.
 */
template <class T>
bool displayCursorTick(int8_t direct, T* cursor, uint8_t cursor_max, T* page = NULL, uint8_t page_max = 0);

/**
 * @brief 
 * Prints a title on the display.
 *
 * @param[in] x 
 * The x-coordinate of the title.
 * 
 * @param[in] y 
 * The y-coordinate of the title.
 * 
 * @param[in] title 
 * Title text to be displayed.
 * 
 * @param[in] delay_time 
 * Delay time for the title display.
 */
void printTitle(uint8_t x, uint8_t y, String title, uint16_t delay_time = 200);

/**
 * @brief 
 * Prints a digit on the display.
 *
 * @param[in] digit 
 * The digit to be displayed.
 * 
 * @param[in] x 
 * The x-coordinate of the digit.
 * 
 * @param[in] y 
 * The y-coordinate of the digit.
 */
void printDigit(uint8_t digit, uint8_t x, uint8_t y);

/**
 * @brief 
 * Prints a string on the display.
 *
 * @param[in] x 
 * The x-coordinate of the string.
 * 
 * @param[in] y 
 * The y-coordinate of the string.
 * 
 * @param[in] string 
 * The string to be displayed.
 */
void easyPrint(uint8_t x, uint8_t y, String string);

/**
 * @brief 
 * Prints an integer number on the display.
 *
 * @param[in] x 
 * The x-coordinate of the number.
 * 
 * @param[in] y 
 * The y-coordinate of the number.
 * 
 * @param[in] number 
 * The integer number to be displayed.
 */
void easyPrint(uint8_t x, uint8_t y, int32_t number);

/**
 * @brief 
 * Prints a floating-point number on the display.
 *
 * @param[in] x 
 * The x-coordinate of the number.
 * 
 * @param[in] y 
 * The y-coordinate of the number.
 * 
 * @param[in] number 
 * The floating-point number to be displayed.
 */
void easyPrint(uint8_t x, uint8_t y, float number);

/**
 * @brief 
 * Writes a specific code to the display.
 *
 * @param[in] x 
 * The x-coordinate.
 * 
 * @param[in] y 
 * The y-coordinate.
 * 
 * @param[in] code 
 * The code to be written.
 */
void easyWrite(uint8_t x, uint8_t y, uint8_t code);

/**
 * @brief 
 * Clears a specific line on the display.
 *
 * @param[in] line 
 * The line number to be cleared.
 */
void clearLine(uint8_t line);

/**
 * @brief 
 * Clears a specific column on the display.
 *
 * @param[in] column 
 * The column number to be cleared.
 */
void clearColumn(uint8_t column);


/**
 * @brief 
 * Builds the web interface.
 */
void uiBuild();

/**
 * @brief 
 * Processes events from the web interface.
 */
void uiAction();

/**
 * @brief 
 * Attempts to synchronize time using NTP.
 *
 * @return 
 * True if the synchronization was successful, false otherwise.
 */
bool ntpSync();


extern uint8_t read_data_time;
extern bool buzzer_status;
extern bool rele_invert;
extern int8_t gmt;
extern bool ntp_sync;

extern bool problem[PROBLEMS_COUNT];
extern sensor_t ds_sensors[DS_SENSORS_COUNT];
extern float t_home;
extern float h_home;

extern uint32_t read_data_timer;
extern uint32_t blink_timer;
extern uint32_t ntp_sync_timer;
extern uint32_t save_settings_timer;

extern bool rele_flag;
extern bool blink_flag;
extern bool save_settings_flag;

extern Solar solar;
extern Display display;
extern Network network;
extern BlynkManage blynk;

extern DallasTemperature sensor;
extern OneWire oneWire;
extern AM2320 am;
extern Clock clk;
extern Encoder enc;
extern LiquidCrystal_I2C lcd;
extern WiFiUDP udp;
extern GyverPortal ui;