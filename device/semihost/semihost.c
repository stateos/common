#include <stdio.h>
#include <string.h>
#include "semihost.h"

#if !defined(__ARMCC_VERSION)
#if !defined(__SEMIHOST) && !defined(__NOSYS)

/* -------------------------------------------------------------------------- */

int semihost_open( const char *name, int openmode )
{
	int args[3];

	args[0] = (int)name;
	args[1] = (int)openmode;
	args[2] = (int)strlen(name);

	return __semihost(SYS_OPEN, args);
}

/* -------------------------------------------------------------------------- */

int semihost_close( int fh )
{
	return __semihost(SYS_CLOSE, &fh);
}

/* -------------------------------------------------------------------------- */

int semihost_writec( int c )
{
	return __semihost(SYS_WRITEC, &c);
}

/* -------------------------------------------------------------------------- */

int semihost_write0( const char *buffer )
{
	return __semihost(SYS_WRITE0, buffer);
}

/* -------------------------------------------------------------------------- */

int semihost_write( int fh, const char *buffer, int length )
{
	if (length == 0) return 0;

	int args[3];

	args[0] = (int)fh;
	args[1] = (int)buffer;
	args[2] = (int)length;

	return __semihost(SYS_WRITE, args);
}

/* -------------------------------------------------------------------------- */

int semihost_read( int fh, char *buffer, int length )
{
	int args[3];

	args[0] = (int)fh;
	args[1] = (int)buffer;
	args[2] = (int)length;

	return __semihost(SYS_READ, args);
}

/* -------------------------------------------------------------------------- */

int semihost_readc( void )
{
	return __semihost(SYS_READC, 0);
}

/* -------------------------------------------------------------------------- */

int semihost_iserror( int status )
{
	return __semihost(SYS_ISERROR, &status);
}

/* -------------------------------------------------------------------------- */

int semihost_istty( int fh )
{
	return __semihost(SYS_ISTTY, &fh);
}

/* -------------------------------------------------------------------------- */

int semihost_seek( int fh, long position )
{
	int args[2];

	args[0] = (int)fh;
	args[1] = (int)position;

	return __semihost(SYS_SEEK, args);
}

/* -------------------------------------------------------------------------- */

int semihost_ensure( int fh )
{
	return __semihost(SYS_ENSURE, &fh);
}

/* -------------------------------------------------------------------------- */

int semihost_flen( int fh )
{
	return __semihost(SYS_FLEN, &fh);
}

/* -------------------------------------------------------------------------- */

int semihost_tmpnam ( char *buffer, int fh, int length )
{
	int args[3];

	args[0] = (int)buffer;
	args[1] = (int)fh;
	args[2] = (int)length;

	return __semihost(SYS_TMPNAM, args);
}

/* -------------------------------------------------------------------------- */

int semihost_remove( const char *name )
{
	int args[2];

	args[0] = (int)name;
	args[1] = (int)strlen(name);

	return __semihost(SYS_REMOVE, args);
}

/* -------------------------------------------------------------------------- */

int semihost_rename( const char *old_name, const char *new_name )
{
	int args[4];

	args[0] = (int)old_name;
	args[1] = (int)strlen(old_name);
	args[2] = (int)new_name;
	args[3] = (int)strlen(new_name);

	return __semihost(SYS_RENAME, args);
}

/* -------------------------------------------------------------------------- */

int semihost_cmdline( char* buffer, int length )
{
	int args[2];

	args[0] = (int)buffer;
	args[1] = (int)length;

	return __semihost(SYS_GET_CMDLINE, args);
}

/* -------------------------------------------------------------------------- */

int semihost_exit( void )
{
	return __semihost(SYS_EXIT, ADP_Stopped_ApplicationExit);
}

/* -------------------------------------------------------------------------- */

#endif // !__SEMIHOST && !__NOSYS
#endif // !__ARMCC_VERSION
