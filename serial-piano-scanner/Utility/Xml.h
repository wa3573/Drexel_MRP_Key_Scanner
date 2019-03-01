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
		tagName_ = tagName;
	}

	inline XmlElement()
	{

	}

	inline XmlElement* getChildByName(std::string childName)
	{
		std::map<std::string, XmlElement*>::iterator it = childrenMap_.find(
				childName);

		if (it != childrenMap_.end()) {
			return (*it).second;
		}

		return NULL;
	}

	inline XmlElement* getChildByName(std::string childName) const
	{
		std::map<std::string, XmlElement*>::const_iterator it =
				childrenMap_.find(childName);

		if (it != childrenMap_.end()) {
			return (*it).second;
		}

		return NULL;
	}

	inline bool hasAttribute(std::string attributeName)
	{
		std::map<std::string, XmlAttribute*>::iterator it = attributeMap_.find(
				attributeName);

		if (it != attributeMap_.end()) {
			return true;
		}

		return false;
	}

	inline std::string getStringAttribute(std::string attribute)
	{
		std::map<std::string, XmlAttribute*>::iterator it = attributeMap_.find(
				attribute);

		if (it != attributeMap_.end()) {
			return (*it).second->getName();
		}

		return NULL;
	}

	inline int getIntAttribute(std::string attribute)
	{
		return 0;
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

		if (!mapHasKey(attributeMap_, name)) {
			XmlAttribute* newAttribute = new XmlAttribute(name, value);
			attributeMap_[name] = newAttribute;
			attributes_.push_back(newAttribute);
		}

		attributeMap_[name]->setValue(value);
	}

private:
	std::string tagName_;
	std::map<std::string, XmlElement*> childrenMap_;
	std::vector<XmlElement*> children_;
	std::map<std::string, XmlAttribute*> attributeMap_;
	std::vector<XmlAttribute*> attributes_;
};

#endif /* UTILITY_XML_H_ */
