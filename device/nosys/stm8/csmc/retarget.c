#include <stdio.h>

/* -------------------------------------------------------------------------- */

void *sbreak( int size )
{
	extern char  _startmem[];
	extern char  _endmem[];
	static char *_brk = _startmem;
	       char * brk = NULL;

	if (_brk + size < _endmem)
	{
		 brk  = _brk;
		_brk += size;
	}

	return brk;
}

/* -------------------------------------------------------------------------- */
