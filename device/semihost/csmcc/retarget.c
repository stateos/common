#include <stdio.h>
#include "semihost.h"

#if !defined(__SEMIHOST) && !defined(__NOSYS)

/* -------------------------------------------------------------------------- */

int __semihost( int cmd, const void *arg )
{
	return _asm("bkpt #0xAB", cmd, arg);
}

/* -------------------------------------------------------------------------- */

int getchar( void )
{
	return semihost_readc();
}

/* -------------------------------------------------------------------------- */

int putchar( char c )
{
	return semihost_writec(c);
}

/* -------------------------------------------------------------------------- */

#endif // !__SEMIHOST && !__NOSYS
