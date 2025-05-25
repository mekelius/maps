#ifndef __LIBMAPS_HH
#define __LIBMAPS_HH

#include "maps_datatypes.h"

void print_Boolean(maps_Boolean b);
void print_Int(maps_Int i);
void print_String(maps_String str);
void print_Float(maps_Float d);

// compile time casts signal failure with return value
bool CT_to_Int_String(maps_String str, maps_Int* out);
bool CT_to_Float_String(maps_String str, maps_Float* out);

// runtime casts aren't allowed to fail
maps_Float to_Float_Int(maps_Int i);
maps_String to_String_Boolean(maps_Boolean b);

struct maps_MutString to_MutString_Int(maps_Int i);
struct maps_MutString to_MutString_Float(maps_Float f);
maps_String to_String_MutString(struct maps_MutString);

void free_MutString(struct maps_MutString* str);

struct maps_MutString concat_MutString_MutString(struct maps_MutString, struct maps_MutString);

#endif