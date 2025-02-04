/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.1
 * Date: 04.02.2025
 */

#include "data.h"

void NetworkManager::endBegin() {
	ui.attachBuild(uiBuild);
	ui.attach(uiAction);
	ui.enableOTA();

	updateWebBlynkBlock();
	updateWebSensorsBlock();

	web_update_codes = "HSt,HSh,";
	web_update_codes += "HSSbat,HSSboi,HSSext,HSSpu,";
	web_update_codes += "SNm,SNWs,SNAs,SNAp,SBs,SBsdt,SBa,";
	web_update_codes += "STg,STns,SSrdt,";
	web_update_codes += "SDar,SDbot,SDf,SSSs,SSSeo,SSSri,SSSd,SSSba,SSSbo,SSSex,SSb";
}


void NetworkManager::uiBuild() {
	if (system == NULL) {
		return;
	}

	TimeManager* time = system->getTimeManager();
	SensorsManager* sensors = system->getSensorsManager();
	SolarSystemManager* solar = system->getSolarSystemManager();
	DisplayManager* display = system->getDisplayManager();
	NetworkManager* network = system->getNetworkManager();
	BlynkManager* blynk = system->getBlynkManager();
	String update_codes = web_update_codes;
	
	for (byte i = 0;i < sensors->getDS18B20Count();i++) {
		update_codes += "HSdsn";
		update_codes += i;
		update_codes += ",";
		update_codes += "HSdst";
		update_codes += i;
		update_codes += ",";

		update_codes += "SSDSn";
		update_codes += i;
		update_codes += ",";
		update_codes += "SSDSa";
		update_codes += i;
		update_codes += ",";
		update_codes += "SSDSr";
		update_codes += i;
		update_codes += ",";
		update_codes += "SSDSc";
		update_codes += i;
		update_codes += ",";
	}

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		update_codes += "SBLp";
		update_codes += i;
		update_codes += ",";
		update_codes += "SBLe";
		update_codes += i;

		if (i != blynk->getLinksCount() - 1) {
			update_codes += ",";
		}
	}

	GP.BUILD_BEGIN(550);
	GP.THEME(GP_DARK);
	GP.UPDATE(update_codes, SEC_TO_MLS(WEB_UPDATE_TIME));
	
	GP.TITLE("nazotronic");
	GP.NAV_TABS_LINKS("/,/settings,/memory", "Home,Settings,Memory", GP_ORANGE);
	GP.HR();

	if (ui.uri("/")) {
		M_SPOILER("Info", GP_ORANGE,
			GP.SYSTEM_INFO("1.3.1");
		);
		
		M_BLOCK(GP_THIN,
			GP.LABEL("Sensors");
			
			M_BOX(GP_LEFT,
				GP.LABEL("T:");

				if (!sensors->getAM2320Status()) {
					GP.PLAIN(String(sensors->getAM2320T(), 1) + "°", "HSt");
				}
				else {
					GP.PLAIN("err", "HSt");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("H:");

				if (!sensors->getAM2320Status()) {
					GP.PLAIN(String(sensors->getAM2320H(), 1) + "%", "HSh");
				}
				else {
					GP.PLAIN("err", "HSh");
				}
			);

			for (byte i = 0;i < sensors->getDS18B20Count();i++) {
				M_BOX(GP_LEFT,
					GP.LABEL(sensors->getDS18B20Name(i), String("HSdsn") + i);
					GP.LABEL(":");
					
					if (!sensors->getDS18B20Status(i)) {
						GP.PLAIN(String(sensors->getDS18B20T(i), 1) + "°", String("HSdst") + i);
					}
					else {
						GP.PLAIN("err", String("HSdst") + i);
					}
				);
			}
		);

		M_BLOCK(GP_THIN,
			GP.LABEL("Solar system");
			
			M_BOX(GP_LEFT,
				GP.LABEL("Battery:");

				if (!solar->getBatterySensorStatus()) {
					GP.PLAIN(String(solar->getBatteryT(), 1) + "°", "HSSbat");
				}
				else {
					GP.PLAIN("err", "HSSbat");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Boiler:");

				if (!solar->getBoilerSensorStatus()) {
					GP.PLAIN(String(solar->getBoilerT(), 1) + "°", "HSSboi");
				}
				else {
					GP.PLAIN("err", "HSSboi");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Exit:");

				if (!solar->getExitSensorStatus()) {
					GP.PLAIN(String(solar->getExitT(), 1) + "°", "HSSext");
				}
				else {
					GP.PLAIN("err", "HSSext");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Pump:");
				GP.SWITCH("HSSpu", solar->getReleFlag());
			);
		);

		GP.HR();
		GP.SPAN("Solar Battery Control System", GP_LEFT);
		GP.SPAN("Author: Vereshchynskyi Nazar", GP_LEFT);
		GP.SPAN("Version: 1.3.1", GP_LEFT);
		GP.SPAN("Date: 04.02.2025", GP_LEFT);
	}

	if (ui.uri("/settings")) {
		M_SPOILER("Network", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Mode:");
				GP.SELECT("SNm", "off,sta,ap_sta,auto", network->getMode());
			);
			
			M_FORM2("/SNW",
				M_BLOCK(GP_THIN,
					GP.TITLE("WiFi");
					GP.TEXT("SNWs", "ssid", network->getWifiSsid(), "50%", NETWORK_SSID_PASS_SIZE);
					GP.PASS_EYE("SNWp", "pass", "", "", NETWORK_SSID_PASS_SIZE);
					GP.BREAK();
					GP.SUBMIT_MINI(" OK ", GP_ORANGE);
				);
			);
			M_BLOCK(GP_THIN,
				GP.TITLE("AP");
				GP.TEXT("SNAs", "ssid", network->getApSsid(), "50%", NETWORK_SSID_PASS_SIZE);
				GP.PASS_EYE("SNAp", "pass", network->getApPass(), "", NETWORK_SSID_PASS_SIZE);
			);
		);
		GP.BREAK();

		M_SPOILER("Blynk", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Status:");
				GP.SWITCH("SBs", blynk->getWorkFlag());
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Send data time:");
				GP.NUMBER("SBsdt", "time", blynk->getSendDataTime(), "25%");
				GP.PLAIN("sec");
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Auth:");
				GP.TEXT("SBa", "auth", blynk->getAuth(), "100%", BLYNK_AUTH_SIZE);
			);

			M_BLOCK(GP_THIN,
				GP.TITLE("Links");
				GP.BUTTON("SBLs", "Scan", "", GP_ORANGE, "45%", false, true);

				for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
					M_BOX(GP_LEFT,
						char* link_element_code = blynk->getLinkElementCode(i);
						uint8_t index = system->scanBlynkElemetCodeIndex(&web_blynk.element_codes, link_element_code);

						GP.LABEL("V");
						GP.NUMBER(String("SBLp") + i, "port", blynk->getLinkPort(i), "30%");
						GP.SELECT(String("SBLe") + i, web_blynk.element_codes_string, index);
						GP.BUTTON(String("SBLd") + i, "Delete", "", GP_ORANGE, "20%", false, true);
					);
				}

				GP.BUTTON("SBLnl", "New link", "", GP_ORANGE, "45%", false, true);
			);
		);
		GP.BREAK();

		M_SPOILER("Time", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Ntp sync:");
				GP.SWITCH("STns", time->getNtpFlag());
			);

			M_BOX(GP_LEFT,
				GP.LABEL("Gmt:");
				GP.NUMBER("STg", "gmt", time->getGmt(), "25%");
			);
			
			if (!time->getNtpFlag()) {
				GP.TIME("STt", GPtime((uint32_t)time->getUnix(), time->getGmt()) );
				GP.DATE("STd", GPdate((uint32_t)time->getUnix(), time->getGmt()) );
			}
		);
		GP.BREAK();

		M_SPOILER("Sensors", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Read data time:");
				GP.NUMBER("SSrdt", "time", sensors->getReadDataTime(), "25%");
				GP.PLAIN("sec");
			);

			M_BLOCK(GP_THIN,
				GP.TITLE("DS18B20");
				GP.BUTTON("SSDSs", "Scan", "", GP_ORANGE, "45%", false, true);

				for (byte i = 0;i < sensors->getDS18B20Count();i++) {
					M_BLOCK(GP_THIN, 
						M_BOX(GP_CENTER,
							GP.TEXT(String("SSDSn") + i, "", sensors->getDS18B20Name(i), "17%", 2);
						);

						M_BOX(GP_LEFT,
							uint8_t* ds18b20_address = sensors->getDS18B20Address(i);
							uint8_t index = sensors->scanDS18B20AddressIndex(&web_sensors.ds18b20_addresses, ds18b20_address);
							
							GP.LABEL("Address:");
							GP.SELECT(String("SSDSa") + i, web_sensors.ds18b20_addresses_string, index);
						);
						
						M_BOX(GP_LEFT,
							GP.LABEL("Resolution:");
							GP.NUMBER(String("SSDSr") + i, "", sensors->getDS18B20Resolution(i), "25%");
							GP.PLAIN("bit");
						);

						M_BOX(GP_LEFT,
							GP.LABEL("Correction:");
							GP.NUMBER_F(String("SSDSc") + i, "", sensors->getDS18B20Correction(i), 2, "25%");
							GP.PLAIN("°");
						);

						GP.BUTTON(String("SSDSd") + i, "Delete", "", GP_ORANGE, "20%", false, true);
					);
				}

				GP.BUTTON("SSDSnd", "New ds18b20", "", GP_ORANGE, "45%", false, true);
			);
		);
		GP.BREAK();

		M_SPOILER("Display", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Auto reset:");
				GP.SWITCH("SDar", display->getAutoResetFlag());
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Backlight off time:");
				GP.NUMBER("SDbot", "time", display->getBacklightOffTime(), "25%");
				GP.PLAIN("sec");
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Fps:");
				GP.NUMBER("SDf", "fps", display->getFps(), "25%");
				GP.PLAIN("fps");
			);
		);
		GP.BREAK();

		M_SPOILER("Solar system", GP_ORANGE,
			char select_array[20] = "NONE,";

			for (uint8_t i = 0;i < sensors->getDS18B20Count();i++) {
				strcat(select_array, sensors->getDS18B20Name(i));
				
				if (i != sensors->getDS18B20Count() - 1) {
					strcat(select_array, ",");
				}
			}
			
			M_BOX(GP_LEFT,
				GP.LABEL("Status:");
				GP.SWITCH("SSSs", solar->getWorkFlag());
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Error on:");
				GP.SWITCH("SSSeo", solar->getErrorOnFlag());
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Rele invert:");
				GP.SWITCH("SSSri", solar->getReleInvertFlag());
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Delta:");
				GP.NUMBER("SSSd", "delta", solar->getDelta(), "25%");
				GP.PLAIN("°");
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Battery:");
				GP.SELECT("SSSba", select_array, solar->getBatterySensor() + 1);
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Boiler:");
				GP.SELECT("SSSbo", select_array, solar->getBoilerSensor() + 1);
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Exit:");
				GP.SELECT("SSSex", select_array, solar->getExitSensor() + 1);
			);
		);
		GP.BREAK();

		M_SPOILER("System", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Buzzer:");
				GP.SWITCH("SSb", system->getBuzzerFlag());
			);
			
			M_BLOCK(GP_THIN,
				GP.TITLE("Management");

				GP.BUTTON("SSMr", "RESET", "", GP_ORANGE, "45%");
				GP.BUTTON("SSMa", "ALL", "", GP_ORANGE, "45%");
				GP.BUTTON_LINK("/ota_update", "OTA", GP_YELLOW, "45%");
			);
		);
	}

	if (ui.uri("/memory")) {
		GP.FILE_MANAGER(&LittleFS);
		GP.FILE_UPLOAD("file");
	}

	GP.BUILD_END();
}

