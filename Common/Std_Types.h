/**
 * Std_Types.h
 *
 *  Created on: 4/29/2026
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

typedef unsigned char         uint8;
typedef unsigned short        uint16;
typedef unsigned int          uint32;
typedef unsigned long long    uint64;

typedef signed char           sint8;
typedef signed short          sint16;
typedef signed int            sint32;
typedef signed long long      sint64;

typedef float                 float32;
typedef double                float64;

typedef unsigned int          uintptr_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif /* STD_TYPES_H */
