/*
 * File: network.cpp
 * 
 * Description:
 * This file manages the network operations, including the activation and deactivation of the access point (AP) 
 * and overall control of the WiFi library.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Date: 02.09.2024
 */

#include "data.h"

Network::Network() {
	makeDefault();
}

void Network::makeDefault() {
	mode = DEFAULT_NETWORK_MODE;
	reset_request = true;
	tick_allow = true;
	reconnect_timer = 0;

	setAp("", "");
  	setWifi("", "");
}

void Network::saveSettings(char* buffer) {
	setParameter(buffer, "SNm", mode);
	setParameter(buffer, "SNWs", (const char*) ssid_sta);
	setParameter(buffer, "SNWp", (const char*) pass_sta);
	setParameter(buffer, "SNAs", (const char*) ssid_ap);
	setParameter(buffer, "SNAp", (const char*) pass_ap);
}

void Network::readSettings(char* buffer) {
	getParameter(buffer, "SNm", &mode);
	getParameter(buffer, "SNWs", ssid_sta, NETWORK_SSID_PASS_SIZE);
	getParameter(buffer, "SNWp", pass_sta, NETWORK_SSID_PASS_SIZE);
	getParameter(buffer, "SNAs", ssid_ap, NETWORK_SSID_PASS_SIZE);
	getParameter(buffer, "SNAp", pass_ap, NETWORK_SSID_PASS_SIZE);
	
	setAp(ssid_ap, pass_ap);
	reset_request = true;
}

void Network::tick() {
	if (!tick_allow) {
		return;
	}
	
	if (reset_request) {
		reset_request = false;
		off();
	}

	if (mode == NETWORK_OFF) {
		if (WiFi.getMode() != WIFI_OFF) {
			off();
		}
	}

	else if (mode == NETWORK_STA) {
		if (WiFi.getMode() != WIFI_STA) {
			Serial.println("sta");
			
			WiFi.mode(WIFI_STA);
			ui.start();
		}
	}

	else if (mode == NETWORK_AP_STA) {
		if (WiFi.getMode() != WIFI_AP_STA) {
			Serial.println("ap_sta");
			
			WiFi.mode(WIFI_AP_STA);
			ui.start();
		}
	}

	else if (mode == NETWORK_AUTO) {
		if (status() == WL_CONNECTED && WiFi.getMode() != WIFI_STA) {
			Serial.println("auto sta");
			
			WiFi.mode(WIFI_STA);
			ui.start();
		}
		
		else if (status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP_STA) {
			Serial.println("auto ap sta");
			
			WiFi.mode(WIFI_AP_STA);
			ui.start();
		}
	}

	if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
		if (status() != WL_CONNECTED && (millis() - reconnect_timer >= NETWORK_RECONNECT_TIME || !reconnect_timer)) {
			reconnect_timer = millis();
			connect();
		}
	}
}

bool Network::connect(String ssid, String pass, uint8_t connect_time, bool auto_save) {
	bool connect_result = false;
	uint32_t connect_timer = 0;

	tick_allow = false;
	
	if (ssid[0]) {
		off();
   		WiFi.mode(WIFI_STA);
	}
	WiFi.begin((ssid[0]) ? ssid : String(ssid_sta), (ssid[0]) ? pass : String(pass_sta));
	
	connect_timer = millis();
	while (millis() - connect_timer < connect_time * 1000 && connect_time) {
		if (status() == WL_CONNECTED) {
			connect_result = true;
			break;
		}
		
		systemTick();
	}

	if (auto_save && ssid[0] && connect_result) {
		setWifi(ssid.c_str(), pass.c_str());
	}
  
   	if (ssid[0]) {
		reset_request = true;
   	}
	
   	tick_allow = true;
   	tick();

   	return connect_result;
}

void Network::off() {
	ui.stop();
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	
	reconnect_timer = 0;
}


wl_status_t Network::status() {
  	return WiFi.status();
}


void Network::setMode(uint8_t mode) {
  	this->mode = mode;
}

void Network::setWifi(String* ssid, String* pass) {
  	setWifi((ssid != NULL) ? ssid->c_str() : NULL, (pass != NULL) ? pass->c_str() : NULL);
}
void Network::setWifi(const char* ssid, const char* pass) {
	if (ssid != NULL) {
		strcpy(ssid_sta, ssid);
	}
	if (pass != NULL) {
		strcpy(pass_sta, pass);
	}

	reset_request = true;
}

void Network::setAp(String* ssid, String* pass) {
  	setAp((ssid != NULL) ? ssid->c_str() : NULL, (pass != NULL) ? pass->c_str() : NULL);
}
void Network::setAp(const char* ssid, const char* pass) {
	if (ssid != NULL) {
		strcpy(ssid_ap, (!*ssid) ? DEFAULT_NETWORK_SSID_AP : ssid);
	}
	if (pass != NULL) {
		strcpy(pass_ap, (!*pass) ? DEFAULT_NETWORK_PASS_AP : pass);
	}
	
	WiFiMode_t currentMode = WiFi.getMode();
	WiFi.softAP(ssid_ap, pass_ap);
	WiFi.mode(currentMode);
}


uint8_t Network::getMode() {
  	return mode;
}

char* Network::getWifiSsid() {
  	return ssid_sta;
}

char* Network::getWifiPass() {
  	return pass_sta;
}

char* Network::getApSsid() {
  	return ssid_ap;
}

char* Network::getApPass() {
  	return pass_ap;
}


bool Network::isWifi() {
  	return (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA);
}

bool Network::isAp() {
  	return (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA);
}


bool ntpSync() {
	static uint8_t packet[48];
	static uint32_t last_send_timer = 0;

	if (network.status() == WL_CONNECTED && (millis() - last_send_timer >= UDP_RESEND_TIME || !last_send_timer)) {
		last_send_timer = millis();

		packet[0] = 0b11100011;  // LI, Version, Mode
		packet[1] = 0;           // Stratum, or type of clock
		packet[2] = 6;           // Polling Interval
		packet[3] = 0xEC;        // Peer Clock Precision
		// 8 bytes of zero for Root Delay & Root Dispersion
		packet[12] = 49;
		packet[13] = 0x4E;
		packet[14] = 49;
		packet[15] = 52;
	
		udp.beginPacket(NTP_SERVER, NTP_PORT);
		udp.write(packet, 48);
		udp.endPacket();
	}

	if (udp.parsePacket()) {
		udp.read(packet, 48);
		
		uint32_t ntp_time = (word(packet[40], packet[41]) << 16) | word(packet[42], packet[43]);
		int32_t unix_time = ntp_time - 2208988800UL;
		clk.setUnix(unix_time);
		
		return true;
	}
	
	return false;
}