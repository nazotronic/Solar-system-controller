/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.1
 * Date: 04.02.2025
 */

#include "data.h"

BlynkManager::BlynkManager() {
	makeDefault();
}


void BlynkManager::tick() {
	NetworkManager* network = system->getNetworkManager();

	if (!getWorkFlag() || network->getStatus() != WL_CONNECTED) {
		return;
	}

	if (network->getStatus() == WL_CONNECTED && !getStatus()) {
		connectBlynk();
	}

	if (millis() - send_data_timer > SEC_TO_MLS(getSendDataTime()) ) {
		send_data_timer = millis();

		if (getStatus()) {
			sendData();
		}
	}
	
	Blynk.run();
}

void BlynkManager::makeDefault() {
	links.~DynamicArray();

	work_flag = DEFAULT_BLYNK_WORK_STATUS;
	send_data_time = DEFAULT_BLYNK_SEND_DATA_TIME;
	memset(auth, 0, BLYNK_AUTH_SIZE);

	send_data_timer = 0;
	blynk_reconnect_timer = 0;
}

void BlynkManager::writeSettings(char* buffer) {
	setParameter(buffer, "SBs", getWorkFlag());
	setParameter(buffer, "SBsdt", getSendDataTime());
	setParameter(buffer, "SBa", (const char*) getAuth());

	for (uint8_t i = 0;i < links.size();i++) {
		setParameter(buffer, String("SBLp") + i, getLinkPort(i));
		setParameter(buffer, String("SBLe") + i, (const char*) getLinkElementCode(i));
	}
}

void BlynkManager::readSettings(char* buffer) {
	uint8_t link_index = 0;
	char element_code[BLYNK_ELEMENT_CODE_SIZE];

	getParameter(buffer, "SBs", &work_flag);
	getParameter(buffer, "SBsdt", &send_data_time);
	getParameter(buffer, "SBa", auth, BLYNK_AUTH_SIZE);

	while (getParameter(buffer, String("SBLe") + link_index, element_code, BLYNK_ELEMENT_CODE_SIZE)) {
		if (addLink()) {
			uint8_t link_port;
			setLinkElementCode(link_index, element_code);

			if (getParameter(buffer, String("SBLp") + link_index, &link_port)) {
				setLinkPort(link_index, link_port);
			}
		}

		link_index++;
	}
	
	setWorkFlag(work_flag);
	setSendDataTime(send_data_time);
	setAuth(auth);
}


bool BlynkManager::addLink() {
	if (links.add()) {
		setLinkPort(links.size() - 1, links.size() - 1);

		return true;
	}

	return false;
}

bool BlynkManager::deleteLink(uint8_t index) {
	if (links.del(index)) {
		return true;
	}

	return false;
}

bool BlynkManager::deleteLink(String element_code) {
	return deleteLink(scanLinkIndex(element_code));
}

bool BlynkManager::modifyLinkElementCode(String previous_code, String new_code) {
	int8_t link_index = scanLinkIndex(previous_code);

	if (link_index < 0) {
		return false;
	}


	setLinkElementCode(link_index, new_code);
	return true;
}


void BlynkManager::setSystemManager(SystemManager* system) {
	this->system = system;
}


void BlynkManager::setWorkFlag(bool work_flag) {
	this->work_flag = work_flag;

	if (!work_flag) {
		disconnectBlynk();
	}
}

void BlynkManager::setSendDataTime(uint8_t time) {
	send_data_time = constrain(time, 0, 100);
}

void BlynkManager::setAuth(String auth) {
	if (!*auth.c_str()) {
		strcpy(this->auth, "");
		disconnectBlynk();

		return;
	}
	
	strcpy(this->auth, auth.c_str());
}


void BlynkManager::setLinkPort(uint8_t index, uint8_t port) {
	if (!isCorrectLinkIndex(index)) {
		return;
	}

	links[index].port = port;
}

void BlynkManager::setLinkElementCode(uint8_t index, String code) {
	if (!isCorrectLinkIndex(index)) {
		return;
	}

	strcpy(links[index].element_code, code.c_str());
}


SystemManager* BlynkManager::getSystemManager() {
	return system;
}

bool BlynkManager::getStatus() {
	return Blynk.connected();
}


bool BlynkManager::getWorkFlag() {
	return work_flag;
}

uint8_t BlynkManager::getSendDataTime() {
	return send_data_time;
}

char* BlynkManager::getAuth() {
	return auth;
}


uint8_t BlynkManager::getLinksCount() {
	return links.size();
}

uint8_t BlynkManager::getLinkPort(uint8_t index) {
	if (!isCorrectLinkIndex(index)) {
		return 0;
	}

	return links[index].port;
}

char* BlynkManager::getLinkElementCode(uint8_t index) {
	if (!isCorrectLinkIndex(index)) {
		return NULL;
	}

	return links[index].element_code;
}


bool BlynkManager::isCorrectLinkIndex(uint8_t index) {
	if (index >= getLinksCount()) {
		return false;
	}

	return true;
}

int8_t BlynkManager::scanLinkIndex(String element_code) {
	if (!links.size()) {
		return -1;
	}

	for (uint8_t i = 0; i < links.size();i++) {
		if (!strcmp(links[i].element_code, element_code.c_str()) ) {
			return i;
		}
	}

	return -1;
}


void BlynkManager::connectBlynk() {
	if (!*getAuth()) {
		return;
	}

	if (!blynk_reconnect_timer || millis() - blynk_reconnect_timer >= SEC_TO_MLS(BLYNK_RECONNECT_TIME)) {
		blynk_reconnect_timer = millis();

		Blynk.config(this->auth);
		Blynk.connect(10);
	}
}

void BlynkManager::disconnectBlynk() {
	Blynk.disconnect();
}


void BlynkManager::sendData() {
	for (uint8_t i = 0;i < links.size();i++) {
		system->makeBlynkElementSend(&Blynk, &links[i]);
	}
}

extern SystemManager systemManager;
BLYNK_WRITE_DEFAULT() {
	BlynkManager* blynk = systemManager.getBlynkManager();

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		if (blynk->getLinkPort(i) == request.pin) {
			systemManager.makeBlynkElementParse(blynk->getLinkElementCode(i), param);
		}
	}
}

WiFiClient BlynkManager::_blynkWifiClient = WiFiClient();
BlynkArduinoClient BlynkManager::_blynkTransport = BlynkArduinoClient(_blynkWifiClient);
BlynkWifi BlynkManager::Blynk = BlynkWifi(_blynkTransport); 