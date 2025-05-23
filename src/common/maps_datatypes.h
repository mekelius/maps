#ifndef __MAPS_DATA_TYPES_H
#define __MAPS_DATA_TYPES_H

/**
 * This file contains common definitions for types used by maps.
 * There isn't anything special here, Int is 32sbit for compatibility, bools are 8 bits.
 * LLVM uses 1-bit bools, but for interop it's better to stick to 8.
 * At some point this will be expanded to handle other needed types and representations.
 */

#include <stdbool.h>
#include <stdint.h>

// ----- INTEGER TYPES -----

typedef int32_t maps_Int;
typedef bool maps_Boolean;

// typedef char maps_Char_INTERN;

// these should be included at some point
// typedef int32_t maps_Int32;
// typedef int64_t maps_Int64;


// ----- FLOAT TYPE -----

#ifndef __STDC_IEC_559__
#error "64 bit floating point required"
#endif
// c++ version
// static_assert(std::numeric_limits<double>::is_iec559, "IEEE 754 floating point");
typedef double maps_Float;


// ----- STRING TYPE -----

// null terminated constant strings
typedef const char* maps_String;

#endif