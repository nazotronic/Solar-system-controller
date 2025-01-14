/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.3.0 beta
 * Date: 14.01.2025
 */

#include "data.h"

BlynkManager::BlynkManager() {
	makeDefault();
}


void BlynkManager::tick() {
	NetworkManager* network = system->getNetworkManager();

	if (!getWorkFlag() || !*getAuth() || network->getStatus() != WL_CONNECTED) {
		return;
	}

	if (millis() - send_data_timer > SEC_TO_MLS(getSendDataTime()) ) {
		send_data_timer = millis();

		if (getStatus()) {
			for (uint8_t i = 0;i < getLinksCount();i++) {
				sendData(i);
			}
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
}

void BlynkManager::writeSettings(char* buffer) {
	setParameter(buffer, "SBs", getWorkFlag());
	setParameter(buffer, "SBsdt", getSendDataTime());
	setParameter(buffer, "SBa", (const char*) getAuth());

	for (uint8_t i = 0;i < links.getSize();i++) {
		blynk_element_t* element = getLinkElement(i);

		setParameter(buffer, String("SBLp") + i, getLinkPort(i));
		setParameter(buffer, String("SBLe") + i, (const char*) element->code);
	}
}

void BlynkManager::readSettings(char* buffer) {
	DynamicArray<blynk_element_t> elements;
	uint8_t link_index = 0;
	
	char element_code[BLYNK_ELEMENT_CODE_SIZE] = "";
	uint8_t link_port = 0;

	system->makeBlynkElementsList(&elements);

	getParameter(buffer, "SBs", &work_flag);
	getParameter(buffer, "SBsdt", &send_data_time);
	getParameter(buffer, "SBa", auth, BLYNK_AUTH_SIZE);

	while (getParameter(buffer, String("SBLe") + link_index, element_code, BLYNK_ELEMENT_CODE_SIZE)) {
		int8_t el_index = system->scanBlynkElemetIndex(&elements, element_code);

		if (el_index >= 0) {
			if (addLink()) {
				setLinkElement(link_index, &elements[el_index]);

				if (getParameter(buffer, String("SBLp") + link_index, &link_port)) {
					setLinkPort(link_index, link_port);
				}
			}
		}

		link_index++;
	}

	setAuth(auth);
}


bool BlynkManager::addLink() {
	if (links.add()) {
		links[links.getSize() - 1].port = links.getSize() - 1;

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
	uint8_t link_index = scanLinkIndex(previous_code);

	if (link_index < 0) {
		return false;
	}

	strcpy(links[link_index].element.code, new_code.c_str());

	return true;
}


void BlynkManager::setSystemManager(SystemManager* system) {
	this->system = system;
}


void BlynkManager::setWorkFlag(bool work_flag) {
	this->work_flag = work_flag;
}

void BlynkManager::setSendDataTime(uint8_t time) {
	send_data_time = constrain(time, 0, 100);
}

void BlynkManager::setAuth(String auth) {
	if (!*auth.c_str()) {
		strcpy(this->auth, "");
		Blynk.disconnect();

		return;
	}
	
	strcpy(this->auth, auth.c_str());
	Blynk.config(this->auth);
	Blynk.connect(10);
}


void BlynkManager::setLinkPort(uint8_t index, uint8_t port) {
	if (!isCorrectIndex(index)) {
		return;
	}

	links[index].port = port;
}

void BlynkManager::setLinkElement(uint8_t index, blynk_element_t* element) {
	if (!isCorrectIndex(index)) {
		return;
	}

	memcpy(&links[index].element, element, sizeof(blynk_element_t));
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
	return links.getSize();
}

uint8_t BlynkManager::getLinkPort(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return 0;
	}

	return links[index].port;
}


blynk_element_t* BlynkManager::getLinkElement(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return NULL;
	}

	return &links[index].element;
}

const char* BlynkManager::getLinkElementName(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return NULL;
	}

	return links[index].element.name;
}

