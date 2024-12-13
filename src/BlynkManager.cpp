/*
 * Project: Solar Battery Control System
 *
 * Author: Vereshchynskyi Nazar
 * Email: verechnazar12@gmail.com
 * Version: 1.1.0
 * Date: 12.12.2024
 */

#include "data.h"

BlynkManager::BlynkManager() {
	makeDefault();
}

BlynkManager::~BlynkManager() {
	free(elements);
	free(links);
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
	free(elements);
	free(links);

	elements = NULL;
	links = NULL;

	work_flag = DEFAULT_BLYNK_WORK_STATUS;
	send_data_time = DEFAULT_BLYNK_SEND_DATA_TIME;
	memset(auth, 0, BLYNK_AUTH_SIZE);

	send_data_timer = 0;
	elements_count = 0;
	links_count = 0;

}

void BlynkManager::writeSettings(char* buffer) {
	setParameter(buffer, "SBs", getWorkFlag());
	setParameter(buffer, "SBsdt", getSendDataTime());
	setParameter(buffer, "SBa", (const char*) getAuth());

	for (uint8_t i = 0;i < getLinksCount();i++) {
		uint8_t element_index = getLinkElement(i);

		setParameter(buffer, String("SBLp") + i, getLinkPort(i));
		setParameter(buffer, String("SBLe") + i, (const char*) getElementCode(element_index));
	}
}

void BlynkManager::readSettings(char* buffer) {
	uint8_t link_index = 0;
	uint8_t link_port = 0;
	char element_code[BLYNK_ELEMENT_CODE_SIZE] = "";

	getParameter(buffer, "SBs", &work_flag);
	getParameter(buffer, "SBsdt", &send_data_time);
	getParameter(buffer, "SBa", auth, BLYNK_AUTH_SIZE);

	while (getParameter(buffer, String("SBLp") + link_index, &link_port)) {
		if (getParameter(buffer, String("SBLe") + link_index, element_code, 10)) {
			if (addLink()) {
				setLinkPort(link_index, link_port);
				setLinkElement(link_index, element_code);
			}
		}

		link_index++;
	}

	setAuth(auth);
}


bool BlynkManager::addElement(const char* name, String code, void* pointer, uint8_t type) {
	if (name == NULL || pointer == NULL) {
		return false;
	}

	for (uint8_t i = 0;i < getElementsCount();i++) {
		if (!strcmp(getElementName(i), name) || !strcmp(getElementCode(i), code.c_str()) || getElementPointer(i) == pointer) {
			return false;
		}
	}

	if (!(getElementsCount() % 10)) {
		blynk_element_t* new_pointer = (blynk_element_t*) realloc(elements, (getElementsCount() + 10) * sizeof(blynk_element_t));
		
		if (new_pointer == NULL) {
			return false;
		}

		elements = new_pointer;
	}

	elements[elements_count].name = name;
	strcpy(elements[elements_count].code, code.c_str());
	elements[elements_count].pointer = pointer;
	elements[elements_count].type = type;

	elements_count++;
	return true;
}

bool BlynkManager::addLink() {
	if (elements == NULL) {
		return false;
	}

	if (!(getLinksCount() % 10)) {
		blynk_link_t* new_pointer = (blynk_link_t*) realloc(links, (getLinksCount() + 10) * sizeof(blynk_link_t));
		
		if (new_pointer == NULL) {
			return false;
		}

		links = new_pointer;
	}

	links[links_count].port = links_count;
	links[links_count].element_index = 0;
	
	links_count++;
	return true;
}

bool BlynkManager::delLink(uint8_t index) {
	if (index >= getLinksCount() || links == NULL) {
		return false;
	}

	while (index < getLinksCount() - 1) {
		links[index] = links[index + 1];

		index++;
	}
	
	links_count--;
	if (!(getLinksCount() % 10)) {
		links = (blynk_link_t*) realloc(links, getLinksCount() * sizeof(blynk_link_t));
		
		if (!links_count) {
			free(links);
			links = NULL;
		}
	}

	return true;
}


void BlynkManager::setSystemManager(SystemManager* system) {
	this->system = system;
}


void BlynkManager::setWorkFlag(bool work_flag) {
	this->work_flag = work_flag;
}

