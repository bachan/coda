#include "uac.h"
#include "sla.h"
#include <MAFSA/automaton.h>
#include <stdio.h>
#include "uac_config.hpp"
#include <map>
#include <string>

static std::map<int, std::string> texts_browser;
static std::map<int, std::string> texts_os;

const char *uac_texts_browser(uint8_t v)
{
	std::map<int, std::string>::const_iterator it = texts_browser.find(v);
	if (it == texts_browser.end()) return 0;
	return it->second.c_str();
}

const char *uac_texts_os(uint8_t v)
{
	std::map<int, std::string>::const_iterator it = texts_os.find(v);
	if (it == texts_os.end()) return 0;
	return it->second.c_str();
}

int uac_texts_init(const char *fname)
{
	try
	{
		uac::config cfg;
		cfg.load_from_file(fname);

		std::vector<uac::config::root::det>::const_iterator it;
		for (it = cfg.r.dets.begin(); it != cfg.r.dets.end(); it++)
		{
			switch (it->typ)
			{
				case 1:
					texts_os[it->id] = it->title;
					break;
				case 2:
					texts_browser[it->id] = it->title;
					break;
			}
		}
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}

void uac_texts_free()
{
	texts_browser.clear();
	texts_os.clear();
}

