/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.1.0
 * Date: 12.12.2024
 */

#include "data.h"

NetworkManager::NetworkManager() {
	makeDefault();
}

void NetworkManager::begin() {
	udp.begin(2390);
	
	ui.attachBuild(this->uiBuild);
	ui.attach(this->uiAction);
	ui.enableOTA();
}


void NetworkManager::makeDefault() {
	system = NULL;

	mode = DEFAULT_NETWORK_MODE;
	reset_request = true;
	tick_allow = true;
	wifi_reconnect_timer = 0;

	setAp("", "");
  	setWifi("", "");
}

void NetworkManager::tick() {
	if (!tick_allow) {
		return;
	}
	
	if (reset_request) {
		Serial.println("reset");

		reset_request = false;
		off();
	}


	if (getMode() == NETWORK_OFF) {
		if (WiFi.getMode() != WIFI_OFF) {
			off();
		}

		return;
	}

	else if (getMode() == NETWORK_STA) {
		if (WiFi.getMode() != WIFI_STA) {
			Serial.println("sta");
			
			WiFi.mode(WIFI_STA);
			ui.start();
		}
	}

	else if (getMode() == NETWORK_AP_STA) {
		if (WiFi.getMode() != WIFI_AP_STA) {
			Serial.println("ap_sta");
			
			WiFi.mode(WIFI_AP_STA);
			ui.start();
		}
	}

	else if (getMode() == NETWORK_AUTO) {
		if (getStatus() == WL_CONNECTED && WiFi.getMode() != WIFI_STA) {
			Serial.println("auto sta");
			
			WiFi.mode(WIFI_STA);
			ui.start();
		}
		
		else if (getStatus() != WL_CONNECTED && WiFi.getMode() != WIFI_AP_STA) {
			Serial.println("auto ap sta");
			
			WiFi.mode(WIFI_AP_STA);
			ui.start();
		}
	}


	if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
		if (getStatus() != WL_CONNECTED) {
			if (millis() - wifi_reconnect_timer >= SEC_TO_MLS(NETWORK_RECONNECT_TIME) || !wifi_reconnect_timer) {
				wifi_reconnect_timer = millis();

				connect();
			}
		}
	}


	ui.tick();
}

void NetworkManager::writeSettings(char* buffer) {
	setParameter(buffer, "SNm", mode);
	setParameter(buffer, "SNWs", (const char*) ssid_sta);
	setParameter(buffer, "SNWp", (const char*) pass_sta);
	setParameter(buffer, "SNAs", (const char*) ssid_ap);
	setParameter(buffer, "SNAp", (const char*) pass_ap);
}

void NetworkManager::readSettings(char* buffer) {
	getParameter(buffer, "SNm", &mode);
	getParameter(buffer, "SNWs", ssid_sta, NETWORK_SSID_PASS_SIZE);
	getParameter(buffer, "SNWp", pass_sta, NETWORK_SSID_PASS_SIZE);
	getParameter(buffer, "SNAs", ssid_ap, NETWORK_SSID_PASS_SIZE);
	getParameter(buffer, "SNAp", pass_ap, NETWORK_SSID_PASS_SIZE);
	
	setAp(ssid_ap, pass_ap);
	reset_request = true;
}


bool NetworkManager::connect(String ssid, String pass, uint8_t connect_time, bool auto_save) {
	bool connect_status = false;
	
	if (!ssid[0]) {
		WiFi.begin(getWifiSsid(), getWifiPass());
		connect_status = getStatus();
	}
	else {
		uint32_t connect_timer = millis();
		tick_allow = false;
		
		off();
   		WiFi.mode(WIFI_STA);
		WiFi.begin(ssid, pass);

		while (millis() - connect_timer < SEC_TO_MLS(connect_time) && connect_time) {
			if (getStatus() == WL_CONNECTED) {
				connect_status = true;
				break;
			}
			
			system->tick();
		}
		
		if (auto_save && getStatus()) {
			setWifi(ssid.c_str(), pass.c_str());
		}

   		tick_allow = true;
		reset_request = true;
	}

	return connect_status;
}

void NetworkManager::off() {
	ui.stop();
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	
	wifi_reconnect_timer = 0;
}

bool NetworkManager::isWifiOn() {
  	return (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA);
}

bool NetworkManager::isApOn() {
  	return (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA);
}


bool NetworkManager::ntpSync(TimeManager* time) {
	uint8_t packet[48];
	static uint32_t last_send_timer;

	if (getStatus() == WL_CONNECTED) {
		if (millis() - last_send_timer >= SEC_TO_MLS(UDP_RESEND_TIME) || !last_send_timer) {
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
	}

	if (udp.parsePacket()) {
		udp.read(packet, 48);
		
		uint32_t ntp_time = (word(packet[40], packet[41]) << 16) | word(packet[42], packet[43]);
		int32_t unix_time = ntp_time - 2208988800UL;
		time->setUnix(unix_time);
		
		return true;
	}
	
	return false;
}


void NetworkManager::setSystemManager(SystemManager* system) {
	this->system = system;
}


void NetworkManager::setMode(uint8_t mode) {
  	this->mode = mode;
}

void NetworkManager::setWifi(String* ssid, String* pass) {
  	setWifi((ssid != NULL) ? ssid->c_str() : NULL, (pass != NULL) ? pass->c_str() : NULL);
}
void NetworkManager::setWifi(const char* ssid, const char* pass) {
	if (ssid != NULL) {
		strcpy(ssid_sta, ssid);
	}
	if (pass != NULL) {
		strcpy(pass_sta, pass);
	}

	reset_request = true;
}

void NetworkManager::setAp(String* ssid, String* pass) {
  	setAp((ssid != NULL) ? ssid->c_str() : NULL, (pass != NULL) ? pass->c_str() : NULL);
}
void NetworkManager::setAp(const char* ssid, const char* pass) {
	if (ssid != NULL) {
		strcpy(ssid_ap, (!*ssid) ? DEFAULT_NETWORK_SSID_AP : ssid);
	}
	if (pass != NULL) {
		strcpy(pass_ap, (!*pass) ? DEFAULT_NETWORK_PASS_AP : pass);
	}
	
	WiFiMode_t current_mode = WiFi.getMode();
	WiFi.softAP(ssid_ap, pass_ap);
	WiFi.mode(current_mode);
}


SystemManager* NetworkManager::getSystemManager() {
	return system;
}

wl_status_t NetworkManager::getStatus() {
  	return WiFi.status();
}


uint8_t NetworkManager::getMode() {
  	return mode;
}

char* NetworkManager::getWifiSsid() {
  	return ssid_sta;
}

char* NetworkManager::getWifiPass() {
  	return pass_sta;
}

char* NetworkManager::getApSsid() {
  	return ssid_ap;
}

char* NetworkManager::getApPass() {
  	return pass_ap;
}


SystemManager* NetworkManager::system = NULL;
GyverPortal NetworkManager::ui = GyverPortal(&LittleFS);