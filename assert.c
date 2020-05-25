#include "utils/assert.h"
#include <stdio.h>
#include <stdlib.h>

void
uassert_fail(const char   *restrict prefix,
             const char   *restrict expr,
             const char   *restrict file,
             unsigned int  line,
             const char   *restrict func)
{
	fflush(NULL);

	fprintf(stderr,
	        "[assert]%s:%s:%u:%s:\'%s\' failed\n",
	        prefix,
	        file,
	        line,
	        func,
	        expr);

	abort();
}
