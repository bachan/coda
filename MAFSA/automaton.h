/*
 * Copyright (C) 2007, libMAFSA Development Team
 *
 * This file is released under the LGPL
 */

#ifndef __MAFSA_AUTOMATON_H__
#define __MAFSA_AUTOMATON_H__

#include "mafsa_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Load and attach errors */
#define MAFSA_ERROR_NOMEM		1
#define MAFSA_ERROR_CANT_OPEN_FILE	2
#define MAFSA_ERROR_CORRUPTED_FILE	3
#define MAFSA_ERROR_CORRUPTED_NODES	4
#define MAFSA_ERROR_CORRUPTED_LINKS	5

struct MAFSA_automaton_struct
{
	u_int32_t *ptr_nodes;
	u_int32_t *ptr_links;
	int shared;
};

typedef struct MAFSA_automaton_struct *MAFSA_automaton;

/* Opens binary file, load it to memory. Remember to free by calling MAFSA_automaton_close()
*/
extern MAFSA_automaton MAFSA_automaton_load_from_binary_file(const char *fname, int *error);

/* Attaches automaton nodes and links
*/
extern MAFSA_automaton MAFSA_automaton_attach(const void *ptr_nodes, const void *ptr_links, int *error);

/* Frees memory, allocated by MAFSA_automaton_load_from_binary_file, Does nothing if 
 * MAFSA_automaton was created by MAFSA_automaton_attach
*/
extern void MAFSA_automaton_close(MAFSA_automaton mautomaton);

/* Checks, whether string *l of letter is in automaton
*/
extern int MAFSA_automaton_find(MAFSA_automaton mautomaton, const MAFSA_letter *l, size_t sz);

extern ssize_t MAFSA_automaton_search(MAFSA_automaton mautomaton, const MAFSA_letter *l, size_t sz, MAFSA_letter *out, size_t max_out_sz, size_t *out_sz);

/* Enumerates all strings, which start with l, if l or sz are 0, enumerates all strings
*/
typedef void (MAFSACALL *MAFSA_automaton_string_handler) (void *user_data, const MAFSA_letter *s, size_t sz);

extern void MAFSA_automaton_enumerate(MAFSA_automaton mautomaton,
                                        const MAFSA_letter *l,
                                        size_t sz,
					MAFSA_letter *tmp,
					size_t tmp_size,
                                        void *user_data,
					MAFSA_automaton_string_handler fetcher);

/* Searches for any substring + delim in automaton in l, enumerates all variants */
extern void MAFSA_automaton_search_enumerate(MAFSA_automaton mautomaton,
                                        const MAFSA_letter *l,
                                        size_t sz,
					MAFSA_letter *tmp,
					size_t tmp_size,
                                        void *user_data,
					MAFSA_letter delim,
					MAFSA_automaton_string_handler fetcher);

typedef unsigned (MAFSACALL *MAFSA_automaton_fuzzy_callback) (void *user_data, const MAFSA_letter* s, size_t ls, const MAFSA_letter* t, size_t lt);

extern void MAFSA_automaton_fuzzy_enumerate(
	MAFSA_automaton mautomaton,
	const MAFSA_letter* l,
	size_t sz,
	MAFSA_letter* tmp,
	size_t sz_tmp,
	void *user_data,
	MAFSA_automaton_fuzzy_callback sdist,
	unsigned max_dist,
	MAFSA_automaton_fuzzy_callback fetcher
);


#ifdef __cplusplus
}
#endif

#endif /* __MAFSA_AUTOMATON_H__ */

