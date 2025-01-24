/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.0
 * Date: 25.01.2025
 */

#pragma once

/* --- Macroces --- */
/* DisplayManager */
#define DISPLAY_AUTO_RESET_TIME 30 // min

/* SettingsWindow */
#define SCREEN_EXIT_BUZZER_FREQ 200
#define SCREEN_EXIT_BUZZER_TIME 300 // mls

/* MainWindow */
#define SOLAR_TICK_POINTER_TIME 500 // mls

/* SetDS18B20AddressWindow */
#define DS18B20_START_PRINT_BYTE 4 // byte [0 - 7]


class LcdManager : public LiquidCrystal_I2C {
public:
	LcdManager();

	void printTitle(uint8_t y, String title, uint16_t delay_time = 800, bool clear_flag = true);
	void easyPrint(uint8_t x, uint8_t y, String string);
	void easyPrint(uint8_t x, uint8_t y, int32_t number);
	void easyPrint(uint8_t x, uint8_t y, float number);
	void easyWrite(uint8_t x, uint8_t y, uint8_t code);
	void clearLine(uint8_t line);
	void clearColumn(uint8_t column);
};


class Window;
struct window_list_node_t {
	Window* window;
	window_list_node_t* next;		
};


class DisplayManager {
public: 
	DisplayManager();
	~DisplayManager();
	void begin();
	
	void tick();
	void makeDefault();
	void writeSettings(char* buffer);
	void readSettings(char* buffer);
#ifdef DISPLAY_MANAGER_BLYNK_SUPPORT
	void addBlynkElementCodes(DynamicArray<String>* array);
	bool blynkElementSend(BlynkWifi* Blynk, blynk_link_t* link);
	bool blynkElementParse(String code, const BlynkParam& param);
#endif

	bool action();
	void addWindowToStack(Window* window);
	void deleteWindowFromStack(Window* window);

	void setSystemManager(SystemManager* system);

	void setWorkFlag(bool work_flag);
	void setAutoResetFlag(bool auto_reset_flag);
	void setBacklightOffTime(uint8_t time);
	void setFps(uint8_t fps);

	SystemManager* getSystemManager();
	LcdManager* getLcdManager();
	Window* getWindowFromStack();

	bool getWorkFlag();
	bool getAutoResetFlag();
	uint8_t getBacklightOffTime();
	uint8_t getFps();

private:
	void freeStack();

	SystemManager* system;
	LcdManager lcd;

	window_list_node_t* stack;

	bool work_flag;
	bool auto_reset_flag;
	uint8_t backlight_off_time;
	uint8_t fps;

	uint32_t auto_reset_timer;
	uint32_t backlight_off_timer;
	uint32_t fps_timer;
	bool backlight_flag;
};


class Window {
public:
	virtual void print(LcdManager* lcd, DisplayManager* display, SystemManager* system) = 0;
};

class MainWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	void printHome(LcdManager* lcd, SystemManager* system);
	void printSensors(LcdManager* lcd, SystemManager* system);
	void printSolar(LcdManager* lcd, SystemManager* system);
	void printWifi(LcdManager* lcd, SystemManager* system);
	void printAp(LcdManager* lcd, SystemManager* system);
	void printBlynk(LcdManager* lcd, SystemManager* system);

	void printDigit(LcdManager* lcd, uint8_t x, uint8_t y, uint8_t digit);
	void makeSymbols(LcdManager* lcd);

	struct solar_window_data_t {
		uint8_t pointer = 0;
		uint32_t pointer_tick_timer = 0;

	} solar_window_data;

	bool create_symbol_flag = true;
	uint8_t cursor = 0;
};

class DS18B20Window : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	uint8_t cursor = 0;
};

class SettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;
};

class NetworkSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_flag = true;
	uint8_t cursor = 0;

	char ssid_ap_to_set[NETWORK_SSID_PASS_SIZE] = "";
	char pass_ap_to_set[NETWORK_SSID_PASS_SIZE] = "";
};

class WifiSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool initialization_flag = true;
	
	bool print_flag = true;
	uint8_t cursor = 0;

	char ssid_to_set[NETWORK_SSID_PASS_SIZE] = "";
	char pass_to_set[NETWORK_SSID_PASS_SIZE] = "";
};

class BlynkSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_flag = true;
	uint8_t cursor = 0;
};

class BlynkLinksSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool scan_flag = true;
	bool scan_element_index_flag = true;
	bool print_flag = true;
	bool value_cursor = 0;
	uint8_t element_index = 0;
	uint8_t cursor = 0;

	DynamicArray<String> element_codes;
};

class SolarSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_flag = true;
	uint8_t cursor = 0;
};

class SystemSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_flag = true;
	uint8_t cursor = 0;
};

class TimeSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_flag = true;
	uint8_t cursor = 0;

	TimeT* time_to_set = NULL;
};

class DS18B20SensorsSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_flag = true;
	uint8_t cursor = 0;
	uint32_t update_timer = 0;
	
	ds18b20_data_t* ds18b20_to_set = NULL;
};

class SetTimeWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setTimeT(TimeT* time);

private:
	bool create_symbol_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;

	TimeT* config_time = NULL;
};

class SetDS18B20Window : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setDS18B20(ds18b20_data_t* ds18b20);

private:
	bool print_flag = true;
	uint8_t cursor = 0;
	uint32_t update_timer = 0;

	ds18b20_data_t* config_ds18b20 = NULL;
};

class SetDS18B20AddressWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setArray(uint8_t* address);

private:
	bool print_flag = true;
	bool scan_flag = true;
	uint8_t cursor = 0;

	DynamicArray<DeviceAddress> ds18b20_addresses;
	DynamicArray<float> t_array;

	uint8_t* config_address;
};

class SetWifiStationWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setString(char* string, uint8_t size);

private:
	bool print_flag = true;
	bool scan_flag = true;
	uint8_t cursor = 0;
	uint8_t stations_count = 0;

	char* config_string;
	uint8_t string_size;
};

class KeyboardWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setString(char* string, uint8_t size);

private:
	bool create_symbol_flag = true;
	bool print_key_flag = true;
	bool print_string_flag = true;
	bool caps = false;
	uint8_t cursor = 0;
	uint8_t key_cursor = 0;
	uint8_t string_size_now = 0;

	char* config_string;
	uint8_t string_size;
};