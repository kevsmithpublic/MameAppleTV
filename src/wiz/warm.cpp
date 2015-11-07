/*
 * wARM - exporting ARM processor specific privileged services to userspace
 * userspace part
 *
 * Copyright (c) Gražvydas "notaz" Ignotas, 2009
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organization nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <errno.h>

#define WARM_CODE
#include "warm.h"
#include "sys_cacheflush.h"

static int warm_fd = -1;

int warm_init(void)
{
	struct utsname unm;
	char buff[128];

	warm_fd = open("/proc/warm", O_RDWR);
	if (warm_fd >= 0)
		return 0;

	memset(&unm, 0, sizeof(unm));
	uname(&unm);
	snprintf(buff, sizeof(buff), "/sbin/insmod warm_%s.ko", unm.release);

	/* try to insmod */
	system(buff);
	warm_fd = open("/proc/warm", O_RDWR);
	if (warm_fd >= 0)
		return 0;

	fprintf(stderr, "wARM: can't init, acting as sys_cacheflush wrapper\n");
	return -1;
}

void warm_finish(void)
{
	if (warm_fd >= 0)
		close(warm_fd);
	system("rmmod warm");
}

int warm_cache_op_range(int op, void *addr, unsigned long size)
{
	struct warm_cache_op wop;
	int ret;

	if (warm_fd < 0) {
		sys_cacheflush(addr, (char *)addr + size);
		return -1;
	}

	wop.ops = op;
	wop.addr = (unsigned long)addr;
	wop.size = size;

	ret = ioctl(warm_fd, WARMC_CACHE_OP, &wop);
	if (ret != 0) {
		perror("WARMC_CACHE_OP failed");
		return -1;
	}

	return 0;
}

int warm_cache_op_all(int op)
{
	return warm_cache_op_range(op, NULL, (size_t)-1);
}

int warm_change_cb_range(int cb, int is_set, void *addr, unsigned long size)
{
	struct warm_change_cb ccb;
	int ret;

	if (warm_fd < 0)
		return -1;
	
	ccb.addr = (unsigned long)addr;
	ccb.size = size;
	ccb.cb = cb;
	ccb.is_set = is_set;

	ret = ioctl(warm_fd, WARMC_CHANGE_CB, &ccb);
	if (ret != 0) {
		perror("WARMC_CHANGE_CB failed");
		return -1;
	}

	return 0;
}

int warm_change_cb_upper(int cb, int is_set)
{
	return warm_change_cb_range(cb, is_set, 0, 0);
}

unsigned long warm_virt2phys(const void *ptr)
{
	unsigned long ptrio;
	int ret;

	ptrio = (unsigned long)ptr;
	ret = ioctl(warm_fd, WARMC_VIRT2PHYS, &ptrio);
	if (ret != 0) {
		perror("WARMC_VIRT2PHYS failed");
		return (unsigned long)-1;
	}

	return ptrio;
}

