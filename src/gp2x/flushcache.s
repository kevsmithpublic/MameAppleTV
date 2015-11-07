.global flushcache

flushcache:	
    swi #0x9f0002
    bx lr
