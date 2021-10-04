#include "utils/assert.h"
#include <stdio.h>
#include <stdlib.h>

void
uassert_fail(const char   * __restrict prefix,
             const char   * __restrict expr,
             const char   * __restrict file,
             unsigned int              line,
             const char   * __restrict func)
{
	fflush(NULL);

	fprintf(stderr,
	        "[  ASSERT] %s:%s:%u:%s:\'%s\' failed\n",
	        prefix,
	        file,
	        line,
	        func,
	        expr);

	abort();
}
