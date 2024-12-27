/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.2.0
 * Date: 27.12.2024
 */

#include "data.h"

void NetworkManager::uiBuild() {
	if (system == NULL) {
		return;
	}

	static uint32_t rebuild_timer = 0;
	String update_codes;

	TimeManager* time = system->getTimeManager();
	ModuleManager* modules = system->getModuleManager();
	SolarSystemManager* solar = system->getSolarSystemManager();
	DisplayManager* display = system->getDisplayManager();
	NetworkManager* network = system->getNetworkManager();
	BlynkManager* blynk = system->getBlynkManager();

	if (millis() - rebuild_timer < WEB_REBUILD_TIME) {
		return;
	}
	rebuild_timer = millis();

	update_codes = "HSt,HSh,";
	for (byte i = 0;i < modules->getDS18B20Count();i++) {
		update_codes += "HSdsn";
		update_codes += i;
		update_codes += ",";
		update_codes += "HSdst";
		update_codes += i;
		update_codes += ",";
	}

	update_codes += "HSSbat,HSSbot,HSSext,HSSpu,";
	update_codes += "SNm,SNWs,SNAs,SNAp,SBs,SBsdt,SBa,";

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		update_codes += "SBLp";
		update_codes += i;
		update_codes += ",";
		update_codes += "SBLe";
		update_codes += i;
		update_codes += ",";
	}

	update_codes += "STg,STns,SMrdt,";

	for (byte i = 0;i < modules->getDS18B20Count();i++) {
		update_codes += "SMDSn";
		update_codes += i;
		update_codes += ",";
		update_codes += "SMDSc";
		update_codes += i;
		update_codes += ",";
		update_codes += "SMDSra";
		update_codes += i;
		update_codes += ",";
	}

	update_codes += "SDar,SDbot,SDf,SSSs,SSSeo,SSSri,SSSd,SSSba,SSSbo,SSSex,SSb";
	
	GP.BUILD_BEGIN(550);
	GP.THEME(GP_DARK);
	GP.UPDATE(update_codes, SEC_TO_MLS(WEB_UPDATE_TIME));
	
	GP.TITLE("nazotronic");
	GP.NAV_TABS_LINKS("/,/settings,/memory", "Home,Settings,Memory", GP_ORANGE);
	GP.HR();

	if (ui.uri("/")) {
		M_SPOILER("Info", GP_ORANGE,
			GP.SYSTEM_INFO("1.2.0");
		);
		
		M_BLOCK(GP_THIN,
			GP.LABEL("Sensors");
			
			M_BOX(GP_LEFT,
				GP.LABEL("T:");

				if (!modules->getAM2320Status()) {
					GP.PLAIN(String(modules->getAM2320T(), 1) + "°", "HSt");
				}
				else {
					GP.PLAIN("err", "HSt");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("H:");

				if (!modules->getAM2320Status()) {
					GP.PLAIN(String(modules->getAM2320H(), 1) + "%", "HSh");
				}
				else {
					GP.PLAIN("err", "HSh");
				}
			);

			for (byte i = 0;i < modules->getDS18B20Count();i++) {
				M_BOX(GP_LEFT,
					GP.LABEL(modules->getDS18B20Name(i), String("HSdsn") + i);
					GP.LABEL(":");
					
					if (!modules->getDS18B20Status(i)) {
						GP.PLAIN(String(modules->getDS18B20T(i), 1) + "°", String("Hdst") + i);
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
					GP.PLAIN(String(solar->getBoilerT(), 1) + "°", "HSSbot");
				}
				else {
					GP.PLAIN("err", "HSSbot");
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
		GP.SPAN("Version: 1.2.0", GP_LEFT);
		GP.SPAN("Date: 27.12.2024", GP_LEFT);
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
			String blynk_elements;

			for (uint8_t i = 0;i < blynk->getElementsCount();i++) {
				blynk_elements += blynk->getElementName(i);

				if (i != blynk->getElementsCount() - 1) {
					blynk_elements += ',';
				}
			}
				
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

				for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
					M_BOX(GP_LEFT,
						GP.LABEL("V");
						GP.NUMBER(String("SBLp") + i, "port", blynk->getLinkPort(i), "30%");
						GP.SELECT(String("SBLe") + i, blynk_elements, blynk->getLinkElement(i));
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
		
		M_SPOILER("Modules", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Read data time:");
				GP.NUMBER("SMrdt", "time", modules->getReadDataTime(), "25%");
				GP.PLAIN("sec");
			);

			M_BLOCK(GP_THIN, 
				GP.TITLE("DS18B20");

				for (byte i = 0;i < modules->getDS18B20Count();i++) {
					M_BOX(GP_LEFT,
						GP.TEXT(String("SMDSn") + i, "", modules->getDS18B20Name(i), "17%", 2);
						GP.LABEL(":");
						GP.NUMBER_F(String("SMDSc") + i, "", modules->getDS18B20Correction(i), 2, "25%");
						GP.PLAIN("°");
						GP.NUMBER(String("SMDSra") + i, "", modules->getDS18B20ReadAttempts(i), "25%");
						GP.PLAIN("✔");
					);
				}
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

			for (uint8_t i = 0;i < modules->getDS18B20Count();i++) {
				strcat(select_array, modules->getDS18B20Name(i));
				
				if (i != modules->getDS18B20Count() - 1) {
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
	ModuleManager* modules = system->getModuleManager();
	SolarSystemManager* solar = system->getSolarSystemManager();
	DisplayManager* display = system->getDisplayManager();
	NetworkManager* network = system->getNetworkManager();
	BlynkManager* blynk = system->getBlynkManager();

	const char* solar_sensors_names[] = {"SSSba", "SSSbo", "SSSex"};

	// update
	if (ui.update("HSt")) {
		ui.answer(!modules->getAM2320Status() ? String(modules->getAM2320T(), 1) + "°" : String("err"));
	}
	if (ui.update("HSh")) {
		ui.answer(!modules->getAM2320Status() ? String(modules->getAM2320H(), 1) + "%" : String("err"));
	}
	
	for (byte i = 0;i < modules->getDS18B20Count();i++) {
		if (ui.update(String("HSdsn") + i)) {
			ui.answer(modules->getDS18B20Name(i));
		}
		if (ui.update(String("HSdst") + i)) {
			ui.answer(!modules->getDS18B20Status(i) ? String(modules->getDS18B20T(i), 1) + "°" : String("err"));
		}
	}

	if (ui.update("HSSba")) {
		ui.answer(!solar->getBatterySensorStatus() ? String(solar->getBatteryT(), 1) + "°" : String("err"));
	}
	if (ui.update("HSSbo")) {
		ui.answer(!solar->getBoilerSensorStatus() ? String(solar->getBoilerT(), 1) + "°" : String("err"));
	}
	if (ui.update("HSSex")) {
		ui.answer(!solar->getExitSensorStatus() ? String(solar->getExitT(), 1) + "°" : String("err"));
	}
	if (ui.update("HSSpu")) {
		ui.answer(solar->getReleFlag());
	}

	// parse
	if (ui.click("HSSpu")) {
		bool state = ui.getBool();
		solar->setReleFlag(state);
	}


	// update
	if (ui.update("SNm")) {
		ui.answer(network->getMode());
	}
	if (ui.update("SNWs")) {
		ui.answer(network->getWifiSsid());
	}
	if (ui.update("SNAs")) {
		ui.answer(network->getApSsid());
	}
	if (ui.update("SNAp")) {
		ui.answer(network->getApPass());
	}

	if (ui.update("SBs")) {
		ui.answer(blynk->getWorkFlag());
	}
	if (ui.update("SBsdt")) {
		ui.answer(blynk->getSendDataTime());
	}
	if (ui.update("SBa")) {
		ui.answer(blynk->getAuth());
	}

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		if (ui.update(String("SBLp") + i)) {
			ui.answer(blynk->getLinkPort(i));
		}
		if (ui.update(String("SBLe") + i)) {
			ui.answer(blynk->getLinkElement(i));
		}
	}

	if (ui.update("STns")) {
		ui.answer(time->getNtpFlag());
	}
	if (ui.update("STg")) {
		ui.answer(time->getGmt());
	}

	if (ui.update("SMrdt")) {
		ui.answer(modules->getReadDataTime());
	}

	for (byte i = 0;i < modules->getDS18B20Count();i++) {
		if (ui.update(String("SMDSn") + i)) {
			ui.answer(modules->getDS18B20Name(i));
		}

		if (ui.update(String("SMDSc") + i)) {
			ui.answer(modules->getDS18B20Correction(i), 1);
		}

		if (ui.update(String("SMDSra") + i)) {
			ui.answer(modules->getDS18B20ReadAttempts(i));
		}
	}

	if (ui.update("SDar")) {
		ui.answer(display->getAutoResetFlag());
	}
	if (ui.update("SDbot")) {
		ui.answer(display->getBacklightOffTime());
	}
	if (ui.update("SDf")) {
		ui.answer(display->getFps());
	}

	if (ui.update("SSSs")) {
		ui.answer(solar->getWorkFlag());
	}
	if (ui.update("SSSeo")) {
		ui.answer(solar->getErrorOnFlag());
	}
	if (ui.update("SSSri")) {
		ui.answer(solar->getReleInvertFlag());
	}
	if (ui.update("SSSd")) {
		ui.answer(solar->getDelta());
	}

	for (uint8_t i = 0;i < 3;i++) {
		if (ui.update(solar_sensors_names[i])) {
			ui.answer(solar->getSensor(i) + 1);
		}
	}

	if (ui.update("SSb")) {
		ui.answer(system->getBuzzerFlag());
	}

	//parse
	if (ui.clickSub("S") || ui.formSub("/S")) {
		system->saveSettingsRequest();
	}

	if (ui.click("SNm")) {
		network->setMode(ui.getInt());
	}
	if (ui.form("/SNW")) {
		char read_ssid[NETWORK_SSID_PASS_SIZE];
		char read_pass[NETWORK_SSID_PASS_SIZE];

		ui.copyStr("SNWs", read_ssid, NETWORK_SSID_PASS_SIZE);
		ui.copyStr("SNWp", read_pass, NETWORK_SSID_PASS_SIZE);

		network->setWifi(read_ssid, read_pass);
	}
	if (ui.click("SNAs")) {
		String read_string(ui.getString());
		network->setAp(&read_string, NULL);
	}
	if (ui.click("SNAp")) {
		String read_string(ui.getString());
		network->setAp(NULL, &read_string);
	}

	if (ui.click("SBs")) {
		blynk->setWorkFlag(ui.getBool());
	}
	if (ui.click("SBsdt")) {
		blynk->setSendDataTime(ui.getInt());
	}
	if (ui.click("SBa")) {
		blynk->setAuth(ui.getString());
	}

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		if (ui.click(String("SBLp") + i)) {
			blynk->setLinkPort(i, ui.getInt());
		}
		if (ui.click(String("SBLe") + i)) {
			blynk->setLinkElement(i, ui.getInt());
		}
		if (ui.click(String("SBLd") + i)) {
			blynk->delLink(i);
		}
	}

	if (ui.click("SBLnl")) {
		blynk->addLink();
	}

	if (ui.click("STns")) {
		time->setNtpFlag(ui.getBool());
	}
	if (ui.click("STg")) {
		time->setGmt(constrain(ui.getInt(), -12, 12));
	}
	if (ui.click("STt")) {
		GPtime get_time = ui.getTime();
		time->setTime(get_time.hour, get_time.minute, get_time.second, time->day(), time->month(), time->year());
	}
	if (ui.click("STd")) {
		GPdate get_date = ui.getDate();
		time->setTime(time->hour(), time->minute(), time->second(), get_date.day, get_date.month, get_date.year);
	}

	if (ui.click("SMrdt")) {
		modules->setReadDataTime(ui.getInt());
	}

	for (byte i = 0;i < modules->getDS18B20Count();i++) {
		if (ui.click(String("SMDSn") + i)) {
			modules->setDS18B20Name(i, ui.getString());
		}

		if (ui.click(String("SMDSc") + i)) {
			modules->setDS18B20Correction(i, ui.getFloat());
		}

		if (ui.click(String("SMDSra") + i)) {
			modules->setDS18B20ReadAttempts(i, ui.getInt());
		}
	}
	
	if (ui.click("SDar")) {
		display->setBacklightOffTime(ui.getBool());
	}
	if (ui.click("SDbot")) {
		display->setBacklightOffTime(ui.getInt());
	}
	if (ui.click("SDf")) {
		display->setFps(ui.getInt());
	}

	if (ui.click("SSSs")) {
		solar->setWorkFlag(ui.getBool());
	}
	if (ui.click("SSSeo")) {
		solar->setErrorOnFlag(ui.getBool());
	}
	if (ui.click("SSSri")) {
		solar->setReleInvertFlag(ui.getBool());
	}
	if (ui.click("SSSd")) {
		solar->setDelta(ui.getInt());
	}
	
	for (uint8_t i = 0;i < 3;i++) {
		if (ui.click(solar_sensors_names[i])) {
			solar->setSensor(i, ui.getInt() - 1);
		}
	}
	
	if (ui.click("SSb")) {
		system->setBuzzerFlag(ui.getBool());
	}	

	if (ui.click("SSMr")) {
		ESP.reset();
	}
	if (ui.click("SSMa")) {
		system->resetAll();
	}
}