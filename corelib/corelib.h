#ifndef __MAPS_BUILTINS_H
#define __MAPS_BUILTINS_H

#include "maps_datatypes.h"

void print_Boolean(maps_Boolean b);
void print_Int(maps_Int i);
void print_String(maps_String str);
void print_Float(maps_Float d);

// compile time casts signal failure with return value
bool __CT_to_Int_String(maps_String str, maps_Int* out);
bool __CT_to_Float_String(maps_String str, maps_Float* out);

// runtime casts aren't allowed to fail
maps_Float __to_Float_Int(maps_Int i);
maps_String __to_String_Boolean(maps_Boolean b);

// These need mutable strings
// maps_String* __Int_to_String(maps_Int i);
// maps_String* __Float_to_String(maps_Float f);

#endif