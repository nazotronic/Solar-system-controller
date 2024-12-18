/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.1.0
 * Date: 12.12.2024
 */

#include "data.h"

void MainWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	SolarSystemManager* solar = system->getSolarSystemManager();
	Encoder* enc = system->getEncoder();

	if (create_symbol_flag) {
		create_symbol_flag = false;
		makeSymbols(lcd);
	}

	switch(cursor) {
		case 0: printHome(lcd, system); break;
		case 1: printModules(lcd, system); break;
		case 2: printSolar(lcd, system); break;
		case 3: printWifi(lcd, system); break;
		case 4: printAp(lcd, system); break;
		case 5: printBlynk(lcd, system); break;
	}

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->clear();
		windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 5);
		
		enc->isRight();
	}

	if (enc->isClick()) {
		create_symbol_flag = true;
		lcd->clear();

		switch (cursor) {
		case 0:
			display->addWindowToStack(new SettingsWindow);
			break;
		case 1:
			display->addWindowToStack(new DS18B20SettingsDisplay);
			break;
		case 2:
			display->addWindowToStack(new SolarSettingsDisplay);
			break;
		case 3:
		case 4:
			display->addWindowToStack(new NetworkSettingsWindow);
			break;
		case 5:
			display->addWindowToStack(new BlynkSettingsWindow);
			break;
		}
	}
	if (enc->isHolded()) {
		switch (cursor) {
		case 2:
			solar->setReleFlag(!solar->getReleFlag());
			break;
		}
	}
}

void MainWindow::printHome(LcdManager* lcd, SystemManager* system) {
	TimeManager* time = system->getTimeManager();
	ModuleManager* modules = system->getModuleManager();
	SolarSystemManager* solar = system->getSolarSystemManager();
	NetworkManager* network = system->getNetworkManager();
	BlynkManager* blynk = system->getBlynkManager();

	uint8_t H = time->hour();
	uint8_t M = time->minute();

	lcd->easyPrint(0, 0, (time->getStatus() && IS_EVEN_SECOND(millis()) ) ? "T" : " ");
	lcd->print((modules->getAM2320Status() && IS_EVEN_SECOND(millis()) ) ? "A" : " ");

	lcd->setCursor(15, 0);
	lcd->print(solar->getReleFlag() ? (IS_EVEN_SECOND(millis()) ? ">" : "<") : " ");
	
	if (!solar->getWorkFlag()) {
		lcd->print("!");
	}
	else {
		lcd->print((solar->getStatus() && IS_EVEN_SECOND(millis()) ) ? "!" : " ");
	}

	lcd->write((network->isApOn()) ? 178 : 32);

	if (!network->isWifiOn()) {
		lcd->write(32);
	}
	else {
		lcd->write((network->getStatus() == WL_CONNECTED || IS_EVEN_SECOND(millis()) ) ? 7 : 32);
	}

	if (!blynk->getWorkFlag()) {
		lcd->print(" ");
	}
	else {
		lcd->print((blynk->getStatus() || IS_EVEN_SECOND(millis()) ) ? "B" : " ");
	}
	
	printDigit(lcd, 1, 1, H / 10);
	printDigit(lcd, 4, 1, H % 10);

	lcd->easyWrite(7, 1, IS_EVEN_SECOND(millis()) ? 111 : 32);
	lcd->easyWrite(7, 2, IS_EVEN_SECOND(millis()) ? 111 : 32);

	printDigit(lcd, 8, 1, M / 10);
	printDigit(lcd, 11, 1, M % 10);

	lcd->easyPrint(15, 2, time->year());
	lcd->easyPrint(15, 3, String(time->day()) + "." + time->month());
}

void MainWindow::printModules(LcdManager* lcd, SystemManager* system) {
	ModuleManager* modules = system->getModuleManager();

	for (uint8_t i = 0;i < 4;i++) {
		lcd->easyPrint(0, i, (!modules->getDS18B20Status(i) || IS_EVEN_SECOND(millis()) ) ? modules->getDS18B20Name(i) : "  ");
		lcd->print(":");
		lcd->print(modules->getDS18B20T(i), 2);
		lcd->write(223);
	}

	lcd->easyPrint(12, 0, (!modules->getAM2320Status() || IS_EVEN_SECOND(millis()) ) ? "T" : " ");
	lcd->print(":");
	lcd->print(modules->getAM2320T(), 2);
	lcd->write(223);

	lcd->easyPrint(12, 1, (!modules->getAM2320Status() || IS_EVEN_SECOND(millis()) ) ? "H" : " ");
	lcd->print(":");
	lcd->print(modules->getAM2320H(), 2);
	lcd->print("%");
}

