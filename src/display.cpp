/*
 * File: display.cpp
 * 
 * Description:
 * This file defines the functions responsible for managing the display.
 * It handles all rendering operations,
 * to show information on the display, ensuring that the user interface is clear and informative.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Date: 02.09.2024
 */

#include "data.h"

Display::Display() {

}

void Display::makeDefault() {
	lcd.init();
	lcd.backlight();
	displayMakeSymbols();

	backlight_status = true;
	screen = &Display::mainDisplay;

	display_off_time = DEFAULT_DISPLAY_OFF_TIME;
	display_fps = DEFAULT_DISPLAY_FPS;

	display_off_timer = 0;
	display_fps_timer = 0;

	tick_allow = true;
}

void Display::saveSettings(char* buffer) {
	setParameter(buffer, "SStds", display_off_time);
	setParameter(buffer, "SSdf", display_fps);
}

void Display::readSettings(char* buffer) {
	getParameter(buffer, "SStds", &display_off_time);
	getParameter(buffer, "SSdf", &display_fps);
}

void Display::addBlynkElements() {
	blynk.addElement("P dspl time", "SStds", &display_off_time, BLYNK_TYPE_UINT8_T);
	blynk.addElement("P dspl fps", "SSdf", &display_fps, BLYNK_TYPE_UINT8_T);
}

void Display::tick() {
	if (!tick_allow) {
		return;
	}

	if (millis() - display_off_timer >= display_off_time * 1000 && display_off_time) {
		if (backlight_status) {
			backlight_status = false;
			lcd.noBacklight();
		}
	}

	if (!backlight_status) {
		return;
	}

	if (millis() - display_fps_timer >= 1000 / display_fps) {
		display_fps_timer = millis();
		(this->*screen)();
	}
}

bool Display::action() {
	display_off_timer = millis();

	if (!backlight_status) {
		backlight_status = true;
		
		lcd.backlight();
		return true;
	}

	return false;
}


void Display::setDisplayOffTime(uint8_t time) {
	display_off_time = time;
}

void Display::setDisplayFps(uint8_t fps) {
	display_fps = constrain(fps, 1, 255);
}


uint8_t Display::getDisplayOffTime() {
	return display_off_time;
}

uint8_t Display::getDisplayFps() {
	return display_fps;
}


