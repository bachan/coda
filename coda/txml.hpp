#ifndef __CODA_XML_PARSER_HPP__
#define __CODA_XML_PARSER_HPP__

#include <stdint.h>
#include <expat.h>
#include <string>
#include <vector>
#include <list>

#define txml_member(p,m) (p)->determineMember(#m, (m))

namespace coda {

struct txml_parser;
struct txml_determination_object
{
	virtual ~txml_determination_object() {}
	virtual void determine(txml_parser* p) = 0;

	void load_from_file(const char* filename);
	void load_from_string(const char* s);
	void load_from_string(const char* s, size_t len);
};

struct txml_parser
{
	struct level_desc
	{
		std::string key;
		int pushed;

		level_desc(const char *k)
			: key(k)
			, pushed(0)
		{}
	};

	XML_Parser p;
	txml_determination_object* data;

	std::string filename;
	std::string current_value;
	std::vector<level_desc> levels;
	std::vector<level_desc>::iterator det_iter;

	void parse(const char* szDataSource, unsigned int iDataLength, bool bIsFinal = true);
	void raise(const char* err);

	void determine();
	void setValue(std::string& var);
	void setValue(       bool& var);
	void setValue(    int32_t& var);
	void setValue(   uint32_t& var);
	void setValue(    int64_t& var);
	void setValue(   uint64_t& var);
	void setValue(    uint8_t& var);
	void setValue(      float& var);
	void setValue(     double& var);
	void setValue(long double& var);

	template <typename _T>
	void setValue(_T& var)
	{
		var.determine(this);
	}

	template <typename _T>
	void setValue(std::list<_T>& var)
	{
		--det_iter;

		if (!det_iter->pushed)
		{
			det_iter->pushed = 1;
			var.push_back(_T());
		}

		_T& item = var.back();
		++det_iter;
		setValue(item);
	}

	template <typename _T>
	void setValue(std::vector<_T>& var)
	{
		--det_iter;

		if (!det_iter->pushed)
		{
			det_iter->pushed = 1;
			var.push_back(_T());
		}

		_T& item = var.back();
		++det_iter;
		setValue(item);
	}

	txml_parser(txml_determination_object *d);
	~txml_parser();

	template <typename _T>
	bool determineMember(const std::string& key, _T& var)
	{
		if (levels.end() == det_iter)
		{
			if (!key.empty()) return false;
			setValue(var);
			return true;
		}

		if (key == det_iter->key)
		{
			det_iter++;
			setValue(var);
			det_iter--;
			return true;
		}

		return false;
	}

	void characters(const char* szChars, unsigned int iLength);
	void begelement(const char* szName, const char** pszAttributes);
	void endelement(const char* szName);
};

} /* namespace coda */

#endif /* __CODA_XML_PARSER_HPP__ */
