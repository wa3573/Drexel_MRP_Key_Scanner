/*
 * Xml.h
 *
 *  Created on: Feb 4, 2019
 *      Author: juniper
 */

#ifndef UTILITY_XML_H_
#define UTILITY_XML_H_

#include <map>
#include <vector>
#include "tinyxml2.h"

template<class T>
inline bool mapHasKey(std::map<std::string, T> map, std::string attributeName)
{
	typename std::map<std::string, T>::iterator it = map.find(attributeName);

	if (it != map.end()) {
		return true;
	}

	return false;
}

class XmlAttribute {
public:
	XmlAttribute(std::string name)
	{
		name_ = name;
	}

	XmlAttribute(std::string name, std::string value) :
			name_(name), value_(value)
	{

	}

	~XmlAttribute()
	{

	}

	const std::string& getName() const
	{
		return name_;
	}

	void setName(const std::string& name)
	{
		name_ = name;
	}

	const std::string& getType() const
	{
		return type_;
	}

	void setType(const std::string& type)
	{
		type_ = type;
	}

	const std::string& getValue() const
	{
		return value_;
	}

	void setValue(const std::string& value)
	{
		value_ = value;
	}

private:
	std::string name_;
	std::string type_;
	std::string value_;
};

class XmlElement {
public:
	inline XmlElement(std::string tagName)
	{
//		tinyElement_ = new tinyxml2::XMLElement();
//		tinyElement_->SetName(tagName.c_str());
	}

	inline XmlElement(tinyxml2::XMLElement* element)
	{
		tinyElement_ = element;
	}

	inline XmlElement()
	{

	}

	inline ~XmlElement()
	{
//		delete tinyElement_;
	}

	inline XmlElement* getChildByName(std::string childName)
	{
		return new XmlElement(
				tinyElement_->FirstChildElement(childName.c_str()));
	}

	inline XmlElement* getChildByName(std::string childName) const
	{

		return const_cast<XmlElement*>(new XmlElement(
				tinyElement_->FirstChildElement(childName.c_str())));
	}

	inline bool hasAttribute(std::string attributeName)
	{
		return tinyElement_->QueryAttribute(attributeName.c_str(), (int*) NULL)
				== tinyxml2::XML_SUCCESS;
	}

	inline std::string getStringAttribute(std::string attribute)
	{

		return NULL;
	}

	inline int getIntAttribute(std::string attribute)
	{
		int ret = 0;

		tinyElement_->QueryAttribute(attribute.c_str(), &ret);

		return ret;
	}

	inline void addChildElement(XmlElement* element)
	{

	}

	inline XmlElement* getNextElementWithTagName(std::string tagName)
	{
		return NULL;
	}

	inline void setAttribute(std::string name, std::string value)
	{
	}

	inline void setAttribute(std::string name, int value)
	{
		tinyElement_->SetAttribute(name.c_str(), value);
	}

	inline XmlElement* createNewChildElement(std::string tagName)
	{
//		tinyxml2::XMLElement* child = new tinyxml2::XMLElement(tagName);
//		return tinyElement_->InsertEndChild(child);
		return NULL;
	}

	inline bool writeToFile(const char* filename, std::string dtd)
	{
		tinyxml2::XMLDocument doc;

		doc.InsertEndChild(tinyElement_);
		return doc.SaveFile(filename);
	}

private:
	tinyxml2::XMLElement* tinyElement_;
	std::string tagName_;
};

class XmlDocument {
public:
	inline XmlDocument()
	{
		tinyDocument_ = new tinyxml2::XMLDocument();
	}

	inline XmlDocument(const char* filename)
	{
		tinyDocument_ = new tinyxml2::XMLDocument(filename);
	}

	inline ~XmlDocument()
	{
		delete tinyDocument_;
	}

	inline XmlElement* getDocumentElement()
	{
		return new XmlElement(tinyDocument_->RootElement());
	}

private:
	tinyxml2::XMLDocument* tinyDocument_;
};

#endif /* UTILITY_XML_H_ */