void Display::mainDisplay() {
	static uint8_t cursor = 0;
	
	switch(cursor) {
	case 0: {
		uint8_t H = clk.hour(gmt);
		uint8_t M = clk.minute(gmt);

		for (uint8_t i = 0;i < PROBLEMS_COUNT;i++) {
			easyPrint(i, 0, (problem[i]) ? "#" : " "); 
		}
		for (uint8_t i = 0;i < DS_SENSORS_COUNT;i++) {
			easyPrint(PROBLEMS_COUNT + i, 0, (!*ds_sensors[i].address || ds_sensors[i].problem) ? "#" : " "); 
		}

		easyPrint(16, 0, (getRele()) ? ((blink_flag) ? ">" : "<") : " ");
		easyPrint(17, 0, (solar.status() && (solar.status() == 1 || blink_flag) ) ? "!" : " ");
		easyWrite(18, 0, (network.isAp()) ? 178 : 32);
		easyWrite(19, 0, (network.isWifi() && (network.status() == WL_CONNECTED || blink_flag) ) ? 7 : 32);
		
		printDigit(H / 10, 1, 1);
		printDigit(H % 10, 4, 1);
		easyWrite(7, 1, (blink_flag) ? 111 : 32);
		easyWrite(7, 2, (blink_flag) ? 111 : 32);
		printDigit(M / 10, 8, 1);
		printDigit(M % 10, 11, 1);

		easyPrint(15, 2, clk.year(gmt));
		easyPrint(15, 3, String(clk.day(gmt)) + "." + clk.month(gmt));
		}
		break;

	case 1:
		for (byte i = 0;i < 4;i++) {
			easyPrint(0, i, (!ds_sensors[i].problem || blink_flag) ? ds_sensors[i].name : "  ");
			lcd.print(":");
			lcd.print(ds_sensors[i].t, 2);
			lcd.write(223);
		}

		easyPrint(12, 0, (!problem[1] || blink_flag) ? "T" : " ");
		lcd.print(":");
		lcd.print(t_home, 2);
		lcd.write(223);

		easyPrint(12, 1, (!problem[1] || blink_flag) ? "H" : " ");
		lcd.print(":");
		lcd.print(h_home, 2);
		lcd.print("%");
		break;

	case 2: {
		static int8_t pointer;
		static bool last_blink_flag;

		for (byte i = 0;i < 3;i++) {
			easyPrint(1, i, "|");
			easyPrint(5, i, "|");
		}
		if (!solar.boilerSensor() || blink_flag) {
			easyPrint(2, 0, (int8_t)solar.getBoilerT());
			easyPrint(2, 1, String(".") + (int8_t)((solar.getBoilerT() - (int8_t)solar.getBoilerT()) * 100) );
			easyWrite(4, 2, 223);
		}
		else {
			easyPrint(2, 0, "   ");
			easyPrint(2, 1, "   ");
			easyPrint(2, 2, "   ");
		}
		easyPrint(1, 3, "|___|");

		easyPrint(8, 1, (solar.getWorkStatus()) ? "ON " : "OFF");
		if (!solar.exitSensor() || blink_flag) {
			easyPrint(7, 2, solar.getExitT());
			lcd.write(223);
		}
		else {
			easyPrint(7, 2, "       ");
		}

		if (blink_flag != last_blink_flag) {
			easyPrint(11 - pointer, 0, " ");
			easyPrint(6 + pointer, 3, " ");
			
			if (getRele()) {
				pointer = (pointer == 5) ? -1 : pointer;
				smartIncr(pointer, 1, 0, 5);
				
				easyPrint(11 - pointer, 0, "<");
				easyPrint(6 + pointer, 3, ">");
			}

			last_blink_flag = blink_flag;
		}

		easyPrint(12, 0, "-----");
		for (byte i = 0;i < 2;i++) {
			easyPrint(13 + i, 1 + i, "|");
			easyPrint(17 + i, 1 + i, "|");
		}
		easyPrint(15, 3, "-----");

		if (!solar.batterySensor() || blink_flag) {
			easyPrint(14, 1, (int)solar.getBatteryT());
			easyPrint(15, 2, String(".") + (int)((solar.getBatteryT() - (int)solar.getBatteryT()) * 10) );
			lcd.write(223);
		}
		else {
			easyPrint(14, 1, "   ");
			easyPrint(15, 2, "   ");
		}
		}
		break;

	case 3:
		easyPrint(0, 0, "WiFi ");

		if (!network.isWifi()) {
			lcd.print("disabled");
		}
		else {
			switch (network.status()) {
			case WL_NO_SHIELD:
				lcd.print("NO_SHIELD");
				break;
			case WL_IDLE_STATUS:
				lcd.print("IDLE_STATUS");
				break;
			case WL_NO_SSID_AVAIL:
				lcd.print("NO_SSID_AVAIL");
				break;
			case WL_SCAN_COMPLETED:
				lcd.print("SCAN_COMPLETED");
				break;
			case WL_CONNECTED:
				lcd.print("CONNECTED");
				break;
			case WL_CONNECT_FAILED:
				lcd.print("CONNECT_FAILED");
				break;
			case WL_CONNECTION_LOST:
				lcd.print("CONNECTION_LOST");
				break;
			case WL_WRONG_PASSWORD:
				lcd.print("WRONG_PASSWORD");
				break;
			case WL_DISCONNECTED:
				lcd.print("DISCONNECTED");
				break;
			default:
				lcd.print("ERR");
			}
			
			easyPrint(0, 1, network.getWifiSsid());
			lcd.print(" ");
			lcd.print(WiFi.RSSI());

			easyPrint(0, 2, "IP:");
			lcd.print(WiFi.localIP());
		}
		break;

	case 4:
		easyPrint(0, 0, "AP ");

		if (!network.isAp()) {
			lcd.print("disabled");
		}
		else {
			lcd.print(network.getApSsid());
			
			easyPrint(0, 1, WiFi.softAPgetStationNum());
			lcd.print(" devices");

			easyPrint(0, 2, "IP:");
			lcd.print(WiFi.softAPIP());
		}
		break;
	}

	if (enc.isLeft(true) || enc.isRight(true)) {
		lcd.clear();
		displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 4);
		
		enc.isRight();
	}

	if (enc.isClick()) {
		switch (cursor) {
		case 0:
			screen = &Display::settingsDisplay;
			break;
		case 1:
			screen = &Display::dsSettingsDisplay;
			break;
		case 2:
			screen = &Display::solarSettingsDisplay;
			break;
		case 3:
		case 4:
			screen = &Display::networkSettingsDisplay;
			break;
		}

		lcd.clear();
	}
	if (enc.isHolded()) {
		switch (cursor) {
		case 2:
			setRele(!getRele());
			break;
		}
	}
}

void Display::settingsDisplay() {
	static uint8_t cursor;
	static bool print_flag = true;
	
	if (print_flag) {
		printTitle(8, 1, "Menu");

		easyPrint(1, 0, "Network       [ok]");
		easyPrint(1, 1, "Blynk         [ok]");
		easyPrint(1, 2, "Solar         [ok]");
		easyPrint(1, 3, "System        [ok]");
		
		print_flag = false;
	}
	easyPrint(0, cursor, ">");

	if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor, " ");
		displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 3);
		
		enc.isRight();
	}
		
	if (enc.isClick()) {
		lcd.clear();
		print_flag = true;

		switch (cursor) {
		case 0:
			screen = &Display::networkSettingsDisplay;
			break;
		case 1:
			screen = &Display::blynkSettingsDisplay;
			break;
		case 2:
			screen = &Display::solarSettingsDisplay;
			break;
		case 3:
			screen = &Display::systemSettingsDisplay;
			break;
		}
	}
	if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);
		lcd.clear();

		save_settings_flag = true;
		screen = &Display::mainDisplay;
		print_flag = true;
	}
}

void Display::networkSettingsDisplay() {
	static uint8_t cursor;
	static uint8_t print_flag = 2;
	
	if (print_flag) {
		if (print_flag == 2) {
			printTitle(4, 1, "Network");
		}

		easyPrint(1, 0, "Mode [");
		switch (network.getMode()) {
		case NETWORK_OFF:
			lcd.print("off");
			break;
		case NETWORK_STA:
			lcd.print("sta");
			break;
		case NETWORK_AP_STA:
			lcd.print("ap_sta");
			break;
		case NETWORK_AUTO:
			lcd.print("auto");
			break;
		}
		lcd.print("]");
		
		easyPrint(1, 1, "WiFi [");
		lcd.print(network.getWifiSsid());
		lcd.print("]");

		easyPrint(1, 2, "Ssid [");
		lcd.print(network.getApSsid());
		lcd.print("]");
		
		easyPrint(1, 3, "Pass [");
		lcd.print(network.getApPass());
		lcd.print("]");
		
		print_flag = 0;
	}
	easyPrint(0, cursor, ">");

	if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor, " ");
		displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 3);
			
		enc.isRight();
	}

	if (enc.isLeftH(true) || enc.isRightH(true)) {
		clearLine(cursor);
		print_flag = 1;

		switch (cursor) {
		case 1:
			network.setWifi("", "");
			break;
		case 2:
			network.setAp("", NULL);
			break;
		case 3:
			network.setAp(NULL, "");
			break;
		}

		enc.isLeftH();
		enc.isRightH();
	}

	if (enc.isClick()) {
		clearLine(cursor);
		print_flag = 1;

		switch (cursor) {
		case 0:
			network.setMode((network.getMode() == NETWORK_AUTO) ? NETWORK_OFF : network.getMode() + 1);
			break;
		case 1:
			screen = &Display::wifiSettingsDisplay;
			print_flag = 2;

			lcd.clear();
			break;
		case 2:
			char ssid[NETWORK_SSID_PASS_SIZE];
			strcpy(ssid, network.getApSsid());
			
			tick_allow = false;
			displayKeyboard(ssid, NETWORK_SSID_PASS_SIZE);
			tick_allow = true;

			network.setAp(ssid, NULL);
			print_flag = 2;
			break;
		case 3:
			char pass[NETWORK_SSID_PASS_SIZE];
			strcpy(pass, network.getApPass());
			
			tick_allow = false;
			displayKeyboard(pass, NETWORK_SSID_PASS_SIZE);
			tick_allow = true;

			network.setAp(NULL, pass);
			print_flag = 2;
			break;
		}
	}
	if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

		screen = &Display::settingsDisplay;
		print_flag = 2;

		lcd.clear();
	}
}

void Display::blynkSettingsDisplay() {
	static uint8_t cursor;
	static uint8_t print_flag = 2;
	
	if (print_flag) {
		if (print_flag == 2) {
			printTitle(4, 1, "Blynk menu");
		}

		easyPrint(1, 0, "Status [");
		lcd.print((blynk.getWorkStatus()) ? "ON" : "OFF");
		lcd.print("]");

		easyPrint(1, 1, "Auth [");
		lcd.print((*blynk.getAuth()) ? "SET" : "UNSET");
		lcd.print("] ");

		easyPrint(1, 2, "Links         [ok]");

		print_flag = 0;
	}
	easyPrint(0, cursor, ">");

    if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor, " ");
		displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 3);
			
		enc.isRight();
    }

    if (enc.isClick()) {
		clearLine(cursor);
		print_flag = 1;
		
		switch (cursor) {
		case 0:
			blynk.setWorkStatus(!blynk.getWorkStatus());
			break;
		case 1:
			blynk.setAuth("");
			break;
		case 2:
			screen = &Display::blynkLinksSettingsDisplay;
			print_flag = 2;

			lcd.clear();
			break;
		}
    }
    if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

		screen = &Display::settingsDisplay;
		print_flag = 2;

		lcd.clear();
	}
}

