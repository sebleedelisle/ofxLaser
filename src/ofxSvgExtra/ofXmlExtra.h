#pragma once

#include "ofConstants.h"
#include "pugixml.hpp"
#include "ofParameter.h"

template<typename It>
class ofXmlExtraIterator;
class ofXmlExtraAttributeIterator;
class ofXmlExtraSearchIterator;

class ofXmlExtra{
public:
	class Search{
	public:
		Search(){}

		// Get collection type
		pugi::xpath_node_set::type_t type() const;

		// Get collection size
		size_t size() const;

		// Indexing operator
		ofXmlExtra operator[](size_t index) const;

		// Collection iterators
		ofXmlExtraSearchIterator begin() const;
		ofXmlExtraSearchIterator end() const;

		// Sort the collection in ascending/descending order by document order
		void sort(bool reverse = false);

		// Get first node in the collection by document order
		ofXmlExtra getFirst() const;

		// Check if collection is empty
		bool empty() const;
	private:
		Search(std::shared_ptr<pugi::xml_document> doc, pugi::xpath_node_set set);
		std::shared_ptr<pugi::xml_document> doc;
		pugi::xpath_node_set search;
		friend class ofXmlExtra;
	};

	class Attribute{
	public:
		Attribute(){}
		std::string getValue() const;

		void setName(const std::string & name);
		std::string getName() const;

		int getIntValue() const;
		unsigned int getUintValue() const;
		float getFloatValue() const;
		double getDoubleValue() const;
		bool getBoolValue() const;
		operator bool() const;

		Attribute getNextAttribute() const;
		Attribute getPreviousAttribute() const;

		template<typename T>
		ofXmlExtra::Attribute & operator=(const T & value){
			this->attr = ofToString(value);
			return *this;
		}

		template<typename T>
		Attribute & set(const T & value){
			this->attr.set_value(ofToString(value).c_str());
			return *this;
		}
	private:
		Attribute(const pugi::xml_attribute & attr);
		pugi::xml_attribute attr;
		friend class ofXmlExtra;
	};

	template<class It>
	class Range{
	public:
		It begin() const {
			if(range.begin() != range.end()){
				return It(doc, *range.begin());
			}else{
				return It(doc, typename It::Node());
			}
		}
		It end() const { return It(doc, typename It::Node()); }

	private:
		Range(std::shared_ptr<pugi::xml_document> doc, pugi::xml_object_range<typename It::Base> range)
			:doc(doc), range(range){}
		std::shared_ptr<pugi::xml_document> doc;
		pugi::xml_object_range<typename It::Base> range;
		friend class ofXmlExtra;
	};

	ofXmlExtra();

	bool load(const std::filesystem::path & file);
	bool load(const ofBuffer & buffer);
	bool parse(const std::string & xmlStr);
	bool save(const std::filesystem::path & file) const;
	void clear();
	std::string toString(const std::string & indent = "\t") const;

	ofXmlExtra getChild(const std::string & name) const;
	Range<ofXmlExtraIterator<pugi::xml_node_iterator>> getChildren() const;
	Range<ofXmlExtraIterator<pugi::xml_named_node_iterator>> getChildren(const std::string & name) const;

	ofXmlExtra appendChild(const ofXmlExtra & xml);
	ofXmlExtra prependChild(const ofXmlExtra & xml);
	bool removeChild(const ofXmlExtra & node);

#if PUGIXML_VERSION>=170
	ofXmlExtra appendChild(ofXmlExtra && xml);
	ofXmlExtra prependChild(ofXmlExtra && xml);
	bool removeChild(ofXmlExtra && node);
#endif

	ofXmlExtra appendChild(const std::string & name);
	ofXmlExtra prependChild(const std::string & name);
	bool removeChild(const std::string & name);

	ofXmlExtra insertChildAfter(const std::string & name, const ofXmlExtra & after);
	ofXmlExtra insertChildBefore(const std::string & name, const ofXmlExtra & after);

	ofXmlExtra getNextSibling() const;
	ofXmlExtra getPreviousSibling() const;
	ofXmlExtra getNextSibling(const std::string & name) const;
	ofXmlExtra getPreviousSibling(const std::string & name) const;

	ofXmlExtra getFirstChild() const;
	ofXmlExtra getLastChild() const;
	
	ofXmlExtra getParent() const;


	Attribute getAttribute(const std::string & name) const;
	Range<ofXmlExtraAttributeIterator> getAttributes() const;
	Attribute getFirstAttribute() const;
	Attribute getLastAttribute() const;
	Attribute appendAttribute(const std::string & name);
	Attribute prependAttribute(const std::string & name);
	bool removeAttribute(const std::string & name);
	bool removeAttribute(const Attribute & attr);
	bool removeAttribute(Attribute && attr);

	template<typename T>
	Attribute setAttribute(const std::string & name, const T & value){
		auto attr = getAttribute(name);
		if(!attr){
			attr = appendAttribute(name);
		}
		attr.set(value);
		return attr;
	}

	ofXmlExtra findFirst(const std::string & path) const;
	Search find(const std::string & path) const;

	template<typename T>
	T getValue() const{
		return ofFromString<T>(this->xml.text().as_string());
	}

	std::string getValue() const;
	std::string getName() const;

	template<typename T>
	void set(const T & value){
		if(!xml){
			xml = doc->append_child(pugi::node_element);
		}
		auto child = this->xml.first_child();
		if(!child){
			child = this->xml.append_child(pugi::node_pcdata);
		}
		if(child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata){
			child.set_value(ofToString(value).c_str());
		}
	}

