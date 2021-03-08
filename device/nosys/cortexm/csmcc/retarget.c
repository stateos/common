#include <stdio.h>

/* -------------------------------------------------------------------------- */

__weak
void *sbreak( int size )
{
	extern char  _memory[];
	extern char  _stack[];
	static char *_brk = _memory;
	       char * brk = NULL;

	if (_brk + size < _stack)
	{
		 brk  = _brk;
		_brk += size;
	}

	return brk;
}

/* -------------------------------------------------------------------------- */

#if !defined(__SEMIHOST) && !defined(__NOSYS)

/* -------------------------------------------------------------------------- */

__weak
int getchar( void )
{
	return (int) 0;
}

/* -------------------------------------------------------------------------- */

__weak
int putchar( char c )
{
	return (int) c;
}

/* -------------------------------------------------------------------------- */

#endif // !__SEMIHOST && !__NOSYS
