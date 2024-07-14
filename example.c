#define MESAURE_PAGEFAULT_IMPLEMENTATION
#include "measure_pagefault.h"

#include <sys/mman.h>
#include <stdio.h>

int main()
{
	struct page_fault_ctx ctx = start_pagefault_counter();

	size_t size = 4096 * 32 * 4;
	int *data = mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	mprotect(data, size, PROT_READ|PROT_WRITE);

	for (int i = 0; i < size / sizeof(int); i++)
	{
		data[i] = 0;
	}

	size_t page_faults_count = end_pagefault_counter(&ctx);
	printf("Page fault count: %zu\n", page_faults_count);

	return 0;
}
