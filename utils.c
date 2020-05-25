#include <stdio.h>
#include <stdlib.h>

void
utils_assert_fail(const char   *prefix,
                  const char   *expr,
                  const char   *file,
                  unsigned int  line,
                  const char   *func)
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
