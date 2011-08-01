#ifndef __DIC_H__
#define __DIC_H__

#include <MAFSA/adapter.h>

#define SIMPLE_LETTER_A            0
#define SIMPLE_LETTER_Z           25
#define SIMPLE_LETTER_DEFIS       26
#define SIMPLE_LETTER_APOSTROPHE  27
#define SIMPLE_LETTER_SPACE       28
#define SIMPLE_LETTER_DELIM       29

#ifdef __cplusplus
extern "C" {
#endif

size_t SIMPLE_conv_string_to_letters(const char *s, MAFSA_letter *l, size_t sz_l);
size_t SIMPLE_conv_letters_to_string(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s);

#define MAX_OUT 1024

#ifdef __cplusplus
}
#endif

#endif /* __DIC_H__ */
