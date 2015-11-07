#ifndef GP2X_WRAPPER
#define GP2X_WRAPPER

/*
#define malloc(size) gp2x_malloc(size)
#define calloc(n,size) gp2x_calloc(n,size)
#define realloc(ptr,size) gp2x_realloc(ptr,size)
#define free(size) gp2x_free(size)
*/

#define printf gp2x_printf
//#define printf if(0)printf

#if defined(__cplusplus)
extern "C" {
#endif

extern void gp2x_printf(char* fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif
