#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

/* -------------------------------------------------------------------------- */

__attribute__((used, weak))
caddr_t _sbrk_r( struct _reent *reent, size_t size )
{
	extern char __heap_start[];
	extern char __heap_end[];
	static char * heap = __heap_start;
	       char * base;
	      (void)  reent;

	if (heap + size <= __heap_end)
	{
		base  = heap;
		heap += size;
	}
	else
	{
		errno = ENOMEM;
		base  = (caddr_t) -1;
	}

	return base;
}

/* -------------------------------------------------------------------------- */

#if !defined(__SEMIHOST) && !defined(__NOSYS)

/* -------------------------------------------------------------------------- */

__attribute__((weak))
int _open_r( struct _reent *reent, const char *path, int flags, int mode )
{
	(void) reent;
	(void) path;
	(void) flags;
	(void) mode;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _close_r( struct _reent *reent, int file )
{
	(void) reent;
	(void) file;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _lseek_r( struct _reent *reent, int file, int pos, int whence )
{
	(void) reent;
	(void) file;
	(void) pos;
	(void) whence;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _read_r( struct _reent *reent, int file, char *buf, size_t size )
{
	(void) reent;
	(void) file;
	(void) buf;
	(void) size;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _write_r( struct _reent *reent, int file, char *buf, size_t size )
{
	(void) reent;
	(void) file;
	(void) buf;
	(void) size;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _isatty_r( struct _reent *reent, int file )
{
	(void) reent;
	(void) file;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _fstat_r( struct _reent *reent, int file, struct stat *st )
{
	(void) reent;
	(void) file;
	(void) st;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _getpid_r( struct _reent *reent )
{
	(void) reent;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

__attribute__((weak))
int _kill_r( struct _reent *reent, int pid, int sig )
{
	(void) reent;
	(void) pid;
	(void) sig;
	errno = ENOSYS; // procedure not implemented
	return -1;
}

/* -------------------------------------------------------------------------- */

#endif // !__SEMIHOST && !__NOSYS

/* -------------------------------------------------------------------------- */

__attribute__((weak, noreturn))
void __assert_func(const char* const file, const int line, const char* const func, const char* const expr)
{
	printf("\nassert error at %s:%d:%s:%s\n", file, line, expr, func);
	abort();
}

/* -------------------------------------------------------------------------- */

__attribute__((weak, noreturn))
void __cxa_pure_virtual()
{
	abort();
}

/* -------------------------------------------------------------------------- */
