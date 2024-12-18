/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.1.0
 * Date: 12.12.2024
 */

#pragma once

/* --- Macroces --- */
#define SOLAR_TICK_POINTER_TIME 500 // mls
#define SCREEN_EXIT_BUZZER_FREQ 200
#define SCREEN_EXIT_BUZZER_TIME 300 // mls


class LcdManager : public LiquidCrystal_I2C {
public:
	LcdManager();

	void printTitle(uint8_t y, String title, uint16_t delay_time = 800);
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
	void begin();
	
	void tick();
	void makeDefault();
	void writeSettings(char* buffer);
	void readSettings(char* buffer);
	void addBlynkElements(BlynkManager* blynk);

	bool action();
	void addWindowToStack(Window* window);
	void deleteWindowFromStack(Window* window);

	void setSystemManager(SystemManager* system);

	void setWorkFlag(bool work_flag);
	void setBacklightOffTime(uint8_t time);
	void setFps(uint8_t fps);

	SystemManager* getSystemManager();
	LcdManager* getLcdManager();
	Window* getWindowFromStack();

	bool getWorkFlag();
	uint8_t getBacklightOffTime();
	uint8_t getFps();

private:
	void freeStack();

	SystemManager* system;
	LcdManager lcd;

	window_list_node_t* stack;

	bool work_flag;
	uint8_t backlight_off_time;
	uint8_t display_fps;

	uint32_t backlight_off_timer;
	uint32_t display_fps_timer;
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
	void printModules(LcdManager* lcd, SystemManager* system);
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
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;

	char ssid_ap[NETWORK_SSID_PASS_SIZE] = "";
	char pass_ap[NETWORK_SSID_PASS_SIZE] = "";
};

class WifiSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool initialization_flag = true;
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;

	char ssid[NETWORK_SSID_PASS_SIZE] = "";
	char pass[NETWORK_SSID_PASS_SIZE] = "";
};

class BlynkSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;

};

class BlynkLinksSettingsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	bool value_cursor = 0;
	uint8_t cursor = 0;
};

class SolarSettingsDisplay : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;
};

class SystemSettingsDisplay : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;
};

class TimeSettingsDisplay : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;

	TimeT* time_to_set = NULL;
};

class DS18B20SettingsDisplay : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;
	uint32_t update_timer = 0;
	
	ds18b20_data_t* ds18b20_to_set = NULL;
};

class TimeSetDisplay : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setTimeT(TimeT* time);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 0;

	TimeT* time = NULL;
};

class DS18B20SetDisplay : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setDS18B20(ds18b20_data_t* ds18b20);

private:
	bool print_title_flag = true;
	bool print_flag = true;
	uint8_t cursor = 1;
	uint32_t update_timer = 0;

	ds18b20_data_t* ds18b20 = NULL;
};

class DS18B20AddressWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setArray(uint8_t* array, uint8_t size);

private:
	bool print_flag = true;
	bool scan_flag = true;
	uint8_t cursor = 0;
	uint8_t ds18b20_count = 0;
	uint32_t scan_timer = 0;

	uint8_t* array;
	uint8_t size;

};

class WifiStationsWindow : public Window {
public:
	void print(LcdManager* lcd, DisplayManager* display, SystemManager* system);
	void setString(char* string, uint8_t size);

private:
	bool print_flag = true;
	bool scan_flag = true;
	uint8_t cursor = 0;
	uint8_t stations_count = 0;

	char* string;
	uint8_t size;

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

	char* string;
	uint8_t size;
};