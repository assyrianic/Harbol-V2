#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "../harbol.h"

int32_t test_plugin_load()
{
	puts("loading plugin.");
	return 1;
}

void test_plugin_unload()
{
	puts("unloading plugin.");
}
