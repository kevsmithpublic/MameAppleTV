#ifndef _WIZ_CACHEFLUSH

#ifdef __cplusplus
extern "C" {
#endif

extern void sys_cacheflush(void *start_addr, void *end_addr);

extern void spend_cycles(int c);

#ifdef __cplusplus
} /* End of extern "C" */
#endif

#endif