void MainWindow::printSolar(LcdManager* lcd, SystemManager* system) {
	SolarSystemManager* solar = system->getSolarSystemManager();

	for (byte i = 0;i < 3;i++) {
		lcd->easyPrint(1, i, "|");
		lcd->easyPrint(5, i, "|");
	}
	if (!solar->getBoilerSensorStatus() || IS_EVEN_SECOND(millis()) ) {
		lcd->easyPrint(2, 0, (int8_t)solar->getBoilerT());
		lcd->easyPrint(2, 1, String(".") + (int8_t)((solar->getBoilerT() - (int8_t)solar->getBoilerT()) * 100) );
		lcd->easyWrite(4, 2, 223);
	}
	else {
		lcd->easyPrint(2, 0, "   ");
		lcd->easyPrint(2, 1, "   ");
		lcd->easyPrint(2, 2, "   ");
	}
	lcd->easyPrint(1, 3, "|___|");

	lcd->easyPrint(8, 1, (solar->getWorkFlag()) ? "ON " : "OFF");

	if (!solar->getExitSensorStatus() || IS_EVEN_SECOND(millis()) ) {
		lcd->easyPrint(7, 2, solar->getExitT());
		lcd->write(223);
	}
	else {
		lcd->easyPrint(7, 2, "       ");
	}

	if (millis() - solar_window_data.pointer_tick_timer >= SOLAR_TICK_POINTER_TIME) {
		solar_window_data.pointer_tick_timer = millis();

		lcd->easyPrint(11 - solar_window_data.pointer, 0, " ");
		lcd->easyPrint(6 + solar_window_data.pointer, 3, " ");

		if (solar->getReleFlag()) {
			solar_window_data.pointer = (solar_window_data.pointer == 5) ? 0 : solar_window_data.pointer + 1;
			
			lcd->easyPrint(11 - solar_window_data.pointer, 0, "<");
			lcd->easyPrint(6 + solar_window_data.pointer, 3, ">");
		}
	}

	lcd->easyPrint(12, 0, "-----");
	for (byte i = 0;i < 2;i++) {
		lcd->easyPrint(13 + i, 1 + i, "|");
		lcd->easyPrint(17 + i, 1 + i, "|");
	}
	lcd->easyPrint(15, 3, "-----");

	if (!solar->getBatterySensorStatus() || IS_EVEN_SECOND(millis()) ) {
		lcd->easyPrint(14, 1, (int)solar->getBatteryT());
		lcd->easyPrint(15, 2, String(".") + (int)((solar->getBatteryT() - (int)solar->getBatteryT()) * 10) );
		lcd->write(223);
	}
	else {
		lcd->easyPrint(14, 1, "   ");
		lcd->easyPrint(15, 2, "   ");
	
	}
}

void MainWindow::printWifi(LcdManager* lcd, SystemManager* system) {
	NetworkManager* network = system->getNetworkManager();
	lcd->easyPrint(0, 0, "WiFi ");

	if (!network->isWifiOn()) {
		lcd->print("disabled");
	}
	else {
		switch (network->getStatus()) {
		case WL_NO_SHIELD:
			lcd->print("NO_SHIELD");
			break;
		case WL_IDLE_STATUS:
			lcd->print("IDLE_STATUS");
			break;
		case WL_NO_SSID_AVAIL:
			lcd->print("NO_SSID_AVAIL");
			break;
		case WL_SCAN_COMPLETED:
			lcd->print("SCAN_COMPLETED");
			break;
		case WL_CONNECTED:
			lcd->print("CONNECTED");
			break;
		case WL_CONNECT_FAILED:
			lcd->print("CONNECT_FAILED");
			break;
		case WL_CONNECTION_LOST:
			lcd->print("CONNECTION_LOST");
			break;
		case WL_WRONG_PASSWORD:
			lcd->print("WRONG_PASSWORD");
			break;
		case WL_DISCONNECTED:
			lcd->print("DISCONNECTED");
			break;
		default:
			lcd->print("ERR");
		}
		
		lcd->easyPrint(0, 1, network->getWifiSsid());
		lcd->print(" ");
		lcd->print(WiFi.RSSI());

		lcd->easyPrint(0, 2, "IP:");
		lcd->print(WiFi.localIP());
	}
}

void MainWindow::printBlynk(LcdManager* lcd, SystemManager* system) {
	BlynkManager* blynk = system->getBlynkManager();

	lcd->easyPrint(0, 0, "Blynk:");
	lcd->print(blynk->getWorkFlag() ? "ON " : "OFF");

	lcd->easyPrint(0, 1, "Auth:");
	lcd->print(*blynk->getAuth() ? "SET  " : "UNSET");

	lcd->easyPrint(0, 2, "Status:");
	lcd->print(blynk->getStatus() ? "CONNECTED   " : "DISCONNECTED");
}

void MainWindow::printAp(LcdManager* lcd, SystemManager* system) {
	NetworkManager* network = system->getNetworkManager();
	lcd->easyPrint(0, 0, "AP ");

	if (!network->isApOn()) {
		lcd->print("disabled");
	}
	else {
		lcd->print(network->getApSsid());
		
		lcd->easyPrint(0, 1, WiFi.softAPgetStationNum());
		lcd->print(" devices");

		lcd->easyPrint(0, 2, "IP:");
		lcd->print(WiFi.softAPIP());
	}
}

