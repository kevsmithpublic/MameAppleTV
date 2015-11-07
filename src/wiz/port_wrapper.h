#ifndef WIZ_WRAPPER
#define WIZ_WRAPPER

#define malloc(size) wiz_malloc(size)
#define calloc(n,size) wiz_calloc(n,size)
#define realloc(ptr,size) wiz_realloc(ptr,size)
#define free(size) wiz_free(size)

#define printf wiz_printf

extern void wiz_printf(char* fmt, ...);

#endif
