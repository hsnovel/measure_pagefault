#define MESAURE_PAGEFAULT_IMPLEMENTATION
#include "measure_pagefault.h"

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

	struct page_fault_ctx ctx = start_pagefault_counter();
	write_zero(data, size);
	size_t page_faults_count = end_pagefault_counter(&ctx);
	printf("Page fault count without prefaulting: %zu\n", page_faults_count);

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
	ctx = start_pagefault_counter();
	write_zero(data, size);
	page_faults_count = end_pagefault_counter(&ctx);
	printf("Page fault count with prefaulting: %zu\n", page_faults_count);

	return 0;
}