	void set(const unsigned char & value){
		if(!xml){
			xml = doc->append_child(pugi::node_element);
		}
		auto child = this->xml.first_child();
		if(!child){
			child = this->xml.append_child(pugi::node_pcdata);
		}
		if(child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata){
			child.set_value(ofToString(int(value)).c_str());
		}
	}


	void setName(const std::string & name);

	int getIntValue() const;
	unsigned int getUintValue() const;
	float getFloatValue() const;
	double getDoubleValue() const;
	bool getBoolValue() const;

	operator bool() const;

private:
	ofXmlExtra(std::shared_ptr<pugi::xml_document> doc, const pugi::xml_node & xml);
	std::shared_ptr<pugi::xml_document> doc;
	pugi::xml_node xml;

	template<typename It>
	friend class ofXmlExtraIterator;
	friend class ofXmlExtraAttributeIterator;
	friend class ofXmlExtraSearchIterator;
};

template<typename It>
class ofXmlExtraIterator{
public:
	ofXmlExtraIterator(){}

	// Iterator operators
	bool operator==(const ofXmlExtraIterator& rhs) const{
		return this->xml.xml == rhs.xml.xml;
	}

	bool operator!=(const ofXmlExtraIterator& rhs) const{
		return this->xml.xml != rhs.xml.xml;
	}

	const ofXmlExtra& operator*() const{
		return this->xml;
	}

	const ofXmlExtra* operator->() const{
		return &this->xml;
	}

	ofXmlExtra& operator*(){
		return this->xml;
	}

	ofXmlExtra* operator->(){
		return &this->xml;
	}

	const ofXmlExtraIterator& operator++(){
		this->xml = xml.getNextSibling();
		return *this;
	}

	ofXmlExtraIterator operator++(int){
		auto now = xml;
		this->xml = xml.getNextSibling();
		return now;
	}

	const ofXmlExtraIterator& operator--(){
		this->xml = xml.getPreviousSibling();
		return *this;
	}

	ofXmlExtraIterator operator--(int){
		auto now = xml;
		this->xml = xml.getPreviousSibling();
		return now;
	}
	typedef It Base;
	typedef pugi::xml_node Node;
private:

	// Construct an iterator which points to the specified node
	ofXmlExtraIterator(std::shared_ptr<pugi::xml_document> doc, const pugi::xml_node & xml)
	:xml(doc, xml){

	}

	// Construct an iterator which points to the specified node
	ofXmlExtraIterator(ofXmlExtra && xml)
	:xml(xml){

	}
	mutable ofXmlExtra xml;
	friend class ofXmlExtra;
};

class ofXmlExtraAttributeIterator{
public:
	ofXmlExtraAttributeIterator(){}

	// Iterator operators
	bool operator==(const ofXmlExtraAttributeIterator& rhs) const{
		return this->attr == rhs.attr;
	}

	bool operator!=(const ofXmlExtraAttributeIterator& rhs) const{
		return this->attr != rhs.attr;
	}

	const ofXmlExtra::Attribute & operator*() const{
		return this->attr;
	}

	const ofXmlExtra::Attribute* operator->() const{
		return &this->attr;
	}

	ofXmlExtra::Attribute & operator*(){
		return this->attr;
	}

	ofXmlExtra::Attribute* operator->(){
		return &this->attr;
	}

	const ofXmlExtraAttributeIterator& operator++(){
		this->attr = attr.getNextAttribute();
		return *this;
	}

	ofXmlExtraAttributeIterator operator++(int){
		auto now = attr;
		this->attr = attr.getNextAttribute();
		return now;
	}

	const ofXmlExtraAttributeIterator& operator--(){
		this->attr = attr.getPreviousAttribute();
		return *this;
	}

	ofXmlExtraAttributeIterator operator--(int){
		auto now = attr;
		this->attr = attr.getPreviousAttribute();
		return now;
	}

	typedef pugi::xml_attribute_iterator Base;
	typedef pugi::xml_attribute Node;
private:

	// Construct an iterator which points to the specified node
	ofXmlExtraAttributeIterator(std::shared_ptr<pugi::xml_document>, const ofXmlExtra::Attribute & attr)
	:attr(attr){

	}

	ofXmlExtraAttributeIterator(const ofXmlExtra::Attribute & attr)
	:attr(attr){

	}
	ofXmlExtra::Attribute attr;
	friend class ofXmlExtra;
};


class ofXmlExtraSearchIterator{
public:
	ofXmlExtraSearchIterator();

	// Iterator operators
	bool operator==(const ofXmlExtraSearchIterator& rhs) const;
	bool operator!=(const ofXmlExtraSearchIterator& rhs) const;

	ofXmlExtra & operator*() const;
	ofXmlExtra * operator->() const;

	const ofXmlExtraSearchIterator& operator++();
	ofXmlExtraSearchIterator operator++(int);

	const ofXmlExtraSearchIterator& operator--();
	ofXmlExtraSearchIterator operator--(int);

private:
	ofXmlExtraSearchIterator(std::shared_ptr<pugi::xml_document> doc, const pugi::xpath_node * node)
		:node(node)
	{
		if(node){
			xml = ofXmlExtra(doc, node->node());
		}
	}
	const pugi::xpath_node * node = nullptr;
	mutable ofXmlExtra xml;
	friend ofXmlExtra::Search;
};
// serializer
void ofSerialize(ofXmlExtra & xml, const ofAbstractParameter & parameter);
void ofDeserialize(const ofXmlExtra & xml, ofAbstractParameter & parameter);
