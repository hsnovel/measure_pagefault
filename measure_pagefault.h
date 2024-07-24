#ifndef MEASURE_PAGEFAULT_H
#define MEASURE_PAGEFAULT_

#include <stddef.h>
#include <linux/perf_event.h>

enum page_fault_measure_flags
{
	MEASURE_HARD_FAULTS = 0x1,
	MEASURE_SOFT_FAULTS = 0x2,
};

// If both flags are used soft flag is used to capture
// both instead of having two different reads.
struct page_fault_ctx
{
	int soft_fd;
	int hard_fd;
	int failed;

	enum page_fault_measure_flags flags;

	struct perf_event_attr soft_attr;
	struct perf_event_attr hard_attr;
};

struct page_fault_result
{
	size_t soft_fault_count;
	size_t hard_fault_count;
};

struct page_fault_ctx start_pagefault_counter();
struct page_fault_result get_pagefault_counter(struct page_fault_ctx *ctx);
struct page_fault_result end_pagefault_counter(struct page_fault_ctx *ctx);

/******************/
/* IMPLEMENTATION */
/******************/
#if defined(MEASURE_PAGEFAULT_IMPLEMENTATION)

#define _GNU_SOURCE
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>

#include <linux/hw_breakpoint.h> /* Definition of HW_* constants */
#include <sys/syscall.h>         /* Definition of SYS_* constants */
#include <unistd.h>
#include <string.h>

// The error can be printed with strerror(errno) if the function
// returns -1
struct page_fault_ctx start_pagefault_counter(enum page_fault_measure_flags flags)
{
	struct page_fault_ctx ctx;
	memset(&ctx, 0, sizeof(struct page_fault_ctx));
	ctx.flags = flags;

	if (flags & MEASURE_SOFT_FAULTS)
	{
		ctx.soft_attr.config = PERF_COUNT_SW_PAGE_FAULTS_MIN;
		ctx.soft_attr.size = sizeof(ctx.soft_attr);
		ctx.soft_attr.type =   PERF_TYPE_SOFTWARE;
		ctx.soft_attr.disabled = 1;
		ctx.soft_attr.exclude_kernel = 1;
		ctx.soft_fd = syscall(SYS_perf_event_open, &ctx.soft_attr, 0, -1, -1, 0);

		if (ctx.soft_fd == -1) {
			ctx.failed = 1;
			return ctx;
		}

		if (ioctl(ctx.soft_fd, PERF_EVENT_IOC_RESET, 0) == -1) {
			return ctx;
		}

		if (ioctl(ctx.soft_fd, PERF_EVENT_IOC_ENABLE, 0) == -1) {
			ctx.failed = 1;
			return ctx;
		}
	}

	if (flags & MEASURE_HARD_FAULTS)
	{
		ctx.hard_attr.config = PERF_COUNT_SW_PAGE_FAULTS_MAJ;
		ctx.hard_attr.size = sizeof(ctx.hard_attr);
		ctx.hard_attr.type =   PERF_TYPE_SOFTWARE;
		ctx.hard_attr.disabled = 1;
		ctx.hard_attr.exclude_kernel = 1;
		ctx.hard_fd = syscall(SYS_perf_event_open, &ctx.hard_attr, 0, -1, -1, 0);

		if (ctx.hard_fd == -1) {
			ctx.failed = 1;
			return ctx;
		}

		ioctl(ctx.soft_fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(ctx.hard_fd, PERF_EVENT_IOC_ENABLE, 0);
	}

	return ctx;
}

struct page_fault_result get_pagefault_counter(struct page_fault_ctx *ctx)
{
	struct page_fault_result result = {};

	if (ctx->flags & MEASURE_SOFT_FAULTS) {
		read(ctx->soft_fd, &result.soft_fault_count, sizeof(result.soft_fault_count));
	}

	if (ctx->flags & MEASURE_HARD_FAULTS) {
		read(ctx->hard_fd, &result.hard_fault_count, sizeof(result.hard_fault_count));
	}

	return result;
}

struct page_fault_result end_pagefault_counter(struct page_fault_ctx *ctx)
{
	struct page_fault_result result = {};

	if (ctx->flags & MEASURE_SOFT_FAULTS) {
		ioctl(ctx->soft_fd, PERF_EVENT_IOC_DISABLE, 0);
		read(ctx->soft_fd, &result.soft_fault_count, sizeof(result.soft_fault_count));
	}

	if (ctx->flags & MEASURE_HARD_FAULTS) {
		ioctl(ctx->hard_fd, PERF_EVENT_IOC_DISABLE, 0);
		read(ctx->hard_fd, &result.hard_fault_count, sizeof(result.hard_fault_count));
	}

	return result;
}

#endif // MESAURE_PAGEFAULT_IMPLEMENTATION

#endif // MEASURE_PAGEFAULT_H
