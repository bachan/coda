#include <stdio.h>
#include <coda/txml.hpp>

struct config : public coda::txml_determination_object
{
	struct ROOT : public coda::txml_determination_object
	{
		std::vector<std::string> s;
		void determine(coda::txml_parser* p)
		{
			p->determineMember("s", s);
		}
	};

	ROOT r;
	void determine(coda::txml_parser* p)
	{
		p->determineMember("r", r);
	}
};

int test1()
{
	config cfg;
	cfg.load_from_string("<r><s>s1</s><s>s2</s></r>");
	if (cfg.r.s.size() != 2) return -1;

	return 0;
}

int main(int argc, char** argv)
{
	if (test1()) return -1;

	return 0;
}
