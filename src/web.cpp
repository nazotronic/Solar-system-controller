/*
 * File: web.cpp
 * 
 * Description:
 * This file handles the web interface functionality.
 * It defines the code necessary,
 * for interacting with the web interface, allowing users to configure and monitor the system remotely.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Date: 02.09.2024
 */

#include "data.h"

void uiBuild() {
	static uint32_t rebuild_timer = 0;
	String update_codes;

	if (millis() - rebuild_timer < WEB_REBUILD_TIME) {
		return;
	}
	rebuild_timer = millis();

	update_codes = "HSt,HSh,";
	for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
		update_codes += "HStn";
		update_codes += i;
		update_codes += ",";
		update_codes += "HSt";
		update_codes += i;
		update_codes += ",";
	}

	update_codes += "HSSba,HSSbo,HSSex,HSSpu,";
	update_codes += "SNm,SNWs,SNAs,SNAp,SNBs,SNBa,";

	for (uint8_t i = 0;i < blynk.getLinksCount();i++) {
		update_codes += "SNBLp";
		update_codes += i;
		update_codes += ",";
		update_codes += "SNBLe";
		update_codes += i;
		update_codes += ",";
	}
							
	update_codes += "SSSs,SSSo,SSSd,SSSba,SSSbo,SSSex,";
	update_codes += "SStd,SStds,SSdf,SSb,SSri,";

	for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
		update_codes += "SSDtn";
		update_codes += i;
		update_codes += ",";
		update_codes += "SSDtc";
		update_codes += i;
		update_codes += ",";
	}

	update_codes += "SSTg,SSTns";
	
	GP.BUILD_BEGIN(550);
	GP.THEME(GP_DARK);
	GP.UPDATE(update_codes, WEB_UPDATE_TIME);
	
	GP.TITLE("nazotronic");
	GP.NAV_TABS_LINKS("/,/settings,/memory", "Home,Settings,Memory", GP_ORANGE);
	GP.HR();

	if (ui.uri("/")) {
		M_SPOILER("Info", GP_ORANGE,
			GP.SYSTEM_INFO("1.0.0");
		);
		
		M_BLOCK(GP_THIN,
			GP.LABEL("Sensors");
			
			M_BOX(GP_LEFT,
				GP.LABEL("T:");
				if (!problem[1]) {
					GP.PLAIN(String(t_home, 1) + "°", "HSt");
				}
				else {
					GP.PLAIN("err", "HSt");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("H:");
				if (!problem[1]) {
					GP.PLAIN(String(h_home, 1) + "%", "HSh");
				}
				else {
					GP.PLAIN("err", "HSh");
				}
			);

			for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
				M_BOX(GP_LEFT,
					GP.LABEL(ds_sensors[i].name, String("HStn") + i);
					GP.LABEL(":");
					
					if (!ds_sensors[i].problem) {
						GP.PLAIN(String(ds_sensors[i].t, 1) + "°", String("HSt") + i);
					}
					else {
						GP.PLAIN("err", String("HSt") + i);
					}
				);
			}
		);

		M_BLOCK(GP_THIN,
			GP.LABEL("Solar system");
			
			M_BOX(GP_LEFT,
				GP.LABEL("Battery:");
				if (!solar.batterySensor()) {
					GP.PLAIN(String(solar.getBatteryT(), 1) + "°", "HSSba");
				}
				else {
					GP.PLAIN("err", "HSSba");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Boiler:");
				if (!solar.boilerSensor()) {
					GP.PLAIN(String(solar.getBoilerT(), 1) + "°", "HSSbo");
				}
				else {
					GP.PLAIN("err", "HSSbo");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Exit:");
				if (!solar.exitSensor()) {
					GP.PLAIN(String(solar.getExitT(), 1) + "°", "HSSex");
				}
				else {
					GP.PLAIN("err", "HSSex");
				}
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Pump:");
				GP.SWITCH("HSSpu", getRele());
			);
		);

		GP.HR();
		GP.SPAN("Project: Solar Battery Control System", GP_LEFT);
		GP.SPAN("Author: Nazarii Vereshchynskyi", GP_LEFT);
		GP.SPAN("Email: verechnazar12@gmail.com", GP_LEFT);
		GP.SPAN("Version: 1.0.0", GP_LEFT);
		GP.SPAN("Date: 02.09.2024", GP_LEFT);
	}

	if (ui.uri("/settings")) {
		M_SPOILER("Network", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Mode:");
				GP.SELECT("SNm", "off,sta,ap_sta,auto", network.getMode());
			);
			
			M_FORM2("/SNW",
				M_BLOCK(GP_THIN,
					GP.TITLE("WiFi");
					GP.TEXT("SNWs", "ssid", network.getWifiSsid(), "50%", NETWORK_SSID_PASS_SIZE);
					GP.PASS_EYE("SNWp", "pass", "", "", NETWORK_SSID_PASS_SIZE);
					GP.BREAK();
					GP.SUBMIT_MINI(" OK ", GP_ORANGE);
				);
			);
			M_BLOCK(GP_THIN,
				GP.TITLE("AP");
				GP.TEXT("SNAs", "ssid", network.getApSsid(), "50%", NETWORK_SSID_PASS_SIZE);
				GP.PASS_EYE("SNAp", "pass", network.getApPass(), "", NETWORK_SSID_PASS_SIZE);
			);
			M_BLOCK(GP_THIN,
				GP.TITLE("Blynk");
				String blynk_elements;

				for (uint8_t i = 0;i < blynk.getElementsCount();i++) {
					blynk_elements += blynk.getElementName(i);

					if (i != blynk.getElementsCount() - 1) {
						blynk_elements += ',';
					}
				}
				
				M_BOX(GP_LEFT,
					GP.LABEL("Status:");
					GP.SWITCH("SNBs", blynk.getWorkStatus());
				);
				M_BOX(GP_LEFT,
					GP.LABEL("Auth:");
					GP.TEXT("SNBa", "auth", blynk.getAuth(), "100%", BLYNK_AUTH_SIZE);
				);

				M_BLOCK(GP_THIN,
					GP.TITLE("Links");

					for (uint8_t i = 0;i < blynk.getLinksCount();i++) {
						M_BOX(GP_LEFT,
							GP.LABEL("V");
							GP.NUMBER(String("SNBLp") + i, "port", blynk.getLinkPort(i), "25%");
							GP.SELECT(String("SNBLe") + i, blynk_elements, blynk.getLinkElement(i));
							GP.BUTTON(String("SNBLd") + i, "Delete", "", GP_ORANGE, "20%", false, true);
						);
					}

					GP.BUTTON("SNBLnl", "New link", "", GP_ORANGE, "45%", false, true);
				);
			);
		);
		GP.BREAK();

		M_SPOILER("Solar system", GP_ORANGE,
			char select_array[20] = "NONE,";

			for (uint8_t i = 0;i < DS_SENSORS_COUNT;i++) {
				strcat(select_array, ds_sensors[i].name);
				
				if (i != DS_SENSORS_COUNT - 1) {
					strcat(select_array, ",");
				}
			}
			
			M_BOX(GP_LEFT,
				GP.LABEL("Status:");
				GP.SWITCH("SSSs", solar.getWorkStatus());
			);
			M_BOX(GP_LEFT,
				GP.LABEL("OnIfProblem:");
				GP.SWITCH("SSSo", solar.getOnIfProblem());
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Delta:");
				GP.NUMBER("SSSd", "delta", solar.getDelta(), "25%");
				GP.PLAIN("°");
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Battery:");
				GP.SELECT("SSSba", select_array, solar.getBatterySensor() + 1);
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Boiler:");
				GP.SELECT("SSSbo", select_array, solar.getBoilerSensor() + 1);
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Exit:");
				GP.SELECT("SSSex", select_array, solar.getExitSensor() + 1);
			);
		);
		GP.BREAK();
		
		M_SPOILER("System", GP_ORANGE,
			M_BOX(GP_LEFT,
				GP.LABEL("Time data read:");
				GP.NUMBER("SStd", "time", read_data_time, "25%");
				GP.PLAIN("sec");
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Time display off:");
				GP.NUMBER("SStds", "time", display.getDisplayOffTime(), "25%");
				GP.PLAIN("sec");
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Display fps:");
				GP.NUMBER("SSdf", "fps", display.getDisplayFps(), "25%");
				GP.PLAIN("fps");
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Buzzer:");
				GP.SWITCH("SSb", buzzer_status);
			);
			M_BOX(GP_LEFT,
				GP.LABEL("Rele invert:");
				GP.SWITCH("SSri", rele_invert);
			);

			M_BLOCK(GP_THIN, 
				GP.TITLE("Time");

				M_BOX(GP_LEFT,
					GP.LABEL("Gmt:");
					GP.NUMBER("SSTg", "gmt", gmt, "25%");
				);
				M_BOX(GP_LEFT,
					GP.LABEL("Ntp sync:");
					GP.SWITCH("SSTns", ntp_sync);
				);
				
				GP.TIME("SSTt", GPtime((uint32_t)clk.getUnix(), gmt));
				GP.DATE("SSTd", GPdate((uint32_t)clk.getUnix(), gmt));
			);
			M_BLOCK(GP_THIN, 
				GP.TITLE("DS18B20");

				for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
					M_BOX(GP_LEFT,
						GP.TEXT(String("SSDtn") + i, "", ds_sensors[i].name, "17%", 2);
						GP.LABEL(":");
						GP.NUMBER_F(String("SSDtc") + i, "", ds_sensors[i].correction, 2, "25%");
						GP.PLAIN("°");
					);
				}
			);
			
			
			M_BLOCK(GP_THIN, //buttons block
				GP.BUTTON("SSBr", "RESET", "", GP_ORANGE, "45%");
				GP.BUTTON("SSBa", "ALL", "", GP_ORANGE, "45%");
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

void uiAction() {
	if (ui.update("HSt")) {
		ui.answer((!problem[1]) ? String(t_home, 1) + "°" : String("err"));
	}
	if (ui.update("HSh")) {
		ui.answer((!problem[1]) ? String(h_home, 1) + "%" : String("err"));
	}
	
	for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
		if (ui.update(String("HStn") + i)) {
		ui.answer(ds_sensors[i].name);
		}
		if (ui.update(String("HSt") + i)) {
		ui.answer((!ds_sensors[i].problem) ? String(ds_sensors[i].t, 1) + "°" : String("err"));
		}
	}

	if (ui.update("HSSba")) {
		ui.answer((!solar.batterySensor()) ? String(solar.getBatteryT(), 1) + "°" : String("err"));
	}
	if (ui.update("HSSbo")) {
		ui.answer((!solar.boilerSensor()) ? String(solar.getBoilerT(), 1) + "°" : String("err"));
	}
	if (ui.update("HSSex")) {
		ui.answer((!solar.exitSensor()) ? String(solar.getExitT(), 1) + "°" : String("err"));
	}
	if (ui.update("HSSpu")) {
		ui.answer(getRele());
	}
	//parse
	if (ui.click("HSSpu")) {
		bool state = ui.getBool();
		setRele(state);
	}


	const char* solar_sensors_names[] = {"SSSba", "SSSbo", "SSSex"};

	if (ui.update("SNm")) {
		ui.answer(network.getMode());
	}
	if (ui.update("SNWs")) {
		ui.answer(network.getWifiSsid());
	}
	if (ui.update("SNAs")) {
		ui.answer(network.getApSsid());
	}
	if (ui.update("SNAp")) {
		ui.answer(network.getApPass());
	}
	if (ui.update("SNBs")) {
		ui.answer(blynk.getWorkStatus());
	}
	if (ui.update("SNBa")) {
		ui.answer(blynk.getAuth());
	}

	for (uint8_t i = 0;i < blynk.getLinksCount();i++) {
		if (ui.update(String("SNBLp") + i)) {
			ui.answer(blynk.getLinkPort(i));
		}
		if (ui.update(String("SNBLe") + i)) {
			ui.answer(blynk.getLinkElement(i));
		}
	}

	if (ui.update("SSSs")) {
		ui.answer(solar.getWorkStatus());
	}
	if (ui.update("SSSo")) {
		ui.answer(solar.getOnIfProblem());
	}
	if (ui.update("SSSd")) {
		ui.answer(solar.getDelta());
	}

	for (uint8_t i = 0;i < 3;i++) {
		if (ui.update(solar_sensors_names[i])) {
			ui.answer(solar.getSensor(i) + 1);
		}
	}

	if (ui.update("SStd")) {
		ui.answer(read_data_time);
	}
	if (ui.update("SStds")) {
		ui.answer(display.getDisplayOffTime());
	}
	if (ui.update("SSdf")) {
		ui.answer(display.getDisplayFps());
	}
	if (ui.update("SSb")) {
		ui.answer(buzzer_status);
	}
	if (ui.update("SSri")) {
		ui.answer(rele_invert);
	}

	if (ui.update("SSTg")) {
		ui.answer(gmt);
	}
	if (ui.update("SSTns")) {
		ui.answer(ntp_sync);
	}
	
	for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
		if (ui.update(String("SSDtn") + i)) {
			ui.answer(ds_sensors[i].name);
		}
		if (ui.update(String("SSDtc") + i)) {
			ui.answer(ds_sensors[i].correction, 1);
		}
	}
	
	//parse
	if (ui.clickSub("S") || ui.formSub("/S")) {
		save_settings_flag = true;
	}
	if (ui.click("SNm")) {
		network.setMode(ui.getInt());
	}

	if (ui.form("/SNW")) {
		char read_ssid[NETWORK_SSID_PASS_SIZE];
		char read_pass[NETWORK_SSID_PASS_SIZE];

		ui.copyStr("SNWs", read_ssid, NETWORK_SSID_PASS_SIZE);
		ui.copyStr("SNWp", read_pass, NETWORK_SSID_PASS_SIZE);

		network.setWifi(read_ssid, read_pass);
	}

	if (ui.click("SNAs")) {
		String read_string(ui.getString());
		network.setAp(&read_string, NULL);
	}
	if (ui.click("SNAp")) {
		String read_string(ui.getString());
		network.setAp(NULL, &read_string);
	}
	if (ui.click("SNBs")) {
		blynk.setWorkStatus(ui.getBool());
	}
	if (ui.click("SNBa")) {
		blynk.setAuth(ui.getString());
	}
	if (ui.click("SNBLnl")) {
		blynk.addLink();
	}

	for (uint8_t i = 0;i < blynk.getLinksCount();i++) {
		if (ui.click(String("SNBLp") + i)) {
			blynk.setLinkPort(i, ui.getInt());
		}
		if (ui.click(String("SNBLe") + i)) {
			blynk.setLinkElement(i, ui.getInt());
		}
		if (ui.click(String("SNBLd") + i)) {
			blynk.delLink(i);
		}
	}
	
	if (ui.click("SSSs")) {
		solar.setWorkStatus(ui.getBool());
	}
	if (ui.click("SSSo")) {
		solar.setOnIfProblem(ui.getBool());
	}
	if (ui.click("SSSd")) {
		solar.setDelta(ui.getInt());
	}
	
	for (uint8_t i = 0;i < 3;i++) {
		if (ui.click(solar_sensors_names[i])) {
			solar.setSensor(i, ui.getInt() - 1);
		}
	}
	
	if (ui.click("SStd")) {
		read_data_time = constrain(ui.getInt(), 0, 255);
	}
	if (ui.click("SStds")) {
		display.setDisplayOffTime(ui.getInt());
	}
	if (ui.click("SSdf")) {
		display.setDisplayFps(ui.getInt());
	}
	if (ui.click("SSb")) {
		buzzer_status = ui.getBool();
	}
	if (ui.click("SSri")) {
		rele_invert = ui.getBool();
	}

	if (ui.click("SSTg")) {
		gmt = constrain(ui.getInt(), -12, 12);
	}
	if (ui.click("SSTns")) {
		ntp_sync = ui.getBool();
	}
	if (ui.click("SSTt")) {
		GPtime get_time = ui.getTime();
		clk.setTime(gmt, get_time.hour, get_time.minute, get_time.second, clk.day(gmt), clk.month(gmt), clk.year(gmt));
	}
	if (ui.click("SSTd")) {
		GPdate get_date = ui.getDate();
		clk.setTime(gmt, clk.hour(gmt), clk.minute(gmt), clk.second(gmt), get_date.day, get_date.month, get_date.year);
	}

	for (byte i = 0;i < DS_SENSORS_COUNT;i++) {
		if (ui.click(String("SSDtn") + i)) {
			ui.getString().toCharArray(ds_sensors[i].name, 3);
		}

		if (ui.click(String("SSDtc") + i)) {
			ds_sensors[i].correction = ui.getFloat();
		}
	}

	if (ui.click("SSBr")) {
		ESP.reset();
	}
	if (ui.click("SSBa")) {
		resetAll();
	}
}