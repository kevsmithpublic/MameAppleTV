
#include "wiz_lib.h"

#define UPPERMEM_BLOCKSIZE 1024
#define UPPERMEM_START 0x2A00000
#define UPPERMEM_SIZE  (0x4000000-UPPERMEM_START)

static void *uppermem;
static int uppermap[UPPERMEM_SIZE/UPPERMEM_BLOCKSIZE];

static void * upper_malloc(size_t size)
{
  	int i=0,j=0;
 	void *mem = NULL;
 	int BSize;

    if (size%UPPERMEM_BLOCKSIZE==0)
    {
        BSize=size/UPPERMEM_BLOCKSIZE;
    }
    else
    {
        BSize=(size/UPPERMEM_BLOCKSIZE)+1;
    }

  	while (i<UPPERMEM_SIZE/UPPERMEM_BLOCKSIZE)
  	{
		for(j=0;j<BSize;j++)
  		{
    			if (uppermap[i+j])
    			{
      				i+=j;
      				i+=uppermap[i];
      				break;
    			}
  		}
		if (j==BSize) {
			uppermap[i] = BSize;
  			mem = ((char*)uppermem) + (i*UPPERMEM_BLOCKSIZE);
  			return mem;
  		}
	}
	return NULL;
}

void upper_malloc_init(void *pointer)
{
	uppermem=pointer;
}

void * upper_take(int Start, size_t Size)
{
 	int BSize;
 	if (Size%UPPERMEM_BLOCKSIZE==0)
    {
        BSize=Size/UPPERMEM_BLOCKSIZE;
    }
    else
    {
        BSize=(Size/UPPERMEM_BLOCKSIZE)+1;
    }
	uppermap[(Start - UPPERMEM_START) / UPPERMEM_BLOCKSIZE] = BSize;
	return(((char*)uppermem) + (Start-UPPERMEM_START));
}


void * wiz_malloc(size_t size)
{
	void *ptr=NULL;
	if (size>=(2*1024)) {
		ptr=upper_malloc(size);
		if (ptr)
		{
			return (ptr);
		}
	}
	ptr=malloc(size);
	if (ptr)
	{
		return (ptr);
	}
	ptr=upper_malloc(size);
	if (ptr)
	{
		return (ptr);
	}
	printf("Could not malloc %d\n", size);fflush(stdout);
	return (NULL);
}

void * wiz_calloc(size_t n, size_t size)
{
	void *ptr=NULL;
	if ((n*size)>0)
	{
		ptr=wiz_malloc(n*size);
		if (ptr)
			memset (ptr,0,n*size);
	}
	return ptr;
}

void *wiz_realloc(void *ptr, size_t size)
{
	void *ptr_new=NULL;

	if ((ptr==NULL) && (size>0))
	{
		ptr_new=wiz_malloc (size);
	}
	else if ((ptr!=NULL) && (size==0))
	{
		wiz_free (ptr);
	}
	else if ((ptr!=NULL) && (size>0))
	{
	  	int i = (((int)ptr) - ((int)uppermem));
	  	if (i < 0 || i >= UPPERMEM_SIZE)
	  	{
  			ptr_new=realloc(ptr,size);
  			if (ptr_new==NULL)
  			{
  				printf("Could not realloc %d\n", size);fflush(stdout);
  			}

	  	}
	  	else
		{
			ptr_new=wiz_malloc(size);
			if (ptr_new!=NULL)
			{
				memcpy(ptr_new,ptr,uppermap[i/UPPERMEM_BLOCKSIZE]);
				wiz_free(ptr);
			}
		}
	}
	return ptr_new;
}

void wiz_free(void *ptr)
{
  	int i = (((int)ptr) - ((int)uppermem));
  	if (i < 0 || i >= UPPERMEM_SIZE) {
  		free(ptr);
  	} else {
		uppermap[i/UPPERMEM_BLOCKSIZE] = 0;
	}
}
