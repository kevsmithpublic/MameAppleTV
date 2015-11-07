#ifndef __WIZUPPERMEM__
#define __WIZUPPERMEM__

#include <string.h>

extern void upper_malloc_init(void *pointer);
extern void *wiz_malloc(size_t size);
extern void *wiz_calloc(size_t n, size_t size);
extern void *wiz_realloc(void *ptr, size_t size);
extern void wiz_free(void *ptr);
extern void *upper_take(int Start, size_t Size);

#endif /* __WIZUPPERMEM__ */