void BlynkManager::setSendDataTime(uint8_t time) {
	send_data_time = time;
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


void BlynkManager::setLinkPort(uint8_t link_index, uint8_t port) {
	if (link_index >= getLinksCount() || links == NULL) {
		return;
	}

	links[link_index].port = port;
}

void BlynkManager::setLinkElement(uint8_t link_index, uint8_t element_index) {
	if (link_index >= getLinksCount() || links == NULL || element_index >= getElementsCount()) {
		return;
	}

	links[link_index].element_index = element_index;
}
void BlynkManager::setLinkElement(uint8_t index, const char* element_code) {
	if (index >= getLinksCount() || links == NULL || element_code == NULL) {
		return;
	}

	for (uint8_t i = 0;i < getElementsCount();i++) {
		if (!strcmp(getElementCode(i), element_code)) {
			links[index].element_index = i;

			return;
		}
	}
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
	return links_count;
}

uint8_t BlynkManager::getLinkPort(uint8_t link_index) {
	if (link_index >= getLinksCount() || links == NULL) {
		return 0;
	}

	return links[link_index].port;
}

uint8_t BlynkManager::getLinkElement(uint8_t link_index) {
	if (link_index >= getLinksCount() || links == NULL) {
		return 0;
	}

	return links[link_index].element_index;
}


uint8_t BlynkManager::getElementsCount() {
	return elements_count;
}

const char* BlynkManager::getElementName(uint8_t el_index) {
	if (el_index >= getElementsCount() || elements == NULL) {
		return NULL;
	}

	return elements[el_index].name;
}

char* BlynkManager::getElementCode(uint8_t el_index) {
	if (el_index >= getElementsCount() || elements == NULL) {
		return NULL;
	}

	return elements[el_index].code;
}

void* BlynkManager::getElementPointer(uint8_t el_index) {
	if (el_index >= getElementsCount() || elements == NULL) {
		return NULL;
	}

	return elements[el_index].pointer;
}

uint8_t BlynkManager::getElementType(uint8_t el_index) {
	if (el_index >= getElementsCount() || elements == NULL) {
		return 0;
	}

	return elements[el_index].type;
}


void BlynkManager::sendData(uint8_t link_index) {
	uint8_t element_index = getLinkElement(link_index);

	if (link_index >= getLinksCount() || links == NULL) {
		return;
	}

	if (getElementPointer(element_index) == NULL) {
		return;
	}

	switch (getElementType(element_index)) {
	case BLYNK_TYPE_UINT8_T:
		Blynk.virtualWrite(getLinkPort(link_index), *(uint8_t *)getElementPointer(element_index));
		break;
	case BLYNK_TYPE_INT8_T:
		Blynk.virtualWrite(getLinkPort(link_index), *(int8_t *)getElementPointer(element_index));
		break;
	case BLYNK_TYPE_UINT32_T:
		Blynk.virtualWrite(getLinkPort(link_index), *(uint32_t *)getElementPointer(element_index));
		break;
	case BLYNK_TYPE_INT32_T:
		Blynk.virtualWrite(getLinkPort(link_index), *(int32_t *)getElementPointer(element_index));
		break;
	case BLYNK_TYPE_BOOL:
		Blynk.virtualWrite(getLinkPort(link_index), *(bool *)getElementPointer(element_index));
		break;
	case BLYNK_TYPE_FLOAT:
		Blynk.virtualWrite(getLinkPort(link_index), *(float *)getElementPointer(element_index));
		break;
	}
}

extern SystemManager systemManager;
BLYNK_WRITE_DEFAULT() {
	BlynkManager* blynk = systemManager.getBlynkManager();

	for (uint8_t i = 0;i < blynk->getLinksCount();i++) {
		uint8_t element_index = blynk->getLinkElement(i);

		if (blynk->getLinkPort(i) == request.pin) {
			switch (blynk->getElementType(element_index)) {
			case BLYNK_TYPE_UINT8_T:
			case BLYNK_TYPE_INT8_T:
			case BLYNK_TYPE_UINT32_T:
			case BLYNK_TYPE_INT32_T:
			case BLYNK_TYPE_BOOL:  blynk->setElementValue(element_index, param.asInt()); break;
			case BLYNK_TYPE_FLOAT: blynk->setElementValue(element_index, param.asFloat()); break;
			}
		}
	}
}

template <class T>
void BlynkManager::setElementValue(uint8_t index, T value) {
	if (index >= getElementsCount() || elements == NULL) {
		return;
	}

	switch (getElementType(index)) {
	case BLYNK_TYPE_UINT8_T:
		*(uint8_t *)getElementPointer(index) = value;
		break;
	case BLYNK_TYPE_INT8_T:
		*(int8_t *)getElementPointer(index) = value;
		break;
	case BLYNK_TYPE_UINT32_T:
		*(uint32_t *)getElementPointer(index) = value;
		break;
	case BLYNK_TYPE_INT32_T:
		*(int32_t *)getElementPointer(index) = value;
		break;
	case BLYNK_TYPE_BOOL:
		*(bool *)getElementPointer(index) = value;
		break;
	case BLYNK_TYPE_FLOAT:
		*(float *)getElementPointer(index) = value;
		break;
	}
}

WiFiClient BlynkManager::_blynkWifiClient = WiFiClient();
BlynkArduinoClient BlynkManager::_blynkTransport = BlynkArduinoClient(_blynkWifiClient);
BlynkWifi BlynkManager::Blynk = BlynkWifi(_blynkTransport); 