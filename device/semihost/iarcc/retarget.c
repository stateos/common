#include <stdio.h>
#include "semihost.h"

#if !defined(__SEMIHOST) && !defined(__NOSYS)

/* Read from a file. -------------------------------------------------------- */

size_t __read( int file, char *buf, size_t size )
{
	size_t length;

	length = (unsigned)semihost_read(file, buf, size);

	return length ? length : size;
}

/* Write to a file. --------------------------------------------------------- */

size_t __write( int file, const char *buf, size_t size )
{
	size_t length;

	if (file == -1) return -1U; // flush

	length = (unsigned)semihost_write(file, buf, size);

	return length ? length : size;
}

/* -------------------------------------------------------------------------- */

#endif // !__SEMIHOST && !__NOSYS
