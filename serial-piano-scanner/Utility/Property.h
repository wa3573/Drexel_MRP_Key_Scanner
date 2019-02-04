/*
 * Property.h
 *
 *  Created on: Feb 4, 2019
 *      Author: juniper
 */

#ifndef UTILITY_PROPERTY_H_
#define UTILITY_PROPERTY_H_

#include "Xml.h"
#include <map>
#include <string>

class PropertyNode {
public:
	PropertyNode()
	{

	}

	PropertyNode(std::string value) :
		value_(value)
	{

	}

	int toInt()
	{
		return std::stoi(value_);
	}

	double toDouble()
	{
		return std::stod(value_);
	}

	bool toBool()
	{
		if (value_.compare("1") == 0) {
			return true;
		} else {
			return false;
		}
	}

	std::string getValue()
	{
		return value_;
	}

private:
	std::string value_;
};

class PropertySet {
public:
	PropertySet()
{

}

	~PropertySet()
	{

	}

	void restoreFromXml(const XmlElement& xml)
	{

	}

	bool containsKey(std::string keyName)
	{
		map<std::string, PropertyNode>::iterator it = propertyMap_.find(keyName);

		if (it != propertyMap_.end())
		{
			return true;
		}

		return false;
	}

	bool containsKey(std::string keyName) const
	{
		map<std::string, PropertyNode>::const_iterator it = propertyMap_.find(keyName);

		if (it != propertyMap_.end())
		{
			return true;
		}

		return false;
	}

	std::string getValue(std::string keyName)
	{
		PropertyNode node = propertyMap_[keyName];
		return node.getValue();
	}

	std::string getValue(std::string keyName) const
	{
		PropertyNode node = (*const_cast<map<std::string, PropertyNode>*>(&propertyMap_))[keyName];
		return node.getValue();
	}

	int getIntValue(std::string keyName)
	{
		PropertyNode node = propertyMap_[keyName];
		return node.toInt();
	}

	int getIntValue(std::string keyName) const
	{
		PropertyNode node = (*const_cast<map<std::string, PropertyNode>*>(&propertyMap_))[keyName];
		return node.toInt();
	}

	bool getBoolValue(std::string keyName)
	{
		PropertyNode node = propertyMap_[keyName];
		return node.toBool();
	}

	bool getBoolValue(std::string keyName) const
	{
		PropertyNode node = (*const_cast<map<std::string, PropertyNode>*>(&propertyMap_))[keyName];
		return node.toBool();
	}

	double getDoubleValue(std::string keyName)
	{
		PropertyNode node = propertyMap_[keyName];
		return node.toDouble();
	}

	double getDoubleValue(std::string keyName) const
	{
		PropertyNode node = (*const_cast<map<std::string, PropertyNode>*>(&propertyMap_))[keyName];
		return node.toDouble();
	}

	void setValue(std::string keyName, int value)
	{
		propertyMap_[keyName] = PropertyNode(std::to_string(value));
	}

	void setValue(std::string keyName, double value)
	{
		propertyMap_[keyName] = PropertyNode(std::to_string(value));
	}

	void setValue(std::string keyName, bool value)
	{
		int intValue = (int) value;
		propertyMap_[keyName] = PropertyNode(std::to_string(intValue));
	}

	void setValue(std::string keyName, std::string value)
	{
		propertyMap_[keyName] = PropertyNode(value);
	}

	XmlElement* createXml(std::string keyName)
	{
		return NULL;
	}

	std::map<std::string, PropertyNode> propertyMap_;
};


#endif /* UTILITY_PROPERTY_H_ */
