#include "easy_parsing.hpp"

int get_next_word(const char* &be, const char* &en)
{
	for(; (*be == ' ') || (*be == '\t') || (*be == '\n') || (*be == '\r'); be++) {}
	en = be;
	en = strpbrk(be, " \t\n\r");
	if (!en) 
	{
		en = be + strlen(be);
	}
	if (en > be) 
	{
		return 1;
	} 
	else 
	{
		return 0;
	}
}