void Display::blynkLinksSettingsDisplay() {
	static uint8_t cursor;
	static uint8_t value_cursor = 0;
	static uint8_t print_flag = 2;
	
	if (print_flag) {
		if (print_flag == 2) {
			printTitle(4, 1, "Blynk links");
		}

		for (uint8_t i = 0;i < 4;i++) {
			uint8_t link_index = (cursor / 4) * 4 + i;

			if (link_index < blynk.getLinksCount()) {
				easyPrint(1, i, String("V") + blynk.getLinkPort(link_index));
				easyPrint(8, i, "[");
				lcd.print(blynk.getElementName(blynk.getLinkElement(link_index)) );
				lcd.print("]");
			}

			else {
				easyPrint(1, i, "Add new       [ok]");
				break;
			} 
		}

		print_flag = 0;
	}
	easyPrint(0, cursor % 4, ">");
	easyPrint(6, cursor % 4, (cursor != blynk.getLinksCount()) ? ((!value_cursor) ? "<" : ">") : "");

    if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor % 4, " ");
		easyPrint(6, cursor % 4, (cursor != blynk.getLinksCount()) ? " " : "");

		if (displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, blynk.getLinksCount())) {
			print_flag = 1;
			lcd.clear();
		}

		enc.isRight();
    }

	if (enc.isLeftH(true) || enc.isRightH(true)) {
		clearLine(cursor % 4);
		print_flag = 1;

		if (cursor < blynk.getLinksCount()) {
			switch (value_cursor) {
			case 0:
				blynk.setLinkPort(cursor, blynk.getLinkPort(cursor) + (enc.isLeftH() ? -1 : 1));
				break;
			case 1:
				if (!blynk.getLinkElement(cursor) && enc.isLeftH(true)) {
					blynk.delLink(cursor);

					lcd.clear();
					break;
				}

				blynk.setLinkElement(cursor, blynk.getLinkElement(cursor) + (enc.isLeftH() ? -1 : 1));
				break;
			}
      }

      enc.isLeftH();
      enc.isRightH();
    }
    
    if (enc.isClick()) {
      	clearLine(cursor % 4);
		print_flag = 1;
      
	  	if (cursor < blynk.getLinksCount()) {
			if (++value_cursor == 2) {
				value_cursor = 0;
			}
		}
		else {
			if (!blynk.addLink()) {
				easyPrint(1, cursor % 4, "ERR");
				delay(500);
			}
      	}
    }
    if (enc.isHolded()) {
      	tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

		screen = &Display::blynkSettingsDisplay;
		print_flag = 2;

		lcd.clear();
	}
}

void Display::wifiSettingsDisplay() {
	static uint8_t cursor = 0;
	static uint8_t station_count = 0;
	static bool scan_flag = true;
	static uint8_t print_flag = 2;

	if (scan_flag) {
		lcd.clear();
		easyPrint(8, 1, "Scanning");

		station_count = WiFi.scanNetworks(false, true);
		cursor = (cursor >= station_count) ? station_count - 1 : cursor;
		print_flag = 1;
		
		scan_flag = false;
		lcd.clear();
	}
	
	if (print_flag) {
		if (print_flag == 2) {
			printTitle(8, 1, "WiFi");
		}

		for (uint8_t i = 0;i < 4;i++) {
			uint8_t ssid_index = (cursor / 4) * 4 + i;

			if (ssid_index < station_count) {
				easyPrint(1, i, WiFi.SSID(ssid_index));
			}
		}

		print_flag = 0;
	}
	easyPrint(0, cursor % 4, ">");

	if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor % 4, " ");

		if (displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, station_count - 1)) {
			print_flag = 1;

			lcd.clear();
		}

		enc.isRight();
	}

	if (enc.isLeftH() || enc.isRightH()) {
		scan_flag = true;
	}
		
	if (enc.isClick()) {
		char pass[NETWORK_SSID_PASS_SIZE] = "";
		print_flag = 1;
		tick_allow = false;

		displayKeyboard(pass, NETWORK_SSID_PASS_SIZE);

		if (*pass) {
			easyPrint(0, 0, "Connecting to:");
			easyPrint(2, 1, WiFi.SSID(cursor));
			easyPrint(2, 2, "...");
			easyPrint(2, 2, network.connect(WiFi.SSID(cursor), pass, 10, true) ? "OK " : "ERR");
		
			delay(500);
			lcd.clear();
		}
		
		tick_allow = true;
	}
	if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);
		lcd.clear();

		screen = &Display::networkSettingsDisplay;
		scan_flag = true;
		print_flag = 2;
	}
}

