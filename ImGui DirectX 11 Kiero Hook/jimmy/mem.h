#include <stddef.h>
#include <stdint.h>

extern const void* jvm;

void readmem(void* buf, const void* addr, size_t size);
void writemem(void* buf, void* addr, size_t size);