char* BlynkManager::getLinkElementCode(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return NULL;
	}

	return links[index].element.code;
}

void* BlynkManager::getLinkElementPointer(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return NULL;
	}

	return links[index].element.pointer;
}

uint8_t BlynkManager::getLinkElementType(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return BLYNK_TYPE_UINT8_T;
	}

	return links[index].element.type;
}


bool BlynkManager::isCorrectIndex(uint8_t index) {
	if (index >= getLinksCount()) {
		return false;
	}

	return true;
}

int8_t BlynkManager::scanLinkIndex(String element_code) {
	if (!links.getSize()) {
		return -1;
	}

	for (uint8_t i = 0; i < links.getSize();i++) {
		if (!strcmp(links[i].element.code, element_code.c_str()) ) {
			return i;
		}
	}

	return -1;
}


void BlynkManager::sendData(uint8_t index) {
	if (!isCorrectIndex(index)) {
		return;
	}

	switch (getLinkElementType(index)) {
	case BLYNK_TYPE_UINT8_T:
		Blynk.virtualWrite(getLinkPort(index), *(uint8_t *)getLinkElementPointer(index));
		break;
	case BLYNK_TYPE_INT8_T:
		Blynk.virtualWrite(getLinkPort(index), *(int8_t *)getLinkElementPointer(index));
		break;
	case BLYNK_TYPE_UINT32_T:
		Blynk.virtualWrite(getLinkPort(index), *(uint32_t *)getLinkElementPointer(index));
		break;
	case BLYNK_TYPE_INT32_T:
		Blynk.virtualWrite(getLinkPort(index), *(int32_t *)getLinkElementPointer(index));
		break;
	case BLYNK_TYPE_BOOL:
		Blynk.virtualWrite(getLinkPort(index), *(bool *)getLinkElementPointer(index));
		break;
	case BLYNK_TYPE_FLOAT:
		Blynk.virtualWrite(getLinkPort(index), *(float *)getLinkElementPointer(index));
		break;
	}
}

extern SystemManager systemManager;
BLYNK_WRITE_DEFAULT() {
	BlynkManager* blynk = systemManager.getBlynkManager();

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		if (blynk->getLinkPort(i) == request.pin) {
			switch (blynk->getLinkElementType(i)) {
				case BLYNK_TYPE_UINT8_T:
				case BLYNK_TYPE_INT8_T:
				case BLYNK_TYPE_UINT32_T:
				case BLYNK_TYPE_INT32_T:
				case BLYNK_TYPE_BOOL:  blynk->setLinkElementValue(i, param.asInt()); break;
				case BLYNK_TYPE_FLOAT: blynk->setLinkElementValue(i, param.asFloat()); break;
			}
		}
	}
}

template <class T>
void BlynkManager::setLinkElementValue(uint8_t index, T value) {
	if (!isCorrectIndex(index)) {
		return;
	}

	switch (getLinkElementType(index)) {
	case BLYNK_TYPE_UINT8_T:
		*(uint8_t *)getLinkElementPointer(index) = value;
		break;
	case BLYNK_TYPE_INT8_T:
		*(int8_t *)getLinkElementPointer(index) = value;
		break;
	case BLYNK_TYPE_UINT32_T:
		*(uint32_t *)getLinkElementPointer(index) = value;
		break;
	case BLYNK_TYPE_INT32_T:
		*(int32_t *)getLinkElementPointer(index) = value;
		break;
	case BLYNK_TYPE_BOOL:
		*(bool *)getLinkElementPointer(index) = value;
		break;
	case BLYNK_TYPE_FLOAT:
		*(float *)getLinkElementPointer(index) = value;
		break;
	}
}

WiFiClient BlynkManager::_blynkWifiClient = WiFiClient();
BlynkArduinoClient BlynkManager::_blynkTransport = BlynkArduinoClient(_blynkWifiClient);
BlynkWifi BlynkManager::Blynk = BlynkWifi(_blynkTransport); 