void Display::solarSettingsDisplay() {
	static uint8_t cursor = 0;
	static uint8_t print_flag = 2;

	if (print_flag) {
		if (print_flag == 2) {
			printTitle(8, 1, "Solar");
		}

		if (!(cursor / 4)) {
			easyPrint(1, 0, "Status [");
			lcd.print((solar.getWorkStatus()) ? "ON" : "OFF");
			lcd.print("]");

			easyPrint(1, 1, "OnIfProblem [");
			lcd.print((solar.getOnIfProblem()) ? "ON" : "OFF");
			lcd.print("]");
			
			easyPrint(1, 2, "Delta [");
			lcd.print(solar.getDelta());
			lcd.print("]");
		}

		else if (cursor / 4 == 1) {
			for (uint8_t i = 0;i < 3;i++) {
				int8_t sensor_index = solar.getSensor(i);
				
				switch (i) {
				case 0:
					easyPrint(1, i, "Battery");
					break;
				case 1:
					easyPrint(1, i, "Boiler");
					break;
				case 2:
					easyPrint(1, i, "Exit");
					break;
				}
				
				lcd.print(" [");
				lcd.print((sensor_index < 0) ? "NONE" : ds_sensors[sensor_index].name);
				lcd.print("]");
			}
		}

		print_flag = 0;
	}
	easyPrint(0, cursor % 4, ">");

	if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor % 4, " ");

		if (displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 7)) {
			print_flag = 1;

			lcd.clear();
		}

		enc.isRight();
	}

	if (enc.isLeftH(true) || enc.isRightH(true)) {
	  	clearLine(cursor % 4);
		print_flag = 1;
		
		if (!(cursor / 4)) {
			switch (cursor) {
			case 2:
				solar.setDelta(solar.getDelta() + (enc.isLeftH() ? -1 : 1));
				break;
			}
		}

		if (cursor / 4 == 1) {
			int8_t sensor_index = solar.getSensor(cursor % 4);
			solar.setSensor(cursor % 4, sensor_index + (enc.isLeftH() ? -1 : 1));
		}

		enc.isLeftH();
	  	enc.isRightH();
	}
	
	if (enc.isClick()) {
	  	clearLine(cursor % 4);
		print_flag = 1;

		switch (cursor) {
		case 0:
			solar.setWorkStatus(!solar.getWorkStatus());
			break;
		case 1:
			solar.setOnIfProblem(!solar.getOnIfProblem());
			break;
		}
	}
	if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

		screen = &Display::settingsDisplay;
		print_flag = 2;

		lcd.clear();
	}
}

void Display::systemSettingsDisplay() {
	static uint8_t cursor = 0;
	static uint8_t print_flag = 2;

	if (print_flag) {
		if (print_flag == 2) {
			printTitle(8, 1, "System");
		}

		if (!(cursor / 4)) {
			easyPrint(1, 0, "Time          [ok]");
			easyPrint(1, 1, "DS18B20       [ok]");
			easyPrint(1, 2, "Reset All     [ok]");

			easyPrint(1, 3, "Time data [");
			lcd.print(read_data_time);
			lcd.print("]");
		}

		if (cursor / 4 == 1) {
			easyPrint(1, 0, "Time display [");
			lcd.print(getDisplayOffTime());
			lcd.print("]");

			easyPrint(1, 1, "Display fps [");
			lcd.print(getDisplayFps());
			lcd.print("]");
	
			easyPrint(1, 2, "Rele invert [");
			lcd.print((rele_invert) ? "ON" : "OFF");
			lcd.print("]");

			easyPrint(1, 3, "Buzzer [");
			lcd.print((buzzer_status) ? "ON" : "OFF");
			lcd.print("]");
		}

		print_flag = 0;
	}
	easyPrint(0, cursor % 4, ">");

	if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor % 4, " ");

		if (displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 7)) {
			print_flag = 1;
			lcd.clear();
		}

		enc.isRight();
	}

	if (enc.isLeftH(true) || enc.isRightH(true)) {
		clearLine(cursor % 4);
		print_flag = 1;

		switch (cursor) {
		case 3:
			smartIncr(read_data_time, (enc.isLeftH()) ? -1 : 1, 0, 255);
			break;
		case 4:
			setDisplayOffTime(getDisplayOffTime() + (enc.isLeftH() ? -1 : 1));
			break;
		case 5:
			setDisplayFps(getDisplayFps() + (enc.isLeftH() ? -1 : 1));
			break;
		}

		enc.isRightH();
	}

	if (enc.isClick()) {
		clearLine(cursor % 4);
		print_flag = 1;

		switch(cursor) {
		case 0:
			screen = &Display::timeSettingsDisplay;
			print_flag = 2;

			lcd.clear();
			break;
		case 1:
			screen = &Display::dsSettingsDisplay;
			print_flag = 2;

			lcd.clear();
			break;
		case 2:
			resetAll();
			break;
		case 6:
			rele_invert = !rele_invert;
			break;
		case 7:
			buzzer_status = !buzzer_status;
			break;
		}
	}

	if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

		screen = &Display::settingsDisplay;
		print_flag = 2;

		lcd.clear();
	}
}

