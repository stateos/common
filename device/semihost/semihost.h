#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

// ARM Semihosting Commands

#define SYS_OPEN        (0x01)
#define SYS_CLOSE       (0x02)
#define SYS_WRITEC      (0x03)
#define SYS_WRITE0      (0x04)
#define SYS_WRITE       (0x05)
#define SYS_READ        (0x06)
#define SYS_READC       (0x07)
#define SYS_ISERROR     (0x08)
#define SYS_ISTTY       (0x09)
#define SYS_SEEK        (0x0A)
#define SYS_ENSURE      (0x0B)
#define SYS_FLEN        (0x0C)
#define SYS_TMPNAM      (0x0D)
#define SYS_REMOVE      (0x0E)
#define SYS_RENAME      (0x0F)
#define SYS_CLOCK       (0x10)
#define SYS_TIME        (0x11)
#define SYS_SYSTEM      (0x12)
#define SYS_ERRNO       (0x13)
#define SYS_GET_CMDLINE (0x15)
#define SYS_HEAPINFO    (0x16)
#define SYS_EXIT        (0x18)
#define SYS_ELAPSED     (0x30)
#define SYS_TICKFREQ    (0x31)

/* -------------------------------------------------------------------------- */

// Hardware vector reason codes
#define ADP_Stopped_BranchThroughZero    (void*)0x20000
#define ADP_Stopped_UndefinedInstr       (void*)0x20001
#define ADP_Stopped_SoftwareInterrupt    (void*)0x20002
#define ADP_Stopped_PrefetchAbort        (void*)0x20003
#define ADP_Stopped_DataAbort            (void*)0x20004
#define ADP_Stopped_AddressException     (void*)0x20005
#define ADP_Stopped_IRQ                  (void*)0x20006
#define ADP_Stopped_FIQ                  (void*)0x20007

// Software reason codes
#define ADP_Stopped_BreakPoint           (void*)0x20020
#define ADP_Stopped_WatchPoint           (void*)0x20021
#define ADP_Stopped_StepComplete         (void*)0x20022
#define ADP_Stopped_RunTimeErrorUnknown  (void*)0x20023
#define ADP_Stopped_InternalError        (void*)0x20024
#define ADP_Stopped_UserInterruption     (void*)0x20025
#define ADP_Stopped_ApplicationExit      (void*)0x20026
#define ADP_Stopped_StackOverflow        (void*)0x20027
#define ADP_Stopped_DivisionByZero       (void*)0x20028
#define ADP_Stopped_OSSpecific           (void*)0x20029

/* -------------------------------------------------------------------------- */

#if defined(__ICCARM__) || defined(__GNUC__) && !defined(__ARMCC_VERSION)

static inline __attribute__ (( always_inline ))
int __semihost( int cmd, const void *arg )
{
	__asm volatile
	(
		"bkpt 0xAB"
		: "+r"(cmd)                                 /* output operands             */
		:  "r"(arg)                                 /* input operands              */
		:  "r2", "r3", "r12", "lr", "cc", "memory"  /* list of clobbered registers */
	);

	return cmd;
}

#endif

/* -------------------------------------------------------------------------- */

#if defined(__CSMC__)

int __semihost( int cmd, const void *arg );

#endif

/* -------------------------------------------------------------------------- */

int semihost_open   ( const char *name, int openmode );
int semihost_close  ( int fh );
int semihost_writec ( int c );
int semihost_write0 ( const char *buffer );
int semihost_write  ( int fh, const char *buffer, int length );
int semihost_read   ( int fh,       char *buffer, int length );
int semihost_readc  ( void );
int semihost_iserror( int status );
int semihost_istty  ( int fh);
int semihost_seek   ( int fh, long position );
int semihost_ensure ( int fh );
int semihost_flen   ( int fh );
int semihost_tmpnam ( char *buffer, int fh, int length );
int semihost_remove ( const char *name );
int semihost_rename ( const char *old_name, const char *new_name );
int semihost_cmdline( char *buffer, int length );
int semihost_exit   ( void );

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif
