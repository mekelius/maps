#ifndef __MAPS_BUILTINS_H
#define __MAPS_BUILTINS_H

#include "maps_datatypes.h"

void print_Boolean(maps_Boolean b);
void print_Int(maps_Int i);
void print_String(maps_String str);
void print_Float(maps_Float d);

bool const_String_to_Int(maps_String str, maps_Int* out);
bool const_String_to_Float(maps_String str, maps_Float* out);

#endif