void Display::timeSettingsDisplay() {
	static uint8_t cursor = 0;
	static uint8_t print_flag = 2;

	if (print_flag) {
		if (print_flag == 2) {
			printTitle(8, 1, "Time");
		}

		easyPrint(1, 0, "Gmt [");
		lcd.print(gmt);
		lcd.print("] ");

		easyPrint(1, 1, "Ntp sync [");
		lcd.print((ntp_sync) ? "ON" : "OFF");
		lcd.print("] ");
		
		easyPrint(1, 2, "Time          ");
		lcd.print((ntp_sync) ? "    " : "[OK]");
		easyPrint(0, cursor, ">");

		print_flag = 0;
	}
	easyPrint(0, cursor, ">");

	if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor, " ");
		displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 3);
		
		enc.isRight();
	}

	if (enc.isLeftH(true) || enc.isRightH(true)) {
		clearLine(cursor);
		print_flag = 1;

		switch (cursor) {
		case 0:
			smartIncr(gmt, enc.isLeftH() ? -1 : 1, -12, 12);
			break;
		}

	  	enc.isRightH();
	}

	if (enc.isClick()) {
		clearLine(cursor);
		print_flag = 1;

		switch (cursor) {
		case 1:
			ntp_sync = !ntp_sync;
			break;
		case 2:
			if (!ntp_sync) {
				tick_allow = false;
				print_flag = 2;

				displaySetTime();
				tick_allow = true;
			}
			break;
		}
	}
	if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

		screen = &Display::systemSettingsDisplay;
		print_flag = 2;

		lcd.clear();
	}
}

void Display::dsSettingsDisplay() {
	static uint8_t cursor = 0;
	static uint8_t value_cursor = 0;
	static bool print_flag = true;

	if (print_flag) {
		printTitle(8, 1, "Ds18b20");
		print_flag = false;
	}

	for (uint8_t i = 0;i < 4;i++) {
		uint8_t ds_index = (cursor / 4) * 4 + i;

		if (ds_index < DS_SENSORS_COUNT) {
			easyPrint(1, i, (value_cursor || blink_flag || cursor % 4 != i) ? ds_sensors[ds_index].name : "  ");
	  
			if (!*ds_sensors[ds_index].address) {
				easyPrint(4, i, (value_cursor != 1 || blink_flag || cursor % 4 != i) ? "-NONE-" : "      ");
			}
			else {
				lcd.setCursor(4, i);

				if (value_cursor != 1 || blink_flag || cursor % 4 != i) {
					for (uint8_t j = 0;j < 3;j++) {
						lcd.print(ds_sensors[ds_index].address[j], HEX);
					
						if (j != 2) {
							lcd.print("-");
						}
					}
				}
				else {
					lcd.print("        ");
				}
			}

			if (value_cursor != 2 || blink_flag || cursor % 4 != i) {
				easyPrint(13, i, "[");
				lcd.print(ds_sensors[ds_index].correction, 1);
				lcd.print("]");
			}
			else {
				easyPrint(13, i, "       ");
			}
		}
	}
	easyPrint(0, cursor % 4, ">");

    if (enc.isLeft(true) || enc.isRight(true)) {
		easyPrint(0, cursor % 4, " ");
		displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, DS_SENSORS_COUNT - 1);
		
		enc.isRight();
    }
	
    if (enc.isLeftH(true) || enc.isRightH(true)) {
		clearLine(cursor);

		switch (value_cursor) {
		case 0:
			tick_allow = false;
			print_flag = true;

			displayKeyboard(ds_sensors[cursor].name, 3);
			tick_allow = true;

			break;
		case 1:
			getNextSensor(ds_sensors[cursor].address, (enc.isLeftH()) ? -1 : 1);
			break;
		case 2:
			smartIncr(ds_sensors[cursor].correction, (enc.isLeftH()) ? -0.1 : 0.1, -20, 20);
			break;
		}

		enc.isLeftH();
		enc.isRightH();
    }

    if (enc.isClick()) {
		if (++value_cursor == 3) {
			value_cursor = 0;
		}
    }
	if (enc.isHolded()) {
		tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

		screen = &Display::systemSettingsDisplay;
		print_flag = true;

		lcd.clear();
	}
}


