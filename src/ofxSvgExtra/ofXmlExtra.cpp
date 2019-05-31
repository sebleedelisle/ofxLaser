#include "ofXmlExtra.h"
#include "ofUtils.h"

using namespace std;

ofXmlExtra::ofXmlExtra()
:doc(new pugi::xml_document){
	xml = doc->root();
}

ofXmlExtra::ofXmlExtra(std::shared_ptr<pugi::xml_document> doc, const pugi::xml_node & xml)
:doc(doc)
,xml(xml){

}

bool ofXmlExtra::load(const std::filesystem::path & file){
	auto auxDoc = std::make_shared<pugi::xml_document>();
	if(auxDoc->load_file(ofToDataPath(file).c_str())){
		doc = auxDoc;
		xml = doc->root();
		return true;
	}else{
		return false;
	}
}

bool ofXmlExtra::load(const ofBuffer & buffer){
	return parse(buffer.getText());
}

bool ofXmlExtra::parse(const std::string & xmlStr){
	auto auxDoc = std::make_shared<pugi::xml_document>();
	if(auxDoc->load(xmlStr.c_str())){
		doc = auxDoc;
		xml = doc->root();
		return true;
	}else{
		return false;
	}
}

bool ofXmlExtra::save(const std::filesystem::path & file) const{
	if(xml == doc->root()){
		return doc->save_file(ofToDataPath(file).c_str());
	}else{
		pugi::xml_document doc;
		if(doc.append_copy(xml.root())){
			return doc.save_file(ofToDataPath(file).c_str());
		}
	}
	return false;
}

void ofXmlExtra::clear(){
	doc.reset(new pugi::xml_document);
	xml = doc->root();
}

std::string ofXmlExtra::toString(const std::string & indent) const{
	ostringstream stream;
	if(xml == doc->root()){
		doc->print(stream, indent.c_str());
	}else{
		pugi::xml_document doc;
		doc.append_copy(xml.root());
	}
	return stream.str();
}

ofXmlExtra ofXmlExtra::getChild(const std::string & name) const{
	return ofXmlExtra(doc, xml.child(name.c_str()));
}

ofXmlExtra::Range<ofXmlExtraIterator<pugi::xml_node_iterator>> ofXmlExtra::getChildren() const{
	return ofXmlExtra::Range<ofXmlExtraIterator<pugi::xml_node_iterator>>(doc, xml.children());
}

ofXmlExtra::Range<ofXmlExtraIterator<pugi::xml_named_node_iterator>> ofXmlExtra::getChildren(const std::string & name) const{
	return ofXmlExtra::Range<ofXmlExtraIterator<pugi::xml_named_node_iterator>>(doc, xml.children(name.c_str()));
}

ofXmlExtra ofXmlExtra::appendChild(const ofXmlExtra & xml){
	return ofXmlExtra(doc, this->xml.append_copy(xml.xml));
}

ofXmlExtra ofXmlExtra::prependChild(const ofXmlExtra & xml){
	return ofXmlExtra(doc, this->xml.prepend_copy(xml.xml));
}

#if PUGIXML_VERSION>=170
ofXmlExtra ofXmlExtra::appendChild(ofXmlExtra && xml){
	return ofXmlExtra(doc, this->xml.append_move(xml.xml));
}

ofXmlExtra ofXmlExtra::prependChild(ofXmlExtra && xml){
	return ofXmlExtra(doc, this->xml.prepend_move(xml.xml));
}

bool ofXmlExtra::removeChild(ofXmlExtra && node){
	return xml.remove_child(node.xml);
}
#endif

ofXmlExtra ofXmlExtra::appendChild(const std::string & name){
	return ofXmlExtra(doc, this->xml.append_child(name.c_str()));
}

ofXmlExtra ofXmlExtra::prependChild(const std::string & name){
	return ofXmlExtra(doc, this->xml.prepend_child(name.c_str()));
}

ofXmlExtra ofXmlExtra::insertChildAfter(const std::string & name, const ofXmlExtra & after){
	return ofXmlExtra(doc, this->xml.insert_child_after(name.c_str(), after.xml));
}

ofXmlExtra ofXmlExtra::insertChildBefore(const std::string & name, const ofXmlExtra & before){
	return ofXmlExtra(doc, this->xml.insert_child_before(name.c_str(), before.xml));
}

bool ofXmlExtra::removeChild(const std::string & name){
	return xml.remove_child(name.c_str());
}

bool ofXmlExtra::removeChild(const ofXmlExtra & node){
	return xml.remove_child(node.xml);
}

ofXmlExtra ofXmlExtra::getNextSibling() const{
	return ofXmlExtra(doc, this->xml.next_sibling());
}

ofXmlExtra ofXmlExtra::getPreviousSibling() const{
	return ofXmlExtra(doc, this->xml.previous_sibling());
}

