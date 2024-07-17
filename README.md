# Measure pagefault

Header only file with only two functions that let you easily measure pagefaults inside any given codeblock on linux.

### Usage
Include the file, if you want the implementation add `#define MESAURE_PAGEFAULT_IMPLEMENTATION` before `#include "measure_pagefault.h"`
You don't need a seperate file like `example_include.c` in the example, but I included it to make it easier for me to debug include
conflicts when developing single header libraries(like forgetting to put the decleration of something on the file etc..), you can simply
put the `#define MESAURE_PAGEFAULT_IMPLEMENTATION` and `#include "measure_pagefault.h"` into the same file and it will work properly.
I seperated only becasue it is easier for me to track include errors.

To start the pagefault counter use `start_pagefault_counter();` It takes one parameter, flags which can be either `MEASURE_HARD_FAULTS`
or `MEASURE_SOFT_FAULTS` or both of them if you or them together. The function returns `page_fault_ctx`, which is a handle to the current
counter, the variable `failed` inside `page_fault_ctx` is set to 1 if there was an error initializing the pagefault counter, the error
can be printed with `strerror(errno)`(not always, if the flags are not valid errno would not be set). Do not forget to store the errno
before doing any operation that can alter errno!

To end the counter call `end_pagefault_counter()` passing the previously returned `struct page_fault_ctx`, the function returns `size_t`
which tells the number of page faults that occoured.

Number of page faults can also be obtained with `get_pagefault_counter()` function, which does not terminate the counter unlike `end_pagefault_counter()`

An example usage is inside `example.c`, which can be compiled with running `./build_debug.sh`, which generates `example` binary which you can run to test the program.

### NOTE
Make sure to `#define _GNU_SOURCE` before including `unistd.h`, the library does it but if you include it before and don't set than
`syscall` will not be avaliable as it is not a POSIX standart, or use a C version that supports gnu like `-std=gnu99` etc..
