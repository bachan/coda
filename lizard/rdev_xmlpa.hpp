#ifndef __RDEV_XMLPA_HPP__
#define __RDEV_XMLPA_HPP__

#include <stdint.h>
#include <expat.h>
#include <string>
#include <vector>
#include <list>

class rdev_xmlparser;

struct rdev_xmlobject
{
	virtual ~rdev_xmlobject() {}
	virtual void determine(rdev_xmlparser *p) = 0;

	void load_from_file(const char *fname);
	void load_from_string(const char *s);
    void load_from_string(const char *s, size_t len);
};

#define DANGER_PARSER

#ifdef DANGER_PARSER
#include <string.h>
#endif
class rdev_xmlparser
{
	struct level_desc
	{
#ifdef DANGER_PARSER
        // NOTE я использую здесь указатель на данные, передаваемые в колбек,
        // в разумном предположении, что expat, как эффективная библиотека,
        // не удаляет и не перемещает строки с именами тегов и параметров,
        // которые являются предками текущего элемента
        const char* key;
#else
		std::string key;
#endif
		int pushed;

	public:
		level_desc(const char *k) : key(k), pushed(0) {}
	};

	friend class rdev_xmlobject;

	XML_Parser p;
	rdev_xmlobject *data;

	std::string filename;
	std::string current_value;
	std::vector<level_desc> levels;
	std::vector<level_desc>::iterator det_iter;

	/* parse */

	void raise(const char *err);
	void parse(const char *szDataSource, unsigned int iDataLength, bool bIsFinal = true);

	void determine();
	void setValue(std::string &var);
	void setValue(       bool &var);
	void setValue(    int32_t &var);
	void setValue(   uint32_t &var);
	void setValue(    int64_t &var);
	void setValue(   uint64_t &var);
	void setValue(    uint8_t &var);
    void setValue(      float &var);
    void setValue(     double &var);
    void setValue(long double &var);

	template <typename _T> void setValue(_T &var)
	{
		var.determine(this);
	}

	template <typename _T> void setValue(std::list<_T> &var)
	{
		det_iter--;

		if (!det_iter->pushed)
		{
			det_iter->pushed = 1;
			var.push_back(_T());
		}

		_T &item = var.back();
		det_iter++;
		setValue(item);
	}

	template <typename _T> void setValue(std::vector<_T> &var)
	{
		det_iter--;

		if (!det_iter->pushed)
		{
			det_iter->pushed = 1;
			var.push_back(_T());
		}

		_T &item = var.back();
		det_iter++;
		setValue(item);
	}

public:
	 rdev_xmlparser(rdev_xmlobject *d);
	~rdev_xmlparser();

	template <typename _T> bool determineMember(const std::string &key, _T &var)
	{
		if (levels.end() == det_iter)
		{
			if (!key.empty()) return false;
			setValue(var);
			return true;
		}

#ifdef DANGER_PARSER
        if (!strcmp(key.c_str(), det_iter->key))
#else
		if (key == det_iter->key)
#endif
		{
			det_iter++;
			setValue(var);
			det_iter--;
			return true;
		}

		return false;
	}

	void chars(const char *szChars, unsigned int iLength);
	void begelt(const char *szName, const char **pszAttributes);
	void endelt(const char *szName);
};

#endif /* __RDEV_XMLPA_HPP__ */