void NetworkManager::uiAction() {
	if (system == NULL) {
		return;
	}
	
	TimeManager* time = system->getTimeManager();
	SensorsManager* sensors = system->getSensorsManager();
	SolarSystemManager* solar = system->getSolarSystemManager();
	DisplayManager* display = system->getDisplayManager();
	NetworkManager* network = system->getNetworkManager();
	BlynkManager* blynk = system->getBlynkManager();

	/* --- Home --- */
	// update
	if (ui.update("HSt")) {
		ui.answer(!sensors->getAM2320Status() ? String(sensors->getAM2320T(), 1) + "°" : String("err"));
		return;
	}
	if (ui.update("HSh")) {
		ui.answer(!sensors->getAM2320Status() ? String(sensors->getAM2320H(), 1) + "%" : String("err"));
		return;
	}
	
	for (byte i = 0;i < sensors->getDS18B20Count();i++) {
		if (ui.update(String("HSdsn") + i)) {
			ui.answer(sensors->getDS18B20Name(i));

			return;
		}
		if (ui.update(String("HSdst") + i)) {
			ui.answer(!sensors->getDS18B20Status(i) ? String(sensors->getDS18B20T(i), 1) + "°" : String("err"));
			return;
		}
	}

	if (ui.update("HSSbat")) {
		ui.answer(!solar->getBatterySensorStatus() ? String(solar->getBatteryT(), 1) + "°" : String("err"));
		return;
	}
	if (ui.update("HSSboi")) {
		ui.answer(!solar->getBoilerSensorStatus() ? String(solar->getBoilerT(), 1) + "°" : String("err"));
		return;
	}
	if (ui.update("HSSext")) {
		ui.answer(!solar->getExitSensorStatus() ? String(solar->getExitT(), 1) + "°" : String("err"));
		return;
	}
	if (ui.update("HSSpu")) {
		ui.answer(solar->getReleFlag());
		return;
	}

	// parse
	if (ui.click("HSSpu")) {
		bool state = ui.getBool();
		solar->setReleFlag(state);
		
		return;
	}
	/* --- Home --- */

	if (ui.clickSub("S") || ui.formSub("/S")) {
		system->saveSettingsRequest();
	}

	/* --- NetworkManager --- */
	// update
	if (ui.update("SNm")) {
		ui.answer(network->getMode());
		return;
	}

	if (ui.update("SNWs")) {
		ui.answer(network->getWifiSsid());
		return;
	}

	if (ui.update("SNAs")) {
		ui.answer(network->getApSsid());
		return;
	}
	if (ui.update("SNAp")) {
		ui.answer(network->getApPass());
		return;
	}

	// parse
	if (ui.click("SNm")) {
		network->setMode(ui.getInt());
		return;
	}

	if (ui.form("/SNW")) {
		char read_ssid[NETWORK_SSID_PASS_SIZE];
		char read_pass[NETWORK_SSID_PASS_SIZE];

		ui.copyStr("SNWs", read_ssid, NETWORK_SSID_PASS_SIZE);
		ui.copyStr("SNWp", read_pass, NETWORK_SSID_PASS_SIZE);

		network->setWifi(read_ssid, read_pass);
		return;
	}

	if (ui.click("SNAs")) {
		String read_string(ui.getString());
		network->setAp(&read_string, NULL);
		
		return;
	}
	if (ui.click("SNAp")) {
		String read_string(ui.getString());
		network->setAp(NULL, &read_string);
		
		return;
	}
	/* --- NetworkManager --- */

	/* --- BlynkManager --- */
	// update
	if (ui.update("SBs")) {
		ui.answer(blynk->getWorkFlag());
		return;
	}
	if (ui.update("SBsdt")) {
		ui.answer(blynk->getSendDataTime());
		return;
	}
	if (ui.update("SBa")) {
		ui.answer(blynk->getAuth());
		return;
	}

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		if (ui.update(String("SBLp") + i)) {
			ui.answer(blynk->getLinkPort(i));

			return;
		}
		if (ui.update(String("SBLe") + i)) {
			char* link_element_code = blynk->getLinkElementCode(i);
			uint8_t index = system->scanBlynkElemetCodeIndex(&web_blynk.element_codes, link_element_code);
		
			ui.answer(index);
			return;
		}
	}

	// parse
	if (ui.click("SBs")) {
		blynk->setWorkFlag(ui.getBool());
		return;
	}
	if (ui.click("SBsdt")) {
		blynk->setSendDataTime(ui.getInt());
		return;
	}
	if (ui.click("SBa")) {
		blynk->setAuth(ui.getString());
		return;
	}

	if (ui.click("SBLs")) {
		updateWebBlynkBlock();
		return;
	}
	if (ui.click("SBLnl")) {
		blynk->addLink();
		return;
	}
	
	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		if (ui.click(String("SBLp") + i)) {
			blynk->setLinkPort(i, ui.getInt());
			
			return;
		}
		if (ui.click(String("SBLe") + i)) {
			uint8_t index = ui.getInt();

			if (index < web_blynk.element_codes.size()) {
				blynk->setLinkElementCode(i, web_blynk.element_codes[index]);
			}
			
			return;
		}
		if (ui.click(String("SBLd") + i)) {
			blynk->deleteLink(i);
			return;
		}
	}
	/* --- BlynkManager --- */

	/* --- TimeManager --- */
	// update
	if (ui.update("STns")) {
		ui.answer(time->getNtpFlag());
		return;
	}
	if (ui.update("STg")) {
		ui.answer(time->getGmt());
		return;
	}

	// parse
	if (ui.click("STns")) {
		time->setNtpFlag(ui.getBool());
		return;
	}
	if (ui.click("STg")) {
		time->setGmt(constrain(ui.getInt(), -12, 12));
		return;
	}

	if (ui.click("STt")) {
		GPtime get_time = ui.getTime();
		time->setTime(get_time.hour, get_time.minute, get_time.second, time->day(), time->month(), time->year());
		
		return;
	}
	if (ui.click("STd")) {
		GPdate get_date = ui.getDate();
		time->setTime(time->hour(), time->minute(), time->second(), get_date.day, get_date.month, get_date.year);
		
		return;
	}
	/* --- TimeManager --- */

	/* --- SensorsManager --- */
	// update
	if (ui.update("SSrdt")) {
		ui.answer(sensors->getReadDataTime());
		return;
	}

	for (byte i = 0;i < sensors->getDS18B20Count();i++) {
		if (ui.update(String("SSDSn") + i)) {
			ui.answer(sensors->getDS18B20Name(i));

			return;
		}

		if (ui.update(String("SSDSa") + i)) {
			uint8_t* ds18b20_address = sensors->getDS18B20Address(i);
			uint8_t index = sensors->scanDS18B20AddressIndex(&web_sensors.ds18b20_addresses, ds18b20_address);
											
			ui.answer(index);
			return;
		}

		if (ui.update(String("SSDSr") + i)) {
			ui.answer(sensors->getDS18B20Resolution(i));
			return;
		}

		if (ui.update(String("SSDSc") + i)) {
			ui.answer(sensors->getDS18B20Correction(i), 1);
			return;
		}

	}

	// parse
	if (ui.click("SSrdt")) {
		sensors->setReadDataTime(ui.getInt());
		return;
	}
	if (ui.click("SSDSs")) {
		updateWebSensorsBlock();
		return;
	}
	if (ui.click("SSDSnd")) {
		sensors->addDS18B20();
		return;
	}

	for (byte i = 0;i < sensors->getDS18B20Count();i++) {
		if (ui.click(String("SSDSn") + i)) {
			sensors->setDS18B20Name(i, ui.getString());

			return;
		}

		if (ui.click(String("SSDSa") + i)) {
			uint8_t index = ui.getInt();

			if (index < web_sensors.ds18b20_addresses.size()) {
				sensors->setDS18B20Address(i, web_sensors.ds18b20_addresses[index]);
			}
			
			return;
		}

		if (ui.click(String("SSDSr") + i)) {
			sensors->setDS18B20Resolution(i, ui.getInt());
			return;
		}

		if (ui.click(String("SSDSc") + i)) {
			sensors->setDS18B20Correction(i, ui.getFloat());
			return;
		}

		if (ui.click(String("SSDSd") + i)) {
			sensors->deleteDS18B20(i);
			return;
		}
	}
	/* --- SensorsManager --- */

	/* --- DisplayManager --- */
	// update
	if (ui.update("SDar")) {
		ui.answer(display->getAutoResetFlag());
		return;
	}
	if (ui.update("SDbot")) {
		ui.answer(display->getBacklightOffTime());
		return;
	}
	if (ui.update("SDf")) {
		ui.answer(display->getFps());
		return;
	}

	// parse
	if (ui.click("SDar")) {
		display->setAutoResetFlag(ui.getBool());
		return;
	}
	if (ui.click("SDbot")) {
		display->setBacklightOffTime(ui.getInt());
		return;
	}
	if (ui.click("SDf")) {
		display->setFps(ui.getInt());
		return;
	}
	/* --- DisplayManager --- */

	/* --- SolarSystemManager --- */
	// update
	if (ui.update("SSSs")) {
		ui.answer(solar->getWorkFlag());
		return;
	}
	if (ui.update("SSSeo")) {
		ui.answer(solar->getErrorOnFlag());
		return;
	}
	if (ui.update("SSSri")) {
		ui.answer(solar->getReleInvertFlag());
		return;
	}

	if (ui.update("SSSd")) {
		ui.answer(solar->getDelta());
		return;
	}

	if (ui.update("SSSba")) {
		ui.answer(solar->getBatterySensor() + 1);
		return;
	}
	if (ui.update("SSSbo")) {
		ui.answer(solar->getBoilerSensor() + 1);
		return;
	}
	if (ui.update("SSSex")) {
		ui.answer(solar->getExitSensor() + 1);
		return;
	}

	// parse
	if (ui.click("SSSs")) {
		solar->setWorkFlag(ui.getBool());
		return;
	}
	if (ui.click("SSSeo")) {
		solar->setErrorOnFlag(ui.getBool());
		return;
	}
	if (ui.click("SSSri")) {
		solar->setReleInvertFlag(ui.getBool());
		return;
	}

	if (ui.click("SSSd")) {
		solar->setDelta(ui.getInt());
		return;
	}
	
	if (ui.click("SSSba")) {
		solar->setBatterySensor(ui.getInt() - 1);
		return;
	}
	if (ui.click("SSSbo")) {
		solar->setBoilerSensor(ui.getInt() - 1);
		return;
	}
	if (ui.click("SSSex")) {
		solar->setExitSensor(ui.getInt() - 1);
		return;
	}
	/* --- SolarSystemManager --- */

	/* --- SystemManager --- */
	// update
	if (ui.update("SSb")) {
		ui.answer(system->getBuzzerFlag());
		return;
	}

	// parse
	if (ui.click("SSb")) {
		system->setBuzzerFlag(ui.getBool());
		return;
	}	

	if (ui.click("SSMr")) {
		ESP.reset();
	}
	if (ui.click("SSMa")) {
		system->resetAll();
	}
	/* --- SystemManager --- */
}


void NetworkManager::updateWebBlynkBlock() {
	web_blynk.element_codes_string.clear();
	system->makeBlynkElementCodesList(&web_blynk.element_codes);

	for (uint8_t i = 0;i < web_blynk.element_codes.size();i++) {
		web_blynk.element_codes_string += web_blynk.element_codes[i];

		if (i != web_blynk.element_codes.size() - 1) {
			web_blynk.element_codes_string += ',';
		}
	}
}

void NetworkManager::updateWebSensorsBlock() {
	SensorsManager* sensors = system->getSensorsManager();
	DynamicArray<String> addresses;

	web_sensors.ds18b20_addresses_string.clear();
	sensors->makeDS18B20AddressList(&web_sensors.ds18b20_addresses, NULL, &addresses);

	for (uint8_t i = 0;i < addresses.size();i++) {
		web_sensors.ds18b20_addresses_string += addresses[i];

		if (i != addresses.size() - 1) {
			web_sensors.ds18b20_addresses_string += ',';
		}
	}
}

String NetworkManager::web_update_codes = String();
NetworkManager::web_sensors_block_t NetworkManager::web_sensors = web_sensors_block_t();
NetworkManager::web_blynk_block_t NetworkManager::web_blynk = web_blynk_block_t();