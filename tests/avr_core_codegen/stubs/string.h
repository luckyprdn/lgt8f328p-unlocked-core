#ifndef LGT_AVR_STUB_STRING_H
#define LGT_AVR_STUB_STRING_H
#include <stddef.h>
static inline size_t strlen(const char *s){const char *p=s;while(*p)++p;return (size_t)(p-s);}
static inline void *memcpy(void *d,const void *s,size_t n){unsigned char*dd=(unsigned char*)d;const unsigned char*ss=(const unsigned char*)s;while(n--)*dd++=*ss++;return d;}
static inline void *memset(void *d,int c,size_t n){unsigned char*dd=(unsigned char*)d;while(n--)*dd++=(unsigned char)c;return d;}
#endif
