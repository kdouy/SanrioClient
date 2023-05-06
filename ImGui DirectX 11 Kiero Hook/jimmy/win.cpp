#include "mem.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

static HANDLE handle;

void readmem(void* buf, const void* addr, size_t size) {
	if (ReadProcessMemory(handle, (void*)addr, buf, size, NULL) == 0)
		memset(buf, 0, size);
}
void writemem(void* buf, void* addr, size_t size) {
	if (WriteProcessMemory(handle, addr, buf, size, NULL) == 0)
		memset(buf, 0, size);
}

void jmy_init(HANDLE process, const void* jvmbase) {
	handle = process;
	jvm = jvmbase;
}
