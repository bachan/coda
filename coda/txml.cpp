#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "error.hpp"
#include "txml.hpp"

#define BUFSZ 4096

void coda::txml_determination_object::load_from_file(const char* filename)
{
	coda::txml_parser p (this);
	p.filename = filename;

	FILE *fp = fopen(filename, "r");

	if (!fp)
	{
		throw coda_errno(errno, "Can't open file '%s'", filename);
	}

	char buf [BUFSZ];

	try
	{
		while (!feof(fp))
		{
			size_t sz = fread(buf, sizeof(char), BUFSZ, fp);

			if (0 != ferror(fp))
			{
				throw coda_errno(errno, "Can't read from file '%s'", filename);
			}

			p.parse(buf, sz, feof(fp) ? true : false);
		}
	}
	catch (...)
	{
		fclose(fp);
		throw;
	}

	fclose(fp);
}

void coda::txml_determination_object::load_from_string(const char* str)
{
	coda::txml_parser p (this);
	p.parse(str, strlen(str), true);
}

void coda::txml_determination_object::load_from_string(const char* str, size_t len)
{
	coda::txml_parser p (this);
	p.parse(str, len, true);
}

static void cb_characters(void* vThis, const char* szData, int iLength)
{
	((coda::txml_parser*) vThis)->characters(szData, iLength);
}

static void cb_begelement(void* vThis, const char* const szName, const char** pszAttributes)
{
	((coda::txml_parser*) vThis)->begelement(szName, pszAttributes);
}

static void cb_endelement(void* vThis, const char* const szName)
{
	((coda::txml_parser*) vThis)->endelement(szName);
}

static int  cb_encunknown(void*, const XML_Char*, XML_Encoding*)
{
	return 0;
}

coda::txml_parser::txml_parser(coda::txml_determination_object* d)
	: data(d)
{
	p = XML_ParserCreate("UTF-8");
	XML_SetUserData(p, this);
	XML_SetUnknownEncodingHandler(p, cb_encunknown, NULL);
	XML_SetElementHandler(p, cb_begelement, cb_endelement);
	XML_SetCharacterDataHandler(p, cb_characters);
}

coda::txml_parser::~txml_parser()
{
	XML_ParserFree(p);
}

void coda::txml_parser::parse(const char* szDataSource, unsigned int iDataLength, bool bIsFinal)
{
	int iFinal = bIsFinal;

	if (XML_STATUS_ERROR == XML_Parse(p, szDataSource, iDataLength, iFinal))
	{
		raise(XML_ErrorString(XML_GetErrorCode(p)));
	}
}

void coda::txml_parser::raise(const char* err)
{
	throw coda_error("XML error (%s): %s, line %d, column %d",
		filename.c_str(), err,
		(int) XML_GetCurrentLineNumber(p),
		(int) XML_GetCurrentColumnNumber(p));
}

void coda::txml_parser::determine()
{
	det_iter = levels.begin();
	data->determine(this);
}

static bool strtobool(const char *s, size_t sz) /* usual on/off switcher -> bool */
{
	if (0 !=      strtol(s, NULL,    10)) return true;
	if (0 == strncasecmp(s, "0",     sz)) return false;

	if (0 == strncasecmp(s, "true",  sz)) return true;
	if (0 == strncasecmp(s, "false", sz)) return false;

	if (0 == strncasecmp(s, "on",    sz)) return true;
	if (0 == strncasecmp(s, "off",   sz)) return false;

	if (0 == strncasecmp(s, "yes",   sz)) return true;
	if (0 == strncasecmp(s, "no",    sz)) return false;

	return false;
}

void coda::txml_parser::setValue(std::string& var) { var = current_value; }
void coda::txml_parser::setValue(       bool& var) { var = strtobool (current_value.c_str(), current_value.size()); }
void coda::txml_parser::setValue(    int32_t& var) { var = strtol    (current_value.c_str(), NULL, 10); }
void coda::txml_parser::setValue(   uint32_t& var) { var = strtoul   (current_value.c_str(), NULL, 10); }
void coda::txml_parser::setValue(    int64_t& var) { var = strtoll   (current_value.c_str(), NULL, 10); }
void coda::txml_parser::setValue(   uint64_t& var) { var = strtoull  (current_value.c_str(), NULL, 10); }
void coda::txml_parser::setValue(    uint8_t& var) { var = strtoul   (current_value.c_str(), NULL, 10); }
void coda::txml_parser::setValue(      float& var) { var = strtof    (current_value.c_str(), NULL    ); }
void coda::txml_parser::setValue(     double& var) { var = strtod    (current_value.c_str(), NULL    ); }
void coda::txml_parser::setValue(long double& var) { var = strtold   (current_value.c_str(), NULL    ); }

void coda::txml_parser::characters(const char* szChars, unsigned int iLength)
{
	current_value.append(szChars, iLength);
}

void coda::txml_parser::begelement(const char* szName, const char** pszAttributes)
{
	levels.push_back(szName);

	for (int i = 0; pszAttributes[i] && pszAttributes[i + 1]; i += 2)
	{
		levels.push_back(pszAttributes[i]);
		current_value = pszAttributes[i + 1];
		determine();
		levels.pop_back();
	}

	/* 2006-02-02 bugfix for list & vector */
	levels.push_back("");
	current_value.clear();
	determine();
	levels.pop_back();
	/* end of bugfix */

	/* current_value.clear(); */
}

void coda::txml_parser::endelement(const char*)
{
	determine();
	current_value.clear();
	levels.pop_back();
}

