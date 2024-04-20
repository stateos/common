/*******************************************************************************
@file     startup.h
@author   Rajmund Szymanski
@date     08.05.2023
@brief    Startup file header for gcc compiler.
*******************************************************************************/

#include <string.h>

#pragma once

/*******************************************************************************
 Additional definitions
*******************************************************************************/

#define __WEAK_ALIAS(function) __attribute__ ((weak, alias(#function)))
#define __VECTORS              __attribute__ ((used, section(".vectors")))
#define __CAST(sp)             (void(*)(void))(intptr_t)(sp)

/*******************************************************************************
 Prototypes of external functions
*******************************************************************************/

__WEAK __USED void hardware_init_hook        ( void );
__WEAK __USED void software_init_hook        ( void );
__WEAK __USED void initialise_monitor_handles( void );
__WEAK __USED void __libc_init_array         ( void );
__WEAK __USED void __libc_fini_array         ( void );
              void _exit                     (  int );
               int main                      ( void );

/*******************************************************************************
 Symbols defined in linker script
*******************************************************************************/

extern char __data_source[];
extern char __data_start [];
extern char __data_end   [];
extern char __bss_start  [];
extern char __bss_end    [];

/*******************************************************************************
 Default reset procedures
*******************************************************************************/

__STATIC_INLINE __NO_RETURN
void __main( void )
{
	int result;
	/* Early hardware init hook */
	hardware_init_hook();
	/* Initialize the data segment */
	memcpy(__data_start, __data_source, (size_t)(__data_end - __data_start));
	/* Zero fill the bss segment */
	memset(__bss_start, 0, (size_t)(__bss_end - __bss_start));
	/* Early software init hook */
	software_init_hook();
	/* Call global & static constructors */
	__libc_init_array();
	/* Initialize monitor handles */
	initialise_monitor_handles();
	/* Call the application's entry point */
	result = main();
	/* Call global & static destructors */
	__libc_fini_array();
	/* Go into an infinite loop */
	_exit(result);
}

/******************************************************************************/
