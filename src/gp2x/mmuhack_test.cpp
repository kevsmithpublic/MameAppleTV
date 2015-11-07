#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>

int mmuhack(void)
{
	int mmufd = open("/dev/mmuhack", O_RDWR);
	if(mmufd < 0) {
		system("/sbin/insmod mmuhack.o");
		mmufd = open("/dev/mmuhack", O_RDWR);
	}
	if(mmufd < 0) return 0;

	close(mmufd);
	return 1;
}

void benchmark (void *memptr)
{
    int starttime = time (NULL);
    int a,b,c,d;
    volatile unsigned int *pp = (unsigned int *) memptr;

    while (starttime == time (NULL));

    printf ("read test\n", memptr);
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                b += pp[a];
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf ("write test\n");
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                pp[a] = 0x37014206;
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf  ("combined test (read, write back)\n");
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                pp[a] += 0x55017601;
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf ("test complete\n");
}

int main( int argc, char* argv[] )
{
	int memfd = open("/dev/mem", O_RDWR);
	if(memfd < 0) return 0;

	volatile unsigned int *myBuf = (volatile unsigned int *)mmap((void *)0, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x03000000);
	volatile unsigned int *myBuf2 = (volatile unsigned int *)mmap((void *)0, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x02000000);
	volatile unsigned int *secbuf = (unsigned int *)malloc(204800);

	mmuhack();

	memset ((void *)myBuf, 0x55, 65536);
	memset ((void *)myBuf2, 0x55, 65536);
	memset ((void *)secbuf, 0x55, 65536);

	printf("mmaped 0x03000000 buffer @ VA: %08X\n\n", myBuf);
	benchmark ((void*)myBuf);
	printf("\nmmaped 0x02000000 buffer @ VA: %08X\n\n", myBuf2);
	benchmark ((void*)myBuf2);
	printf("\nmalloc'd buffer @ VA: %08X\n\n", secbuf);
	benchmark ((void*)secbuf);

	printf ("\nClosing files...\n");
	free((void *)secbuf);
	close (memfd);
	printf ("Exiting...\n");

	return 0;
}
