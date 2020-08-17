#include <unistd.h>

unsigned int usys_pgsz;

static void __attribute__((constructor))
usys_setup_page_size(void)
{
	usys_pgsz = (unsigned int)sysconf(_SC_PAGESIZE);
}
