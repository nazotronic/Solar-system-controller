/*
 * File: blynk.cpp
 * 
 * Description:
 * This file manages the interaction with the Blynk platform.
 * It handles sending data to and receiving data from ports,
 * enabling remote monitoring and control of the system via the Blynk application.
 * 
 * Author: Nazarii Vereshchynskyi
 * Email: verechnazar12@gmail.com
 * Date: 02.09.2024
 */

#include "data.h"

BlynkManage::BlynkManage() {
	makeDefault();
}

BlynkManage::~BlynkManage() {
	free(elements);
	free(links);
}

void BlynkManage::makeDefault() {
	free(elements);
	free(links);

	elements = NULL;
	links = NULL;

	work_status = DEFAULT_BLYNK_WORK_STATUS;
	elements_count = 0;
	links_count = 0;
}

void BlynkManage::saveSettings(char* buffer) {
	setParameter(buffer, "SNBs", work_status);
	setParameter(buffer, "SNBa", (const char*) auth);

	for (uint8_t i = 0;i < getLinksCount();i++) {
		uint8_t element_index = blynk.getLinkElement(i);

		setParameter(buffer, String("SNBLp") + i, getLinkPort(i));
		setParameter(buffer, String("SNBLe") + i, (const char*) getElementCode(element_index));
	}
}

void BlynkManage::readSettings(char* buffer) {
	uint8_t link_index = 0;
	uint8_t link_port = 0;
	char element_code[BLYNK_ELEMENT_CODE_SIZE] = "";

	while (getParameter(buffer, String("SNBLp") + link_index, &link_port)) {
		if (getParameter(buffer, String("SNBLe") + link_index, element_code, 10)) {
			if (addLink()) {
				setLinkPort(link_index, link_port);
				setLinkElement(link_index, element_code);
			}
		}

		link_index++;
	}

	getParameter(buffer, "SNBs", &work_status);
	getParameter(buffer, "SNBa", auth, BLYNK_AUTH_SIZE);

	setAuth(auth);
}

void BlynkManage::tick() {
	if (!work_status || !*auth || network.status() != WL_CONNECTED) {
		return;
	}

	if (status()) {
		for (uint8_t i = 0;i < links_count;i++) {
			send(i);
		}
	}
}

bool BlynkManage::status() {
	return Blynk.connected();
}

BLYNK_WRITE_DEFAULT() {
	for (uint8_t i = 0;i < blynk.getLinksCount();i++) {
		uint8_t element_index = blynk.getLinkElement(i);

		if (blynk.getLinkPort(i) == request.pin) {
			switch (blynk.getElementType(element_index)) {
			case BLYNK_TYPE_UINT8_T:
			case BLYNK_TYPE_INT8_T:
			case BLYNK_TYPE_UINT32_T:
			case BLYNK_TYPE_INT32_T:
			case BLYNK_TYPE_BOOL:  blynk.setElementValue(element_index, param.asInt()); break;
			case BLYNK_TYPE_FLOAT: blynk.setElementValue(element_index, param.asFloat()); break;
			}
		}
	}
}


bool BlynkManage::addElement(const char* name, String code, void* pointer, uint8_t type) {
	if (name == NULL || pointer == NULL) {
		return false;
	}

	for (uint8_t i = 0;i < elements_count;i++) {
		if (!strcmp(elements[i].name, name) || !strcmp(elements[i].code, code.c_str()) || elements[i].pointer == pointer) {
			return false;
		}
	}

	if (!(elements_count % 10)) {
		blynk_element_t* new_pointer = (blynk_element_t*) realloc(elements, (elements_count + 10) * sizeof(blynk_element_t));
		
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

bool BlynkManage::addLink() {
	if (elements == NULL) {
		return false;
	}

	if (!(links_count % 10)) {
		blynk_link_t* new_pointer = (blynk_link_t*) realloc(links, (links_count + 10) * sizeof(blynk_link_t));
		
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

bool BlynkManage::delLink(uint8_t index) {
	if (index >= links_count || links == NULL) {
		return false;
	}

	while (index < links_count - 1) {
		links[index] = links[index + 1];

		index++;
	}
	
	links_count--;
	if (!(links_count % 10)) {
		links = (blynk_link_t*) realloc(links, links_count * sizeof(blynk_link_t));
		
		if (!links_count) {
			free(links);
			links = NULL;
		}
	}

	return true;
}


void BlynkManage::setWorkStatus(bool status) {
	work_status = status;
}

void BlynkManage::setAuth(String auth) {
	if (!*auth.c_str()) {
		strcpy(this->auth, "");
		Blynk.disconnect();

		return;
	}
	
	strcpy(this->auth, auth.c_str());
	Blynk.config(this->auth);
	Blynk.connect(10);
}

void BlynkManage::setLinkPort(uint8_t index, uint8_t port) {
	if (index >= links_count || links == NULL) {
		return;
	}

	links[index].port = port;
}

void BlynkManage::setLinkElement(uint8_t index, uint8_t element_index) {
	if (index >= links_count || links == NULL || element_index >= elements_count) {
		return;
	}

	links[index].element_index = element_index;
}
void BlynkManage::setLinkElement(uint8_t index, const char* element_code) {
	if (index >= links_count || links == NULL || element_code == NULL) {
		return;
	}

	for (uint8_t i = 0;i < getElementsCount();i++) {
		if (!strcmp(getElementCode(i), element_code)) {
			links[index].element_index = i;

			return;
		}
	}
}


bool BlynkManage::getWorkStatus() {
	return work_status;
}

char* BlynkManage::getAuth() {
	return auth;
}

uint8_t BlynkManage::getLinksCount() {
	return links_count;
}

uint8_t BlynkManage::getElementsCount() {
	return elements_count;
}

uint8_t BlynkManage::getLinkPort(uint8_t index) {
	if (index >= links_count || links == NULL) {
		return 0;
	}

	return links[index].port;
}

uint8_t BlynkManage::getLinkElement(uint8_t index) {
	if (index >= links_count || links == NULL) {
		return 0;
	}

	return links[index].element_index;
}

const char* BlynkManage::getElementName(uint8_t index) {
	if (index >= elements_count || elements == NULL) {
		return NULL;
	}

	return elements[index].name;
}

char* BlynkManage::getElementCode(uint8_t index) {
	if (index >= elements_count || elements == NULL) {
		return NULL;
	}

	return elements[index].code;
}

void* BlynkManage::getElementPointer(uint8_t index) {
	if (index >= elements_count || elements == NULL) {
		return NULL;
	}

	return elements[index].pointer;
}

uint8_t BlynkManage::getElementType(uint8_t index) {
	if (index >= elements_count || elements == NULL) {
		return 0;
	}

	return elements[index].type;
}


void BlynkManage::send(uint8_t link_index) {
	uint8_t element_index = getLinkElement(link_index);

	if (link_index >= links_count || links == NULL) {
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

template <class T>
void BlynkManage::setElementValue(uint8_t index, T value) {
	if (index >= elements_count || elements == NULL) {
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