#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "../../harbol.h"


int32_t plugin_load()
{
	puts("loading another plugin.");
	return 1;
}

void plugin_unload()
{
	puts("unloading another plugin.");
}