void MainWindow::printDigit(LcdManager* lcd, uint8_t x, uint8_t y, uint8_t digit) {
	switch (digit) {
	case 0:
		lcd->setCursor(x, y);
		lcd->write(0);
		lcd->write(1);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(3);
		lcd->write(4);
		lcd->write(5);
		break;
	case 1:
		lcd->setCursor(x, y);
		lcd->write(32);
		lcd->write(1);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(32);
		lcd->write(32);
		lcd->write(5);
		break;
	case 2:
		lcd->setCursor(x, y);
		lcd->write(6);
		lcd->write(6);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(3);
		lcd->write(6);
		lcd->write(6);
		break;
	case 3:
		lcd->setCursor(x, y);
		lcd->write(6);
		lcd->write(6);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(6);
		lcd->write(6);
		lcd->write(5);
		break;
	case 4:
		lcd->setCursor(x, y);
		lcd->write(3);
		lcd->write(4);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(32);
		lcd->write(32);
		lcd->write(5);
		break;
	case 5:
		lcd->setCursor(x, y);
		lcd->write(0);
		lcd->write(6);
		lcd->write(6);
		lcd->setCursor(x, y + 1);
		lcd->write(6);
		lcd->write(6);
		lcd->write(5);
		break;
	case 6:
		lcd->setCursor(x, y);
		lcd->write(0);
		lcd->write(6);
		lcd->write(6);
		lcd->setCursor(x, y + 1);
		lcd->write(3);
		lcd->write(6);
		lcd->write(5);
		break;
	case 7:
		lcd->setCursor(x, y);
		lcd->write(1);
		lcd->write(1);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(32);
		lcd->write(32);
		lcd->write(0);
		break;
	case 8:
		lcd->setCursor(x, y);
		lcd->write(0);
		lcd->write(6);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(3);
		lcd->write(6);
		lcd->write(5);
		break;
	case 9:
		lcd->setCursor(x, y);
		lcd->write(0);
		lcd->write(6);
		lcd->write(2);
		lcd->setCursor(x, y + 1);
		lcd->write(32);
		lcd->write(4);
		lcd->write(5);
		break;
	case 10:
		lcd->setCursor(x, y);
		lcd->print("   ");
		lcd->setCursor(x, y + 1);
		lcd->print("   ");
		break;
	}
}

void MainWindow::makeSymbols(LcdManager* lcd) {
	lcd->createChar(0, LT);
	lcd->createChar(1, UB);
	lcd->createChar(2, RT);
	lcd->createChar(3, LL);
	lcd->createChar(4, LB);
	lcd->createChar(5, LR);
	lcd->createChar(6, UMB);
	lcd->createChar(7, wifi);
}


void SettingsWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	Encoder* enc = system->getEncoder();

	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "Menu");
	}

	if (print_flag) {
		print_flag = false;

		lcd->easyPrint(1, 0, "Network       [ok]");
		lcd->easyPrint(1, 1, "Blynk         [ok]");
		lcd->easyPrint(1, 2, "Solar         [ok]");
		lcd->easyPrint(1, 3, "System        [ok]");
	}
	lcd->easyPrint(0, cursor, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor, " ");
		windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 3);
		
		enc->isRight();
	}
		
	if (enc->isClick()) {
		print_flag = true;
		lcd->clear();

		switch (cursor) {
		case 0:
			display->addWindowToStack(new NetworkSettingsWindow);
			break;

		case 1:
			display->addWindowToStack(new BlynkSettingsWindow);
			break;
		
		case 2:
			display->addWindowToStack(new SolarSettingsDisplay);
			break;

		case 3:
			display->addWindowToStack(new SystemSettingsDisplay);
			break;
		}
	}
	if (enc->isHolded()) {
		system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		system->saveSettingsRequest();
		display->deleteWindowFromStack(this);
	}
}


void NetworkSettingsWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	NetworkManager* network = system->getNetworkManager();
	Encoder* enc = system->getEncoder();

	if (*ssid_ap || *pass_ap) {
		network->setAp((*ssid_ap) ? ssid_ap : NULL, (*pass_ap) ? pass_ap : NULL);

		*ssid_ap = '\0';
		*pass_ap = '\0';
	}

	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "Network");
	}
	
	if (print_flag) {
		print_flag = false;

		lcd->easyPrint(1, 0, "Mode [");

		switch (network->getMode()) {
		case NETWORK_OFF:
			lcd->print("off");
			break;
		case NETWORK_STA:
			lcd->print("sta");
			break;
		case NETWORK_AP_STA:
			lcd->print("ap_sta");
			break;
		case NETWORK_AUTO:
			lcd->print("auto");
			break;
		}
		lcd->print("]");
		
		lcd->easyPrint(1, 1, "WiFi [");
		lcd->print(network->getWifiSsid());
		lcd->print("]");

		lcd->easyPrint(1, 2, "Ssid [");
		lcd->print(network->getApSsid());
		lcd->print("]");
		
		lcd->easyPrint(1, 3, "Pass [");
		lcd->print(network->getApPass());
		lcd->print("]");
	}
	lcd->easyPrint(0, cursor, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor, " ");
		windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 3);
		
		enc->isRight();
	}

	if (enc->isClick()) {
		print_flag = true;

		switch (cursor) {
		case 0:
			lcd->clearLine(cursor);
			network->setMode((network->getMode() == NETWORK_AUTO) ? NETWORK_OFF : network->getMode() + 1);
			
			break;
		case 1:
			lcd->clear();
			display->addWindowToStack(new WifiSettingsWindow);

			break;
		case 2:
			{
			KeyboardWindow* keyboard = new KeyboardWindow;

			strcat(ssid_ap, network->getApSsid());
			keyboard->setString(ssid_ap, NETWORK_SSID_PASS_SIZE);

			lcd->clear();
			display->addWindowToStack(keyboard);
			}

			break;
		case 3:
			{
			KeyboardWindow* keyboard = new KeyboardWindow;

			strcat(pass_ap, network->getApPass());
			keyboard->setString(pass_ap, NETWORK_SSID_PASS_SIZE);

			lcd->clear();
			display->addWindowToStack(keyboard);
			}

			break;
		}
	}
	if (enc->isHolded()) {
		system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void WifiSettingsWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	NetworkManager* network = system->getNetworkManager();
	Encoder* enc = system->getEncoder();

	if (initialization_flag) {
		initialization_flag = false;

		strcat(ssid, network->getWifiSsid());
		strcat(pass, network->getWifiPass());
	}

	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "WiFi");
	}

	if (print_flag) {
		print_flag = false;

		lcd->easyPrint(1, 0, "Ssid [");
		lcd->print(ssid);
		lcd->print("]");

		lcd->easyPrint(1, 1, "Pass [");
		lcd->print(pass);
		lcd->print("]");

		lcd->easyPrint(1, 2, "Save          [ok]");
	}
	lcd->easyPrint(0, cursor, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor, " ");
		windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 2);
		
		enc->isRight();
	}

	if (enc->isClick()) {
		print_flag = true;
		lcd->clear();

		switch (cursor) {
		case 0:
			{
			WifiStationsWindow* wifi_stations = new WifiStationsWindow;
			wifi_stations->setString(ssid, NETWORK_SSID_PASS_SIZE);

			// lcd->clear();
			display->addWindowToStack(wifi_stations);			
			}

			break;
		case 1:
			{
			KeyboardWindow* keyboard = new KeyboardWindow;
			keyboard->setString(pass, NETWORK_SSID_PASS_SIZE);

			// lcd->clear();
			display->addWindowToStack(keyboard);			
			}

			break;
		case 2:
			if (!*ssid) {
				network->setWifi("", "");
			}
			else {
				bool connect_flag = false;
				display->setWorkFlag(false);
				
				lcd->easyPrint(0, 0, "Connecting to:");
				lcd->easyPrint(2, 1, ssid);
				lcd->easyPrint(2, 2, "...");

				connect_flag = network->connect(ssid, pass, 10, true);

				lcd->easyPrint(2, 2, (connect_flag) ? "OK " : "ERR");
			
				delay(500);
				lcd->clear();

				display->setWorkFlag(true);

				if (connect_flag) {
					display->deleteWindowFromStack(this);
				}
			}

			break;
		}
	}
	if (enc->isHolded()) {
		system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void BlynkSettingsWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	BlynkManager* blynk = system->getBlynkManager();
	Encoder* enc = system->getEncoder();

	if (print_title_flag) {
		print_title_flag = false;
		
		lcd->printTitle(1, "Blynk menu");
	}

	if (print_flag) {
		print_flag = false;

		lcd->easyPrint(1, 0, "Status [");
		lcd->print(blynk->getWorkFlag() ? "ON" : "OFF");
		lcd->print("]");

		lcd->easyPrint(1, 1, "Send time [");
		lcd->print(blynk->getSendDataTime());
		lcd->print("]");

		lcd->easyPrint(1, 2, "Auth [");
		lcd->print((*blynk->getAuth()) ? "SET" : "UNSET");
		lcd->print("] ");

		lcd->easyPrint(1, 3, "Links         [ok]");
	}
	lcd->easyPrint(0, cursor, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor, " ");
		windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 3);
			
		enc->isRight();
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_flag = true;
		lcd->clearLine(cursor % 4);

		switch (cursor) {
		case 2:
			blynk->setSendDataTime(blynk->getSendDataTime() + (enc->isLeftH() ? -1 : 1));
			break;
		}

		enc->isRightH();
	}

	if (enc->isClick()) {
		print_flag = true;
		
		switch (cursor) {
		case 0:
			lcd->clearLine(cursor);
			blynk->setWorkFlag(!blynk->getWorkFlag());

			break;
		case 2:
			lcd->clearLine(cursor);
			blynk->setAuth("");

			break;
		case 3:
			lcd->clear();
			display->addWindowToStack(new BlynkLinksSettingsWindow);

			break;
		}
	}
	if (enc->isHolded()) {
		system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void BlynkLinksSettingsWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	BlynkManager* blynk = system->getBlynkManager();
	Encoder* enc = system->getEncoder();

	if (print_title_flag) {
		print_title_flag = false;

		lcd->printTitle(1, "Blynk links");
	}

	if (print_flag) {
		print_flag = false;

		for (uint8_t i = 0;i < 4;i++) {
			uint8_t link_index = (cursor / 4) * 4 + i;

			if (link_index < blynk->getLinksCount()) {
				lcd->easyPrint(1, i, String("V") + blynk->getLinkPort(link_index));

				lcd->easyPrint(8, i, "[");
				lcd->print(blynk->getElementName(blynk->getLinkElement(link_index)) );
				lcd->print("]");
			}

			else {
				lcd->easyPrint(1, i, "Add new       [ok]");
				break;
			} 
		}
	}
	lcd->easyPrint(0, cursor % 4, ">");
	lcd->easyPrint(6, cursor % 4, (cursor != blynk->getLinksCount()) ? ((!value_cursor) ? "<" : ">") : "");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor % 4, " ");
		lcd->easyPrint(6, cursor % 4, (cursor != blynk->getLinksCount()) ? " " : "");

		if (windowCursorTick(cursor, enc->isLeft() ? -1 : 1, blynk->getLinksCount()) ) {
			print_flag = true;
			lcd->clear();
		}

		enc->isRight();
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_flag = true;
		lcd->clearLine(cursor % 4);

		if (cursor < blynk->getLinksCount()) {
			switch (value_cursor) {
			case false:
				blynk->setLinkPort(cursor, blynk->getLinkPort(cursor) + (enc->isLeftH() ? -1 : 1));
				break;

			case true:
				if (!blynk->getLinkElement(cursor) && enc->isLeftH(true)) {
					blynk->delLink(cursor);
					lcd->clear();

					break;
				}

				blynk->setLinkElement(cursor, blynk->getLinkElement(cursor) + (enc->isLeftH() ? -1 : 1));
				break;
			}
	  	}

		enc->isLeftH();
		enc->isRightH();
	}
	
	if (enc->isClick()) {
		print_flag = true;
	  	lcd->clearLine(cursor % 4);
	
	  	if (cursor < blynk->getLinksCount()) {
			value_cursor = !value_cursor;
		}
		else {
			if (!blynk->addLink()) {
				lcd->easyPrint(1, cursor % 4, "ERR");
				delay(500);
			}
	  	}
	}
	if (enc->isHolded()) {
	  	system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void SolarSettingsDisplay::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	SolarSystemManager* solar = system->getSolarSystemManager();
	ModuleManager* modules = system->getModuleManager();
	Encoder* enc = system->getEncoder();
	
	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "Solar");
	}

	if (print_flag) {
		print_flag = false;
		if (!(cursor / 4)) {
			lcd->easyPrint(1, 0, "Status [");
			lcd->print((solar->getWorkFlag()) ? "ON" : "OFF");
			lcd->print("]");

			lcd->easyPrint(1, 1, "Error on [");
			lcd->print((solar->getErrorOnFlag()) ? "ON" : "OFF");
			lcd->print("]");

			lcd->easyPrint(1, 2, "Rele invert [");
			lcd->print((solar->getReleInvertFlag()) ? "ON" : "OFF");
			lcd->print("]");
			
			lcd->easyPrint(1, 3, "Delta [");
			lcd->print(solar->getDelta());
			lcd->print("]");
		}

		else if (cursor / 4 == 1) {
			for (uint8_t i = 0;i < 3;i++) {
				int8_t sensor_index = solar->getSensor(i);
				
				switch (i) {
				case 0:
					lcd->easyPrint(1, i, "Battery");
					break;
				case 1:
					lcd->easyPrint(1, i, "Boiler");
					break;
				case 2:
					lcd->easyPrint(1, i, "Exit");
					break;
				}
				
				lcd->print(" [");
				lcd->print((sensor_index >= 0) ? modules->getDS18B20Name(sensor_index) : "NONE");
				lcd->print("]");
			}
		}
	}
	lcd->easyPrint(0, cursor % 4, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor % 4, " ");

		if (windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 6)) {
			print_flag = true;
			lcd->clear();
		}

		enc->isRight();
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_flag = true;
	  	lcd->clearLine(cursor % 4);
		
		switch (cursor) {
		case 3:
			solar->setDelta(solar->getDelta() + (enc->isLeftH() ? -1 : 1));
			break;
		case 4:
		case 5:
		case 6:
			{
			int8_t sensor_index = solar->getSensor(cursor % 4);
			solar->setSensor(cursor % 4, sensor_index + (enc->isLeftH() ? -1 : 1));
			}

			break;
		}

		enc->isLeftH();
	  	enc->isRightH();
	}
	
	if (enc->isClick()) {
		print_flag = true;
	  	lcd->clearLine(cursor % 4);

		switch (cursor) {
		case 0:
			solar->setWorkFlag(!solar->getWorkFlag());
			break;
		case 1:
			solar->setErrorOnFlag(!solar->getErrorOnFlag());
			break;
		case 2:
			solar->setReleInvertFlag(!solar->getReleInvertFlag());
			break;
		}
	}
	if (enc->isHolded()) {
	  	system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void SystemSettingsDisplay::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	ModuleManager* modules = system->getModuleManager();
	Encoder* enc = system->getEncoder();

	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "System");
	}

	if (print_flag) {
		print_flag = false;

		if (!(cursor / 4)) {
			lcd->easyPrint(1, 0, "Time          [ok]");
			lcd->easyPrint(1, 1, "DS18B20       [ok]");
			lcd->easyPrint(1, 2, "Reset All     [ok]");

			lcd->easyPrint(1, 3, "Time data [");
			lcd->print(modules->getReadDataTime());
			lcd->print("]");
		}

		if (cursor / 4 == 1) {
			lcd->easyPrint(1, 0, "Time display [");
			lcd->print(display->getBacklightOffTime());
			lcd->print("]");

			lcd->easyPrint(1, 1, "Display fps [");
			lcd->print(display->getFps());
			lcd->print("]");

			lcd->easyPrint(1, 2, "Buzzer [");
			lcd->print(system->getBuzzerFlag() ? "ON" : "OFF");
			lcd->print("]");
		}
	}
	lcd->easyPrint(0, cursor % 4, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor % 4, " ");

		if (windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 6)) {
			print_flag = true;
			lcd->clear();
		}

		enc->isRight();
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_flag = true;
		lcd->clearLine(cursor % 4);

		switch (cursor) {
		case 3:
			modules->setReadDataTime(modules->getReadDataTime() + (enc->isLeftH() ? -1 : 1));
			break;
		case 4:
			display->setBacklightOffTime(display->getBacklightOffTime() + (enc->isLeftH() ? -1 : 1));
			break;
		case 5:
			display->setFps(display->getFps() + (enc->isLeftH() ? -1 : 1));
			break;
		}

		enc->isLeftH();
		enc->isRightH();
	}

	if (enc->isClick()) {
		print_flag = true;

		switch(cursor) {
		case 0:
			lcd->clear();
			display->addWindowToStack(new TimeSettingsDisplay);

			break; 
		case 1:
			lcd->clear();
			display->addWindowToStack(new DS18B20SettingsDisplay);

			break;
		case 2:
			system->resetAll();
			break;
		case 6:
			lcd->clearLine(cursor % 4);
			system->setBuzzerFlag(!system->getBuzzerFlag());

			break;
		}
	}

	if (enc->isHolded()) {
	  	system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void TimeSettingsDisplay::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	TimeManager* time = system->getTimeManager();
	Encoder* enc = system->getEncoder();
	
	if (time_to_set != NULL) {
		time->setTime(time_to_set);
		free(time_to_set);

		time_to_set = NULL;
	}

	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "Time");
	}

	if (print_flag) {
		print_flag = false;

		lcd->easyPrint(1, 0, "Ntp sync [");
		lcd->print(time->getNtpFlag() ? "ON" : "OFF");
		lcd->print("] ");

		lcd->easyPrint(1, 1, "Gmt [");
		lcd->print(time->getGmt());
		lcd->print("] ");
		
		lcd->easyPrint(1, 2, "Time          ");
		lcd->print(time->getNtpFlag() ? "    " : "[OK]");
		lcd->easyPrint(0, cursor, ">");
	}
	lcd->easyPrint(0, cursor, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor, " ");
		windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 2);
		
		enc->isRight();
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_flag = true;
		lcd->clearLine(cursor);

		switch (cursor) {
		case 1:
			time->setGmt(time->getGmt() + (enc->isLeftH() ? -1 : 1));
			break;
		}

		enc->isLeftH();
	  	enc->isRightH();
	}

	if (enc->isClick()) {
		print_flag = true;

		switch (cursor) {
		case 0:
			lcd->clearLine(cursor);
			time->setNtpFlag(!time->getNtpFlag());

			break;
		case 2:
			if (!time->getNtpFlag()) {
				TimeSetDisplay* time_set_display = new TimeSetDisplay;
				TimeT time_now = time->getTime();
				
				time_to_set = new TimeT;

				memcpy(time_to_set, &time_now, sizeof(TimeT));
				time_set_display->setTimeT(time_to_set);

				lcd->clear();
				display->addWindowToStack(time_set_display);
			}

			break;
		}
	}
	
	if (enc->isHolded()) {
	  	system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void DS18B20SettingsDisplay::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	ModuleManager* modules = system->getModuleManager();
	Encoder* enc = system->getEncoder();

	if (ds18b20_to_set != NULL) {
		modules->setDS18B20(cursor, ds18b20_to_set);
		free(ds18b20_to_set);

		ds18b20_to_set = NULL;
	}

	if (millis() - update_timer > SEC_TO_MLS(modules->getReadDataTime()) ) {
		update_timer = millis();
		print_flag = true;
	}

	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "DS18B20");
	}

	if (print_flag) {
		print_flag = false;

		for (uint8_t i = 0;i < 4;i++) {
			uint8_t ds_index = (cursor / 4) * 4 + i;

			if (ds_index < modules->getDS18B20Count()) {
				lcd->easyPrint(1, i, modules->getDS18B20Name(ds_index));

				lcd->easyPrint(8, i, modules->getDS18B20T(ds_index));
				lcd->write(223);
			}
		}
	}
	lcd->easyPrint(0, cursor % 4, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor % 4, " ");

		if (windowCursorTick(cursor, enc->isLeft() ? -1 : 1, modules->getDS18B20Count() - 1)) {
			print_flag = true;
			lcd->clear();
		}

		enc->isRight();
	}

	if (enc->isClick()) {
		DS18B20SetDisplay* ds18b20_set_display = new DS18B20SetDisplay;	
		ds18b20_to_set = new ds18b20_data_t;

		print_flag = true;

		memcpy(ds18b20_to_set, modules->getDS18B20(cursor), sizeof(ds18b20_data_t));
		ds18b20_set_display->setDS18B20(ds18b20_to_set);

		lcd->clear();
		display->addWindowToStack(ds18b20_set_display);
	}
	if (enc->isHolded()) {
	  	system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}


