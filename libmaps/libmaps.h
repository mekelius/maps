#ifndef __LIBMAPS_HH
#define __LIBMAPS_HH

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

struct maps_Mut_String __to_Mut_String_Int(maps_Int i);
struct maps_Mut_String __to_Mut_String_Float(maps_Float f);

void free_Mut_String(struct maps_Mut_String* str);

struct maps_Mut_String concat_Mut_String_Mut_String(struct maps_Mut_String, struct maps_Mut_String);

#endif