/*	C99 STANDARD INTEGER HEADER
 *	Copyright (c) 2012 by COSMIC Software
 */
#ifndef __STDINT__
#define __STDINT__	1

/*	standard types
 */
typedef signed char int8_t;
typedef signed short int16_t;
typedef	signed long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

/*	minimum types
 */
typedef signed char int_least8_t;
typedef signed short int_least16_t;
typedef	signed long int_least32_t;
typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned long uint_least32_t;

/*	fastest types
 */
typedef signed char int_fast8_t;
typedef signed short int_fast16_t;
typedef	signed long int_fast32_t;
typedef unsigned char uint_fast8_t;
typedef unsigned short uint_fast16_t;
typedef unsigned long uint_fast32_t;

/*	pointer types
 */
typedef signed short intptr_t;
typedef unsigned short uintptr_t;

/*	greatest types
 */
typedef signed long intmax_t;
typedef unsigned long uintmax_t;

/*	limit macros
 */
#define INT8_MIN	(-128)
#define INT16_MIN	(-32768)
#define INT32_MIN	(-2147483648)
#define INT8_MAX	  127
#define INT16_MAX	  32767
#define INT32_MAX	  2147483647
#define UINT8_MAX	  255
#define UINT16_MAX	  0xffff
#define UINT32_MAX	  0xffffffff

/*	minimum limit macros
 */
#define INT_LEAST8_MIN	(-128)
#define INT_LEAST16_MIN	(-32768)
#define INT_LEAST32_MIN	(-2147483648)
#define INT_LEAST8_MAX	  127
#define INT_LEAST16_MAX	  32767
#define INT_LEAST32_MAX	  2147483647
#define UINT_LEAST8_MAX	  255
#define UINT_LEAST16_MAX  0xffff
#define UINT_LEAST32_MAX  0xffffffff

/*	fastest limit macros
 */
#define INT_FAST8_MIN	(-128)
#define INT_FAST16_MIN	(-32768)
#define INT_FAST32_MIN	(-2147483648)
#define INT_FAST8_MAX	  127
#define INT_FAST16_MAX	  32767
#define INT_FAST32_MAX	  2147483647
#define UINT_FAST8_MAX	  255
#define UINT_FAST16_MAX	  0xffff
#define UINT_FAST32_MAX	  0xffffffff

/*	pointer limit macros
 */
#define INTPTR_MIN	(-32768)
#define INTPTR_MAX	  32767
#define UINTPTR_MAX	  0xffff
#define PTRDIFF_MIN	(-32768)
#define PTRDIFF_MAX	  32767

/*	greatest limit macros
 */
#define INTMAX_MIN	(-32768)
#define INTMAX_MAX	  32767
#define UINTMAX_MAX	  0xffff

/*	integer constants macros
 */
#define INT8_C(n)	((int8_t)n)
#define INT16_C(n)	((int16_t)n)
#define INT32_C(n)	(n ## L)

#define UINT8_C(n)	((uint8_t)n)
#define UINT16_C(n)	((uint16_t)n)
#define UINT32_C(n)	(n ## UL)

#define INTMAX_C(n)	(n ## L)
#define UINTMAX_C(n)	(n ## UL)

#endif

