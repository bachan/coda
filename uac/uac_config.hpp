#ifndef __UAC_CONFIG_HPP__TEHRRRTWHRQWW__
#define __UAC_CONFIG_HPP__TEHRRRTWHRQWW__

#include <string>
#include <vector>
#include <coda/txml.hpp>

namespace uac {

struct config : public coda::txml_determination_object
{
	struct root : public coda::txml_determination_object
	{
		struct det : public coda::txml_determination_object
		{
			int typ;
			int id;
			std::string title;
			std::vector<std::string> catch_strings;

			void determine(coda::txml_parser* parser)
			{
				parser->determineMember("type", typ);
				parser->determineMember("id", id);
				parser->determineMember("catch_string", catch_strings);
				parser->determineMember("title", title);
			}
		};

		std::vector<det> dets;

		void determine(coda::txml_parser* parser)
		{
			parser->determineMember("det", dets);
		}
	};

	root r;
	void determine(coda::txml_parser* parser)
	{
		parser->determineMember("uac", r);
	}
};

}

#endif // __UAC_CONFIG_HPP__TEHRRRTWHRQWW__

