#include <stdio.h>
#include <MAFSA/daciuk.hpp>
#include "uac_config.hpp"
#include "sla.h"



int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("usage: uac_compiler uac-config.xml uac.automaton\n");
		return -1;
	}

	uac::config cfg;
	cfg.load_from_file(argv[1]);

	MAFSA::daciuk<MAX_LETTER_SLA + 1> dic;

	std::vector<uac::config::root::det>::const_iterator it;
	for (it = cfg.r.dets.begin(); it != cfg.r.dets.end(); it++)
	{
		unsigned int C1 = it->typ;
		unsigned int C2 = 12 + (it->id / 26);
		unsigned int C3 = 12 + (it->id % 26);
		
		std::vector<std::string>::const_iterator jt;
		for (jt = it->catch_strings.begin(); jt != it->catch_strings.end(); jt++)
		{
			MAFSA_letter l[1024];
			ssize_t ssz = conv_s2l_sla(jt->c_str(), l, 1024);
			if (ssz > 0)
			{
				l[ssz++] = MAX_LETTER_SLA;
				l[ssz++] = C1;
				l[ssz++] = C2;
				l[ssz++] = C3;
				dic.insert(l, ssz);
			}
			else
			{
				printf("bad string '%s'\n", jt->c_str());
			}
		}
	}

	dic.save_to_file(argv[2]);

	return 0;
}

