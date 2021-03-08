#include <stdio.h>

#if !defined(__SEMIHOST) && !defined(__NOSYS)

/* Read from a file. -------------------------------------------------------- */

__attribute__((weak))
size_t __read( int file, char *buf, size_t size )
{
	(void) file;
	(void) buf;
	(void) size;

	return 0;
}

/* Write to a file. --------------------------------------------------------- */

__attribute__((weak))
size_t __write( int file, const char *buf, size_t size )
{
	if (file == -1) return -1U; // flush

	(void) buf;

	return size;
}

/* -------------------------------------------------------------------------- */

#endif // !__SEMIHOST && !__NOSYS