void TimeSetDisplay::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	Encoder* enc = system->getEncoder();

	if (print_title_flag) {
		print_title_flag = false;

		lcd->printTitle(1, "Set time");
		lcd->createChar(0, down_symbol);
	}
	
	if (print_flag) {
		print_flag = false;

		lcd->easyPrint(5, 1, time->hour);
		lcd->easyPrint(7, 1, ":");
		lcd->print(time->minute);
		lcd->easyPrint(10, 1, ":");
		lcd->print(time->second);

		lcd->easyPrint(5, 3, time->day);
		lcd->easyPrint(7, 3, ".");
		lcd->print(time->month);
		lcd->easyPrint(10, 3, ".");
		lcd->print(time->year);
	}
	lcd->easyWrite(5 + (cursor % 3) * 3, 2, (cursor < 3) ? '^' : (char)0);
	lcd->easyWrite(6 + (cursor % 3) * 3, 2, (cursor < 3) ? '^' : (char)0);
		
	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(5 + (cursor % 3) * 3, 2, "  ");
		windowCursorTick(cursor, enc->isLeft() ? -1 : 1, 5);
		
		enc->isRight();
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_flag = true;
		lcd->easyPrint(5 + (cursor % 3) * 3, (cursor < 3) ? 1 : 3, "  ");

		switch (cursor) {
		case 0:
			smartIncr(time->hour, enc->isLeftH() ? -1 : 1, 0, 23);
			break;
		case 1:
			smartIncr(time->minute, enc->isLeftH() ? -1 : 1, 0, 59);
			break;
		case 2:
			smartIncr(time->second, enc->isLeftH() ? -1 : 1, 0, 59);
			break;
		case 3:
			smartIncr(time->day, enc->isLeftH() ? -1 : 1, 1, 31);
			break;
		case 4:
			smartIncr(time->month, enc->isLeftH() ? -1 : 1, 1, 12);
			break;
		case 5:
			smartIncr(time->year, enc->isLeftH() ? -1 : 1, 1970, 2037);
			break;
		}

		enc->isRightH();
	}

	if (enc->isHolded()) {
	  	system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}

