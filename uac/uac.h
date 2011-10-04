#ifndef __UAC_H__USER_AGENT_CLASSIFIER__
#define __UAC_H__USER_AGENT_CLASSIFIER__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int uac_init(const char *fname);
void uac_free();

uint32_t uac_determine(const char *s);
uint32_t uac_determine_binary(const char *s, size_t sz);

int uac_texts_init(const char *fname);
void uac_texts_free();

const char *uac_texts_browser(uint8_t v);
const char *uac_texts_os(uint8_t v);

#ifdef __cplusplus
}
#endif

#endif /* __UAC_H__USER_AGENT_CLASSIFIER__ */

