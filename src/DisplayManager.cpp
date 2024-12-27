/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.2.0
 * Date: 27.12.2024
 */

#include "data.h"

DisplayManager::DisplayManager() {
	makeDefault();
}

void DisplayManager::begin() {
	lcd.init();
	lcd.backlight();
}


void DisplayManager::tick() {
	if (!getWorkFlag()) {
		return;
	}
	
	if (getAutoResetFlag() && millis() - auto_reset_timer >= MIN_TO_MLS(DISPLAY_AUTO_RESET_TIME)) {
		auto_reset_timer = millis();
		lcd.init();
	}
	
	if (getBacklightOffTime() && millis() - backlight_off_timer >= SEC_TO_MLS(getBacklightOffTime()) ) {
		if (backlight_flag) {
			backlight_flag = false;
			lcd.noBacklight();
		}
	}
	
	if (!backlight_flag) {
		return;
	}
	
	Window* window = getWindowFromStack();
	if (window == NULL) {
		return;
	}

	if (millis() - fps_timer >= 1000 / getFps()) {
		fps_timer = millis();
		
		window->print(getLcdManager(), this, getSystemManager());
	}
}

void DisplayManager::makeDefault() {
	system = NULL;
	freeStack();
	addWindowToStack(new MainWindow);

	work_flag = DEFAULT_DISPLAY_WORK_FLAG;
	auto_reset_flag = DEFAULT_DISPLAY_AUTO_RESET_FLAG;
	backlight_off_time = DEFAULT_DISPLAY_BACKLIGHT_OFF_TIME;
	fps = DEFAULT_DISPLAY_FPS;

	backlight_off_timer = 0;
	fps_timer = 0;
	backlight_flag = true;
}

void DisplayManager::writeSettings(char* buffer) {
	setParameter(buffer, "SDar", getAutoResetFlag());
	setParameter(buffer, "SDbot", getBacklightOffTime());
	setParameter(buffer, "SDf", getFps());
}

void DisplayManager::readSettings(char* buffer) {
	getParameter(buffer, "SDar", &auto_reset_flag);
	getParameter(buffer, "SDbot", &backlight_off_time);
	getParameter(buffer, "SDf", &fps);
}

void DisplayManager::addBlynkElements(BlynkManager* blynk) {
	blynk->addElement("P dspl at rs", "SDar", &auto_reset_flag, BLYNK_TYPE_BOOL);
	blynk->addElement("P dspl time", "SDbot", &backlight_off_time, BLYNK_TYPE_UINT8_T);
	blynk->addElement("P dspl fps", "SDf", &fps, BLYNK_TYPE_UINT8_T);
}


bool DisplayManager::action() {
	backlight_off_timer = millis();

	if (!backlight_flag) {
		backlight_flag = true;
		lcd.backlight();

		return true;
	}

	return false;
}

void DisplayManager::addWindowToStack(Window* window) {
	if (window == NULL) {
		return;
	}

	window_list_node_t* new_node = new window_list_node_t;

	new_node->next = stack;
	new_node->window = window;

	stack = new_node;
}

void DisplayManager::deleteWindowFromStack(Window* window) {
	if (stack == NULL || window == NULL) {
		return;
	}

	if (stack->window == window) {
		window_list_node_t* node_to_delete = stack;
		stack = stack->next;

		free(node_to_delete->window);
		free(node_to_delete);
	}
}


void DisplayManager::setSystemManager(SystemManager* system) {
	this->system = system;
}


void DisplayManager::setWorkFlag(bool work_flag) {
	this->work_flag = work_flag;
}

void DisplayManager::setAutoResetFlag(bool auto_reset_flag) {
	this->auto_reset_flag = auto_reset_flag;
}

void DisplayManager::setBacklightOffTime(uint8_t time) {
	backlight_off_time = constrain(time, 0, 255);
}

void DisplayManager::setFps(uint8_t fps) {
	fps = constrain(fps, 1, 255);
}


SystemManager* DisplayManager::getSystemManager() {
	return system;
}

LcdManager* DisplayManager::getLcdManager() {
	return &lcd;
}

Window* DisplayManager::getWindowFromStack() {
	return (stack == NULL) ? NULL : stack->window;
}


bool DisplayManager::getWorkFlag() {
	return work_flag;
}

bool DisplayManager::getAutoResetFlag() {
	return auto_reset_flag;
}

uint8_t DisplayManager::getBacklightOffTime() {
	return backlight_off_time;
}

uint8_t DisplayManager::getFps() {
	return fps;
}


void DisplayManager::freeStack() {
	if (stack == NULL) {
		return;
	}

	do {
		window_list_node_t* node_to_delete = stack;
		stack = stack->next;

		free(node_to_delete->window);
		free(node_to_delete);
	} while (stack != NULL);
}