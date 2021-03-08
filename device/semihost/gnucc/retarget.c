#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "semihost.h"

#if !defined(__SEMIHOST) && !defined(__NOSYS)

/* Open a file. ------------------------------------------------------------- */

int _open_r( struct _reent *reent, const char *path, int flags, int mode )
{
	(void) reent;
	(void) flags;

	return semihost_open(path, mode);
}

/* Close a file. ------------------------------------------------------------ */

int _close_r( struct _reent *reent, int file )
{
	(void) reent;

	return semihost_close(file);
}

/* Set position in a file. -------------------------------------------------- */

int _lseek_r( struct _reent *reent, int file, int pos, int whence )
{
	(void) reent;
	(void) whence;

	return semihost_seek(file, pos);
}

/* Read from a file. -------------------------------------------------------- */

int _read_r( struct _reent *reent, int file, char *buf, size_t size )
{
	(void) reent;

	int length = semihost_read(file, buf, (int)size);

	return length ? length : (int)size;
}

/* Write to a file. --------------------------------------------------------- */

int _write_r( struct _reent *reent, int file, char *buf, size_t size )
{
	(void) reent;

	int length = semihost_write(file, buf, (int)size);

	return length ? length : (int)size;
}

/* Query whether output stream is a terminal. ------------------------------- */

int _isatty_r( struct _reent *reent, int file )
{
	(void) reent;

	return semihost_istty(file);
}

/* -------------------------------------------------------------------------- */

#endif // !__SEMIHOST && !__NOSYS