void TimeSetDisplay::setTimeT(TimeT* time) {
	this->time = time;
}


void DS18B20SetDisplay::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	ModuleManager* modules = system->getModuleManager();
	DallasTemperature* ds18b20_sensor = modules->getDallasTemperature();
	Encoder* enc = system->getEncoder();

	if (millis() - update_timer > SEC_TO_MLS(modules->getReadDataTime()) ) {
		update_timer = millis();
		print_flag = true;
	}

	if (print_title_flag) {
		print_title_flag = false;
		lcd->printTitle(1, "Set DS18B20");
	}

	if (print_flag) {
		print_flag = false;

		ds18b20_sensor->requestTemperaturesByAddress(ds18b20->address);
		float t = ds18b20_sensor->getTempC(ds18b20->address);

		lcd->easyPrint(1, 0, ds18b20->name);

		lcd->setCursor(8, 0);
		if (ds18b20->address != NULL) {
			lcd->print(t);
			lcd->write(223);
		}
		else {
			lcd->print("ERR");
		}

		lcd->easyPrint(1, 1, "Name [");
		lcd->print(ds18b20->name);
		lcd->print("] ");

		lcd->easyPrint(1, 2, "Addr [");
		for (uint8_t i = 0;i < 3;i++) {
			lcd->print(ds18b20->address[i], HEX);
					
			if (i != 2) {
				lcd->print("-");
			}
		}
		lcd->print("] ");

		lcd->easyPrint(1, 3, "Correction [");
		lcd->print(ds18b20->correction);
		lcd->print("] ");
	}
	lcd->easyPrint(0, cursor, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor, " ");
		smartIncr(cursor, enc->isLeft() ? -1 : 1, 1, 3);
		
		enc->isRight();
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_flag = true;
		lcd->clearLine(cursor);

		switch (cursor) {
		case 3:
			smartIncr(ds18b20->correction, enc->isLeftH() ? -0.1 : 0.1, -20, 20);
			break;
		}

		enc->isLeftH();
	  	enc->isRightH();
	}

	if (enc->isClick()) {
		print_flag = true;

		switch (cursor) {
		case 1:
			{
			KeyboardWindow* keyboard = new KeyboardWindow;
			keyboard->setString(ds18b20->name, DS_NAME_SIZE);

			lcd->clear();
			display->addWindowToStack(keyboard);	
			}

			break;
		case 2:
			{
			DS18B20AddressWindow* ds18b20_address_set = new DS18B20AddressWindow;
			ds18b20_address_set->setArray(ds18b20->address, DS_NAME_SIZE);

			lcd->clear();
			display->addWindowToStack(ds18b20_address_set);	
			}

			break;
		}
	}
	if (enc->isHolded()) {
	  	system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
}

