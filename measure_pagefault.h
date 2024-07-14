#ifndef MEASURE_PAGEFAULT_H
#define MEASURE_PAGEFAULT_

#include <stddef.h>

struct page_fault_ctx
{
	int page_fault_fd;
	int failed;
	struct perf_event_attr page_fault_attr;
};

struct page_fault_ctx start_pagefault_counter();
size_t end_pagefault_counter(struct page_fault_ctx *ctx);

/******************/
/* IMPLEMENTATION */
/******************/
#if defined(MESAURE_PAGEFAULT_IMPLEMENTATION)

#define _GNU_SOURCE
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>

#include <linux/perf_event.h>    /* Definition of PERF_* constants */
#include <linux/hw_breakpoint.h> /* Definition of HW_* constants */
#include <sys/syscall.h>         /* Definition of SYS_* constants */
#include <unistd.h>
#include <string.h>

// The error can be printed with strerror(errno) if the function
// returns -1
struct page_fault_ctx start_pagefault_counter()
{
	struct page_fault_ctx ctx;

	memset(&ctx.page_fault_attr, 0, sizeof(ctx.page_fault_attr));
	ctx.page_fault_attr.size = sizeof(ctx.page_fault_attr);
	ctx.page_fault_attr.type =   PERF_TYPE_SOFTWARE;
	ctx.page_fault_attr.config = PERF_COUNT_SW_PAGE_FAULTS;
	ctx.page_fault_attr.disabled = 1;
	ctx.page_fault_attr.exclude_kernel = 1;
	ctx.page_fault_fd = syscall(SYS_perf_event_open, &ctx.page_fault_attr, 0, -1, -1, 0);
	if (ctx.page_fault_fd == -1) {
		ctx.failed = 1;
		return ctx;
	}

	ioctl(ctx.page_fault_fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(ctx.page_fault_fd, PERF_EVENT_IOC_ENABLE, 0);
	ctx.failed = 0;

	return ctx;
}

size_t end_pagefault_counter(struct page_fault_ctx *ctx)
{
	ioctl(ctx->page_fault_fd, PERF_EVENT_IOC_DISABLE, 0);
	size_t page_faults_count;
	read(ctx->page_fault_fd, &page_faults_count, sizeof(page_faults_count));
	return page_faults_count;
}

#endif // MESAURE_PAGEFAULT_IMPLEMENTATION

#endif // MEASURE_PAGEFAULT_H