void displaySetTime() {
	uint8_t cursor = 0;
	bool print_flag = true;
  	TimeT time_now = clk.getTime(gmt);

	printTitle(2, 1, "Set time");

	lcd.createChar(0, down_symbol);
	lcd.clear();

	while(true) {
		if (print_flag) {
			easyPrint(5, 1, time_now.hour);
			easyPrint(7, 1, ":");
			lcd.print(time_now.minute);
			easyPrint(10, 1, ":");
			lcd.print(time_now.second);

			easyPrint(5, 3, time_now.day);
			easyPrint(7, 3, ".");
			lcd.print(time_now.month);
			easyPrint(10, 3, ".");
			lcd.print(time_now.year);

			print_flag = false;
		}
		easyWrite(5 + (cursor % 3) * 3, 2, (cursor < 3) ? '^' : (char)0);
		easyWrite(6 + (cursor % 3) * 3, 2, (cursor < 3) ? '^' : (char)0);
		
		if (enc.isLeft(true) || enc.isRight(true)) {
			easyPrint(5 + (cursor % 3) * 3, 2, "  ");
			displayCursorTick((enc.isLeft()) ? -1 : 1, &cursor, 5);
			
			enc.isRight();
		}

		if (enc.isLeftH(true) || enc.isRightH(true)) {
			print_flag = true;
			easyPrint(5 + (cursor % 3) * 3, (cursor < 3) ? 1 : 3, "  ");

			switch (cursor) {
			case 0:
				smartIncr(time_now.hour, enc.isLeftH() ? -1 : 1, 0, 23);
				break;
			case 1:
				smartIncr(time_now.minute, enc.isLeftH() ? -1 : 1, 0, 59);
				break;
			case 2:
				smartIncr(time_now.second, enc.isLeftH() ? -1 : 1, 0, 59);
				break;
			case 3:
				smartIncr(time_now.day, enc.isLeftH() ? -1 : 1, 1, 31);
				break;
			case 4:
				smartIncr(time_now.month, enc.isLeftH() ? -1 : 1, 1, 12);
				break;
			case 5:
				smartIncr(time_now.year, enc.isLeftH() ? -1 : 1, 1970, 2037);
				break;
			}

			enc.isRightH();
		}

		if (enc.isClick()) {
			print_flag = true;
			easyPrint(5 + (cursor % 3) * 3, (cursor < 3) ? 1 : 3, "  ");

			switch (cursor) {
			case 0:
				time_now.hour = clk.hour(gmt);
				break;
			case 1:
				time_now.minute = clk.minute(gmt);
				break;
			case 2:
				time_now.second = clk.second(gmt);
				break;
			case 3:
				time_now.day = clk.day(gmt);
				break;
			case 4:
				time_now.month = clk.month(gmt);
				break;
			case 5:
				time_now.year = clk.year(gmt);
				break;
			}
		}
		if (enc.isHolded()) {
			clk.setTime(gmt, time_now);
			tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);

			break;
		}
			
		systemTick();
	}

	lcd.clear();
	displayMakeSymbols();
}

void displayKeyboard(char* array, byte size) {
	uint8_t cursor = 0;
	uint8_t key_cursor = 0;
	uint8_t cursor_max = strlen(array);
	bool print_key_flag = true;
	bool print_string_flag = true;
	bool caps = false;

	lcd.createChar(0, down_symbol);
	lcd.clear();

	while(true) {
		if (print_string_flag || cursor < cursor_max) {
			for (uint8_t i = 0;i < cursor_max;i++) {
				easyWrite(i, 0, (i == cursor && blink_flag) ? '|' : array[i]);
			}

			print_string_flag = false;
		}
		
		if (print_key_flag) {
			for (uint8_t i = 0;i < 20;i++) {
				easyWrite(i, 1, (!caps) ? keyboard1[i] : keyboard2[i]);
				easyWrite(i, 3, (!caps) ? keyboard1[20 + i] : keyboard2[20 + i]);
			}

			print_key_flag = false;
		}
		easyWrite(key_cursor % 20, 2, (key_cursor < 20) ? '^' : (char)0);

		if (enc.isLeft()) {
			easyWrite(key_cursor % 20, 2, ' ');
			key_cursor = (key_cursor == 0) ? 39 : smartIncr(key_cursor, -1, 0, 39);
		}
		if (enc.isRight()) {
			easyWrite(key_cursor % 20, 2, ' ');
			key_cursor = (key_cursor == 39) ? 0 : smartIncr(key_cursor, 1, 0, 39);
		}

		if (enc.isLeftH(true) || enc.isRightH(true)) {
			smartIncr(cursor, (enc.isLeftH()) ? -1 : 1, 0, cursor_max);
			print_string_flag = true;

			enc.isRightH();
		}
		
		if (enc.isClick()) {
			if (key_cursor < 38) {
				if (array[cursor]) {
					array[cursor] = (!caps) ? keyboard1[key_cursor] : keyboard2[key_cursor];
					cursor++;
				}
				else {
					if (cursor != size - 1) {
						array[cursor] = (!caps) ? keyboard1[key_cursor] : keyboard2[key_cursor];
						cursor++;
						array[cursor] = '\0';
			
						cursor_max = strlen(array);
					}
				}
			}
			else if (key_cursor == 38) {
				if (cursor) {
					uint8_t pointer = cursor - 1;
					clearLine(0);
			
					while (array[pointer]) {
						array[pointer] = array[pointer + 1];
						pointer++;
					}
					
					cursor--;
					cursor_max = strlen(array);
				}
			}
			else if (key_cursor == 39) {
				tone(BUZZER_PORT, 1000 * buzzer_status, EXIT_TONE_DURATION);
				break;
			}
			
			print_string_flag = true;
		}
		if (enc.isHolded()) {
			caps = !caps;
			print_key_flag = true;
		}
		
		systemTick();
	}

	lcd.clear();
	displayMakeSymbols();
}


