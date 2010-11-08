#ifndef __FDGFBEDRGTBD__UAC_TEXTS_HPP__
#define __FDGFBEDRGTBD__UAC_TEXTS_HPP__

#ifdef __cplusplus
extern "C" {
#endif

void ua_texts_open(const char *fname);
void ua_texts_close();

const char *ua_texts_browser(uint8_t v);
const char *ua_texts_os(uint8_t v)

#ifdef __cplusplus
}
#endif

#endif

