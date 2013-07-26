#ifndef __CODA_URL_PARSER_HPP__
#define __CODA_URL_PARSER_HPP__

#include <inttypes.h>

#define CODA_URL_FETCHABLE (1 << 0)
#define CODA_URL_SUBDOMAIN (1 << 1)

class coda_url
{
	char *val_str;
	uint32_t val_len;
	char *dom_str;
	uint32_t dom_len;

	int create_suburl(const char *u, size_t sz, uint32_t flags, const coda_url &parent);
	int create_scheme(const char *u, size_t sz, uint32_t flags);

	int assign_anchor(char *beg, uint32_t flags);
	int assign_refers(char *beg, uint32_t flags);

	int reset();

public:
	coda_url();
	coda_url(const coda_url &u);
	~coda_url();

	coda_url &operator= (const coda_url &u);

	int create_absolute(const char *u, size_t sz, uint32_t flags);
	int create_relative(const char *u, size_t sz, uint32_t flags, const coda_url &parent);

	const char *get_url() const { return (!empty()) ? val_str : ""; }
	const char *get_domain() const { return (!empty()) ? dom_str : ""; }

	uint32_t get_url_len() const { return val_len; }
	uint32_t get_domain_len() const { return dom_len; }

	bool empty() const { return 0 == val_len; }
	bool allocated() const { return NULL != val_str; }
};

#endif /* __CODA_URL_PARSER_HPP__ */