void printTitle(uint8_t x, uint8_t y, String title, uint16_t delay_time) {
	lcd.clear();
	easyPrint(x, y, title);
	
	delay(delay_time);
	lcd.clear();
}

void printDigit(uint8_t digit, uint8_t x, uint8_t y) {
	switch (digit) {
	case 0:
		lcd.setCursor(x, y);
		lcd.write(0);
		lcd.write(1);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(3);
		lcd.write(4);
		lcd.write(5);
		break;
	case 1:
		lcd.setCursor(x, y);
		lcd.write(32);
		lcd.write(1);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(32);
		lcd.write(32);
		lcd.write(5);
		break;
	case 2:
		lcd.setCursor(x, y);
		lcd.write(6);
		lcd.write(6);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(3);
		lcd.write(6);
		lcd.write(6);
		break;
	case 3:
		lcd.setCursor(x, y);
		lcd.write(6);
		lcd.write(6);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(6);
		lcd.write(6);
		lcd.write(5);
		break;
	case 4:
		lcd.setCursor(x, y);
		lcd.write(3);
		lcd.write(4);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(32);
		lcd.write(32);
		lcd.write(5);
		break;
	case 5:
		lcd.setCursor(x, y);
		lcd.write(0);
		lcd.write(6);
		lcd.write(6);
		lcd.setCursor(x, y + 1);
		lcd.write(6);
		lcd.write(6);
		lcd.write(5);
		break;
	case 6:
		lcd.setCursor(x, y);
		lcd.write(0);
		lcd.write(6);
		lcd.write(6);
		lcd.setCursor(x, y + 1);
		lcd.write(3);
		lcd.write(6);
		lcd.write(5);
		break;
	case 7:
		lcd.setCursor(x, y);
		lcd.write(1);
		lcd.write(1);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(32);
		lcd.write(32);
		lcd.write(0);
		break;
	case 8:
		lcd.setCursor(x, y);
		lcd.write(0);
		lcd.write(6);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(3);
		lcd.write(6);
		lcd.write(5);
		break;
	case 9:
		lcd.setCursor(x, y);
		lcd.write(0);
		lcd.write(6);
		lcd.write(2);
		lcd.setCursor(x, y + 1);
		lcd.write(32);
		lcd.write(4);
		lcd.write(5);
		break;
	case 10:
		lcd.setCursor(x, y);
		lcd.print("   ");
		lcd.setCursor(x, y + 1);
		lcd.print("   ");
		break;
	}
}

void easyPrint(uint8_t x, uint8_t y, String array) {
	lcd.setCursor(x, y);
	lcd.print(array);
}
void easyPrint(uint8_t x, uint8_t y, int32_t number) {
	lcd.setCursor(x, y);
	lcd.print(number);
}
void easyPrint(uint8_t x, uint8_t y, float number) {
	lcd.setCursor(x, y);
	lcd.print(number, 2);
}

void easyWrite(uint8_t x, uint8_t y, uint8_t code) {
	lcd.setCursor(x, y);
	lcd.write(code);
}

void clearLine(uint8_t line) {
	for (uint8_t i = 0;i < 20;i++) {
		easyPrint(i, line, " ");
	}
}

void clearColumn(uint8_t column) {
	for (uint8_t i = 0;i < 4;i++) {
		easyPrint(column, i, " ");
	}
}


template <class T>
bool displayCursorTick(int8_t direct, T* cursor, uint8_t cursor_max, T* page, uint8_t page_max) {
	if (direct < 0) {
		if (page != NULL) {
			if (!*cursor && *page) {
				*cursor = cursor_max;
				smartIncr(*page, -1, 0, page_max);
		
				return true;
			}
		}
		
		smartIncr(*cursor, -1, 0, cursor_max);
		if (*cursor % 4 == 3) {
			return true;
		}
	}

	else if (direct > 0) {
		if (page != NULL) {
			if (*cursor == cursor_max && *page != page_max) {
				*cursor = 0;
				smartIncr(*page, 1, 0, page_max);
		
				return true;
			}
		}
		
		smartIncr(*cursor, 1, 0, cursor_max);
		if (!(*cursor % 4)) {
			return true;
		}
	}

	return false;
}

void displayMakeSymbols() {
	lcd.createChar(0, LT);
	lcd.createChar(1, UB);
	lcd.createChar(2, RT);
	lcd.createChar(3, LL);
	lcd.createChar(4, LB);
	lcd.createChar(5, LR);
	lcd.createChar(6, UMB);
	lcd.createChar(7, wifi);
}