void DS18B20SetDisplay::setDS18B20(ds18b20_data_t* ds18b20) {
	this->ds18b20 = ds18b20;
}


void DS18B20AddressWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	ModuleManager* modules = system->getModuleManager();
	DallasTemperature* ds18b20_sensor = modules->getDallasTemperature();
	Encoder* enc = system->getEncoder();

	if (millis() - scan_timer > SEC_TO_MLS(modules->getReadDataTime()) ) {
		scan_timer = millis();

		scan_flag = true;
		print_flag = true;
	}

	if (scan_flag) {
		scan_flag = false;

		ds18b20_sensor->begin();
		ds18b20_sensor->requestTemperatures();
		ds18b20_count = ds18b20_sensor->getDS18Count();
	}

	if (print_flag) {
		print_flag = false;

		if (!ds18b20_count) {
			lcd->clear();
			lcd->easyPrint(1, 0, "No ds18b20");
		}
		else {
			for (uint8_t i = 0;i < 4;i++) {
				uint8_t ds_index = (cursor / 4) * 4 + i;

				if (ds_index < ds18b20_count) {
					uint8_t ds18b20_address[8];
					float t;

					ds18b20_sensor->getAddress(ds18b20_address, ds_index);
					t = ds18b20_sensor->getTempCByIndex(ds_index);

					lcd->setCursor(1, i);

					for (uint8_t i = 0;i < 4;i++) {
						lcd->print(ds18b20_address[i], HEX);
								
						if (i != 3) {
							lcd->print("-");
						}
					}

					lcd->print(" ");
					lcd->print(t);
					lcd->write(223);
				}

				else {
					lcd->clearLine(i);
				}
			}
		}
	}
	lcd->easyPrint(0, cursor, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor % 4, " ");

		if (windowCursorTick(cursor, enc->isLeft() ? -1 : 1, ds18b20_count - 1)) {
			print_flag = true;
			lcd->clear();
		}

		enc->isRight();
	}

	if (enc->isClick()) {
		ds18b20_sensor->getAddress(array, cursor);

		// system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
	if (enc->isHolded()) {
		scan_flag = true;
		print_flag = true;
	}
}

