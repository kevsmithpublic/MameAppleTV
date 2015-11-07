
#include "minimal.h"

/* Call this MMU Hack kernel module after doing mmap, and before doing memset*/
int mmuhack(void)
{
	int mmufd = open("/dev/mmuhack", O_RDWR);
  
  
	if(mmufd < 0) {
		printf ("Installing NK's kernel module for Squidge MMU Hack...\n");
		system("/sbin/insmod mmuhack.o");
		mmufd = open("/dev/mmuhack", O_RDWR);
	}
	if(mmufd < 0) return 0;
	 
	close(mmufd);
	return 1;
}       


/* Unload MMU Hack kernel module after closing all memory devices*/
int mmuunhack(void)
{
  printf ("Removing NK's kernel module for Squidge MMU Hack...\n");
  system("/sbin/rmmod mmuhack");
  return 0;
}