ofXmlExtra ofXmlExtra::getNextSibling(const std::string & name) const{
	return ofXmlExtra(doc, this->xml.next_sibling(name.c_str()));
}

ofXmlExtra ofXmlExtra::getPreviousSibling(const std::string & name) const{
	return ofXmlExtra(doc, this->xml.previous_sibling(name.c_str()));
}

ofXmlExtra ofXmlExtra::getFirstChild() const{
	return ofXmlExtra(doc, this->xml.first_child());
}

ofXmlExtra ofXmlExtra::getLastChild() const{
	return ofXmlExtra(doc, this->xml.last_child());
}

ofXmlExtra ofXmlExtra::getParent() const {
	return ofXmlExtra(doc, this->xml.parent());
}

ofXmlExtra::Attribute ofXmlExtra::getAttribute(const std::string & name) const{
	return this->xml.attribute(name.c_str());
}

ofXmlExtra::Range<ofXmlExtraAttributeIterator> ofXmlExtra::getAttributes() const{
	return ofXmlExtra::Range<ofXmlExtraAttributeIterator>(doc, this->xml.attributes());
}

ofXmlExtra::Attribute ofXmlExtra::getFirstAttribute() const{
	return this->xml.first_attribute();
}

ofXmlExtra::Attribute ofXmlExtra::getLastAttribute() const{
	return this->xml.last_attribute();
}

ofXmlExtra::Attribute ofXmlExtra::appendAttribute(const std::string & name){
	return this->xml.append_attribute(name.c_str());
}

ofXmlExtra::Attribute ofXmlExtra::prependAttribute(const std::string & name){
	return this->xml.prepend_attribute(name.c_str());
}

bool ofXmlExtra::removeAttribute(const std::string & name){
	auto attr = getAttribute(name);
	if(attr){
		return xml.remove_attribute(attr.attr);
	}else{
		return false;
	}
}

bool ofXmlExtra::removeAttribute(const ofXmlExtra::Attribute & attr){
	return xml.remove_attribute(attr.attr);
}

bool ofXmlExtra::removeAttribute(ofXmlExtra::Attribute && attr){
	return xml.remove_attribute(attr.attr);
}

ofXmlExtra ofXmlExtra::findFirst(const std::string & path) const{
	try{
		return ofXmlExtra(doc, this->xml.select_single_node(path.c_str()).node());
	}catch(pugi::xpath_exception & e){
		return ofXmlExtra();
	}
}

ofXmlExtra::Search ofXmlExtra::find(const std::string & path) const{
	try{
		return ofXmlExtra::Search(doc, this->xml.select_nodes(path.c_str()));
	}catch(pugi::xpath_exception & e){
		ofLogError() << e.what();
		return ofXmlExtra::Search();
	}
}

std::string ofXmlExtra::getValue() const{
	return this->xml.text().as_string();
}

std::string ofXmlExtra::getName() const{
	return this->xml.name();
}

void ofXmlExtra::setName(const std::string & name){
	if(xml==doc->document_element()){
		xml = doc->append_child(pugi::node_element);
	}
	this->xml.set_name(name.c_str());
}

int ofXmlExtra::getIntValue() const{
	return this->xml.text().as_int();
}

unsigned int ofXmlExtra::getUintValue() const{
	return this->xml.text().as_uint();
}

float ofXmlExtra::getFloatValue() const{
	return this->xml.text().as_float();
}

double ofXmlExtra::getDoubleValue() const{
	return this->xml.text().as_double();
}

bool ofXmlExtra::getBoolValue() const{
	return this->xml.text().as_bool();
}

ofXmlExtra::operator bool() const{
	return this->xml;
}



//--------------------------------------------------------
// Attribute

ofXmlExtra::Attribute::Attribute(const pugi::xml_attribute & attr)
:attr(attr){}

std::string ofXmlExtra::Attribute::getValue() const{
	return this->attr.as_string();
}

void ofXmlExtra::Attribute::setName(const std::string & name){
	this->attr.set_name(name.c_str());
}

std::string ofXmlExtra::Attribute::getName() const{
	return this->attr.name();
}

int ofXmlExtra::Attribute::getIntValue() const{
	return this->attr.as_int();
}

unsigned int ofXmlExtra::Attribute::getUintValue() const{
	return this->attr.as_uint();
}

float ofXmlExtra::Attribute::getFloatValue() const{
	return this->attr.as_float();
}

double ofXmlExtra::Attribute::getDoubleValue() const{
	return this->attr.as_double();
}

bool ofXmlExtra::Attribute::getBoolValue() const{
	return this->attr.as_bool();
}

ofXmlExtra::Attribute::operator bool() const{
	return this->attr;
}

ofXmlExtra::Attribute ofXmlExtra::Attribute::getNextAttribute() const{
	return this->attr.next_attribute();
}

