#include "measure_pagefault.h"

#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>

void write_zero(int *data, size_t size)
{
	for (int i = 0; i < size / sizeof(int); i++)
	{
		data[i] = 0;
	}
}

int main()
{
	////////////////////////////
	// WITHOUT PREFAULTING
	size_t size = 4096 * 32 * 4;
	int *data = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	struct page_fault_ctx ctx = start_pagefault_counter(MEASURE_HARD_FAULTS|MEASURE_SOFT_FAULTS);
	write_zero(data, size);
	struct page_fault_result result = end_pagefault_counter(&ctx);
	printf("Page fault count without prefaulting: %zu\n", result.soft_fault_count + result.hard_fault_count);
	printf("\tSoft faults: %zu\n", result.soft_fault_count);
	printf("\tHard faults: %zu\n", result.hard_fault_count);

	//////////////////////////
	// WITH PREFAULTING

	// Deallocate all the pages as they area all already faulted.
	munmap(data, size);
	data = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (madvise(data, size, MADV_POPULATE_WRITE) == -1)
	{
		printf("madvise() failed\n");
		return 1;
	}

	// Normally this should print 0 but due to COW the page fault number may vary.
	ctx = start_pagefault_counter(MEASURE_SOFT_FAULTS|MEASURE_HARD_FAULTS);
	write_zero(data, size);
	result = end_pagefault_counter(&ctx);
	printf("Page fault count with prefaulting: %zu\n", result.soft_fault_count + result.hard_fault_count);
	printf("\tSoft faults: %zu\n", result.soft_fault_count);
	printf("\tHard faults: %zu\n", result.hard_fault_count);

	return 0;
}
