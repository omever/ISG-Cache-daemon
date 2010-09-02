#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>
#include <sstream>
#include "queryresult.h"

const std::string queryResult::__emptystring = "";
const bool queryResult::__true = true;

queryResult::queryResult()
	: std::vector<std::multimap<std::string, std::string> >()
{
}

void queryResult::add_bind(const std::string &name, const std::string &value, bool is_null)
{
	__bindouts[name].first = value;
	__bindouts[name].second = is_null;
}

void queryResult::add_bind(const std::string &name, const char *value, bool is_null)
{
	__bindouts[name].first = value;
	__bindouts[name].second = is_null;
}

void queryResult::add_bind(const char *name, const std::string &value, bool is_null)
{
	__bindouts[name].first = value;
	__bindouts[name].second = is_null;
}

void queryResult::add_bind(const char *name, const char *value, bool is_null)
{
	__bindouts[name].first = value;
	__bindouts[name].second = is_null;
}

const std::string & queryResult::get_bind_value(std::string &name)
{
	if(__bindouts.count(name) > 0) {
		return __bindouts[name].first;
	} else {
		return queryResult::__emptystring;
	}
}

const bool &queryResult::bind_isnull(std::string &name)
{
	if(__bindouts.count(name) > 0) {
		return __bindouts[name].second;
	} else {
		return queryResult::__true;
	}
}

const std::string queryResult::to_xml()
{
	xmlDocPtr doc;
	xmlNodePtr root;
	int rc;
	std::string returnValue;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "result");

	std::stringstream temp;
	temp << size();
	xmlNewProp(root, BAD_CAST "count", BAD_CAST temp.str().c_str()); 
	xmlDocSetRootElement(doc, root);


	for(int i=0; i<size(); ++i) {
			xmlNodePtr branch = xmlNewChild(root, NULL, BAD_CAST "branch", NULL);
	
			std::multimap<std::string, std::string>::iterator it, iend = at(i).end();
			for(it=at(i).begin(); it!=iend; ++it) {
						xmlNodePtr child = xmlNewChild(branch, NULL, BAD_CAST it->first.c_str(), BAD_CAST it->second.c_str());
					}
		}

	xmlNodePtr bind = xmlNewChild(root, NULL, BAD_CAST "bind", NULL);
	std::map<std::string, std::pair<std::string, bool> >::iterator j, jend = __bindouts.end();
	for(j = __bindouts.begin(); j != jend; ++j) {
		if(j->second.second == true)
			continue;
		xmlNodePtr child = xmlNewChild(bind, NULL, BAD_CAST j->first.c_str(), BAD_CAST j->second.first.c_str());
	}

	xmlBufferPtr buf = xmlBufferCreate();
	xmlSaveCtxtPtr sav = xmlSaveToBuffer(buf, "UTF-8", XML_SAVE_FORMAT | XML_SAVE_NO_EMPTY);

	xmlSaveDoc(sav, doc);
	xmlSaveClose(sav);

	returnValue = (char*)buf->content;

	xmlBufferFree(buf);
	xmlFreeDoc(doc);

	return returnValue;
}
