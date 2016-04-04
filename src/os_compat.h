#ifndef OS_COMPAT_H
#define OS_COMPAT_H
#include <stdio.h>
#include "stm32fxxx_hal.h"

#define ATTR_NO_RETURN   __attribute__ ((noreturn))
#define ATTR_UNUSED   __attribute__ ((unused))
#define ATTR_WARN_UNUSED_RESULT   __attribute__ ((warn_unused_result))
#define ATTR_NON_NULL_PTR_ARG(...)   __attribute__ ((nonnull (__VA_ARGS__)))
#define ATTR_NON_NULL   __attribute__ ((nonnull))
#define ATTR_NAKED   __attribute__ ((naked))
#define ATTR_NO_INLINE   __attribute__ ((noinline))
#define ATTR_ALWAYS_INLINE   __attribute__ ((always_inline))
#define ATTR_PURE   __attribute__ ((pure))
#define ATTR_CONST   __attribute__ ((const))
#define ATTR_DEPRECATED   __attribute__ ((deprecated))
#define ATTR_WEAK   __attribute__ ((weak))
#define ATTR_NO_INIT   __attribute__ ((section (".noinit")))
#define ATTR_INIT_SECTION(SectionIndex)   __attribute__ ((used, naked, section (".init" #SectionIndex )))
#define ATTR_ALIAS(Func)   __attribute__ ((alias( #Func )))
#define ATTR_PACKED   __attribute__ ((packed))
#define ATTR_ALIGNED(Bytes)   __attribute__ ((aligned(Bytes)))
#define ATTR(attrs)	__attribute__(attrs)

#define ASSERT(condition)					do { } while( 0 );
#ifndef NULL
#define NULL	0
#endif // NULL

#define _D(MSG)					printf MSG

typedef uint32_t			u32;
typedef uint8_t				u8;
typedef u8					BOOLEAN;

#endif /* OS_COMPAT_H */