ofXmlExtra::Attribute ofXmlExtra::Attribute::getPreviousAttribute() const{
	return this->attr.previous_attribute();
}


//-----------------------------------------------
// Search

// Get collection type
pugi::xpath_node_set::type_t ofXmlExtra::Search::type() const{
	return search.type();
}

// Get collection size
size_t ofXmlExtra::Search::size() const{
	return search.size();
}

// Indexing operator
ofXmlExtra ofXmlExtra::Search::operator[](size_t index) const{
	return ofXmlExtra(doc, search[index].node());
}

// Collection iterators
ofXmlExtraSearchIterator ofXmlExtra::Search::begin() const{
	return ofXmlExtraSearchIterator(doc, search.begin());
}

ofXmlExtraSearchIterator ofXmlExtra::Search::end() const{
	return ofXmlExtraSearchIterator(doc, search.end());
}

// Sort the collection in ascending/descending order by document order
void ofXmlExtra::Search::sort(bool reverse){
	search.sort(reverse);
}

// Get first node in the collection by document order
ofXmlExtra ofXmlExtra::Search::getFirst() const{
	return ofXmlExtra(doc, search.first().node());
}

// Check if collection is empty
bool ofXmlExtra::Search::empty() const{
	return search.empty();
}


ofXmlExtra::Search::Search(std::shared_ptr<pugi::xml_document> doc, pugi::xpath_node_set set)
:doc(doc)
,search(set){}

//----------------------------------------------------
// SearchIterator
ofXmlExtraSearchIterator::ofXmlExtraSearchIterator(){}

// Iterator operators
bool ofXmlExtraSearchIterator::operator==(const ofXmlExtraSearchIterator& rhs) const{
	return this->node == rhs.node;
}

bool ofXmlExtraSearchIterator::operator!=(const ofXmlExtraSearchIterator& rhs) const{
	return this->node != rhs.node;
}

ofXmlExtra & ofXmlExtraSearchIterator::operator*() const{
	return xml;
}

ofXmlExtra * ofXmlExtraSearchIterator::operator->() const{
	return &xml;
}

const ofXmlExtraSearchIterator& ofXmlExtraSearchIterator::operator++(){
	if(node){
		node = node + 1;
		if(node){
			xml.xml = node->node();
		}
	}
	return *this;
}

ofXmlExtraSearchIterator ofXmlExtraSearchIterator::operator++(int){
	if(node){
		auto now = *this;
		++(*this);
		return now;
	}else{
		return *this;
	}
}

const ofXmlExtraSearchIterator& ofXmlExtraSearchIterator::operator--(){
	if(node){
		node = node - 1;
		if(node){
			xml.xml = node->node();
		}
	}
	return *this;
}

ofXmlExtraSearchIterator ofXmlExtraSearchIterator::operator--(int){
	if(node){
		auto now = *this;
		--(*this);
		return now;
	}else{
		return *this;
	}
}

void ofSerialize(ofXmlExtra & xml, const ofAbstractParameter & parameter){
	if(!parameter.isSerializable()){
		return;
	}
	string name = parameter.getEscapedName();
	if(name == ""){
		name = "UnknownName";
	}
	ofXmlExtra child	= xml.findFirst(name);
	
	if(!child){
		child = xml.appendChild(name);
		ofLogVerbose("ofXmlExtra") << "creating group " << name;
	}
	if(parameter.type() == typeid(ofParameterGroup).name()){
		const ofParameterGroup & group = static_cast <const ofParameterGroup &>(parameter);
		
		ofLogVerbose("ofXmlExtra") << "group " << name;
		for(auto & p: group){
			ofSerialize(child, *p);
		}
		ofLogVerbose("ofXmlExtra") << "end group " << name;
	}else{
		string value = parameter.toString();
		child.set(value);
	}
}


void ofDeserialize(const ofXmlExtra & xml, ofAbstractParameter & parameter){
	if(!parameter.isSerializable()){
		return;
	}
	string name = parameter.getEscapedName();
	
	ofXmlExtra child = xml.findFirst(name);
	if(child){
		if(parameter.type() == typeid(ofParameterGroup).name()){
			ofParameterGroup & group = static_cast <ofParameterGroup &>(parameter);
			for(auto & p: group){
				ofDeserialize(child, *p);
			}
		}else{
			if(parameter.type() == typeid(ofParameter <int> ).name()){
				parameter.cast <int>() = child.getIntValue();
			}else if(parameter.type() == typeid(ofParameter <float> ).name()){
				parameter.cast <float>() = child.getFloatValue();
			}else if(parameter.type() == typeid(ofParameter <bool> ).name()){
				parameter.cast <bool>() = child.getBoolValue();
			}else if(parameter.type() == typeid(ofParameter <string> ).name()){
				parameter.cast <string>() = child.getValue();
			}else{
				parameter.fromString(child.getValue());
			}
		}
	}
}