void DS18B20AddressWindow::setArray(uint8_t* array, uint8_t size) {
	this->array = array;
	this->size = size;
}


void WifiStationsWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	Encoder* enc = system->getEncoder();

	if (scan_flag) {
		scan_flag = false;

		lcd->clear();
		lcd->easyPrint(2, 1, "Scanning");

		stations_count = WiFi.scanNetworks(false, true);
		cursor = (cursor >= stations_count) ? stations_count - 1 : cursor;
		print_flag = true;

		lcd->easyPrint(2, 2, (stations_count) ? "OK " : "ERR");
		lcd->easyPrint(2, 3, stations_count);
		lcd->print("stations");

		delay(500);
		lcd->clear();
	}

	if (print_flag) {
		print_flag = false;

		for (uint8_t i = 0;i < 4;i++) {
			uint8_t ssid_index = (cursor / 4) * 4 + i;

			if (ssid_index < stations_count) {
				lcd->easyPrint(1, i, WiFi.SSID(ssid_index));
			}
		}
	}
	lcd->easyPrint(0, cursor % 4, ">");

	if (enc->isLeft(true) || enc->isRight(true)) {
		lcd->easyPrint(0, cursor % 4, " ");

		if (windowCursorTick(cursor, enc->isLeft() ? -1 : 1, stations_count - 1)) {
			print_flag = true;
			lcd->clear();
		}

		enc->isRight();
	}

	if (enc->isClick()) {
		strcpy(string, WiFi.SSID(cursor).c_str());

		// system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
		lcd->clear();

		display->deleteWindowFromStack(this);
	}
	if (enc->isHolded()) {
		scan_flag = true;
	}
}

void WifiStationsWindow::setString(char* string, uint8_t size) {
	this->string = string;
	this->size = size;
}


void KeyboardWindow::print(LcdManager* lcd, DisplayManager* display, SystemManager* system) {
	Encoder* enc = system->getEncoder();
	string_size_now = strlen(string);
	
	if (create_symbol_flag) {
		create_symbol_flag = false;

		lcd->createChar(0, down_symbol);
	}

	if (print_string_flag || cursor < string_size_now) {
		print_string_flag = false;

		for (uint8_t i = 0;i < string_size_now;i++) {
			lcd->easyWrite(i, 0, (i == cursor && IS_EVEN_SECOND(millis()) ) ? '|' : string[i]);
		}
	}

	if (cursor == string_size_now && cursor != size - 1) {
		lcd->easyWrite(cursor, 0, IS_EVEN_SECOND(millis()) ? '_' : 32);
	}
		
	if (print_key_flag) {
		print_key_flag = false;

		for (uint8_t i = 0;i < 20;i++) {
			lcd->easyWrite(i, 1, (!caps) ? keyboard1[i] : keyboard2[i]);
			lcd->easyWrite(i, 3, (!caps) ? keyboard1[20 + i] : keyboard2[20 + i]);
		}		
	}
	lcd->easyWrite(key_cursor % 20, 2, (key_cursor < 20) ? '^' : (char)0);

	if (enc->isLeft()) {
		lcd->easyWrite(key_cursor % 20, 2, ' ');
		key_cursor = (key_cursor == 0) ? 39 : smartIncr(key_cursor, -1, 0, 39);
	}
	if (enc->isRight()) {
		lcd->easyWrite(key_cursor % 20, 2, ' ');
		key_cursor = (key_cursor == 39) ? 0 : smartIncr(key_cursor, 1, 0, 39);
	}

	if (enc->isLeftH(true) || enc->isRightH(true)) {
		print_string_flag = true;

		smartIncr(cursor, (enc->isLeftH()) ? -1 : 1, 0, string_size_now); 
		enc->isRightH();
	}
		
	if (enc->isClick()) {
		if (key_cursor < 38) {
			if (string[cursor]) {
				string[cursor] = (!caps) ? keyboard1[key_cursor] : keyboard2[key_cursor];
				cursor++;
			}
			else {
				if (cursor != size - 1) {
					string[cursor] = (!caps) ? keyboard1[key_cursor] : keyboard2[key_cursor];
					cursor++;
					string[cursor] = '\0';
				}
			}
		}
		else if (key_cursor == 38) {
			if (cursor) {
				uint8_t pointer = cursor - 1;
				lcd->clearLine(0);
			
				while (string[pointer]) {
					string[pointer] = string[pointer + 1];
					pointer++;
				}
					
				cursor--;
			}
		}
		else if (key_cursor == 39) {
			// system->buzzer(SCREEN_EXIT_BUZZER_FREQ, SCREEN_EXIT_BUZZER_TIME);
			lcd->clear();

			display->deleteWindowFromStack(this);
		}
			
		print_string_flag = true;
	}
	if (enc->isHolded()) {
		caps = !caps;
		print_key_flag = true;
	}
}

void KeyboardWindow::setString(char* string, uint8_t size) {
	this->string = string;
	this->size = size;
}