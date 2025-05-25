#include "libmaps.h"

#include <stdio.h>
#include <assert.h>

#include <errno.h>
#include <stdlib.h>

const size_t MAX_INT_LENGTH = 12;

void print_Boolean(maps_Boolean b) {
    printf("%s", b ? "true" : "false");
}

void print_Int(maps_Int i) {
    printf("%i", i);
}

void print_String(maps_String str) {
    printf("%s", str);
}

void print_Float(maps_Float d) {
    printf("%f", d);
}

bool __CT_to_Int_String(maps_String str, maps_Int* out) {
    errno = 0;
    char* end = NULL;
    maps_Int result = strtol(str, &end, 10);

    if (*end)
        return false;

    if (errno != 0 && result == 0)
        return false;

    *out = result;
    return true;
}

bool __CT_to_Float_String(maps_String str, maps_Float* out) {
    errno = 0;
    char* end = NULL;
    maps_Float result = strtod(str, &end);

    if (*end)
        return false;

    if (errno != 0 && result == 0)
        return false;

    *out = result;
    return true;
}

// runtime casts aren't allowed to fail
maps_Float __to_Float_Int(maps_Int i) {
    return (maps_Float) i;
}

maps_String __to_String_Boolean(maps_Boolean b) {
    return b ? "true" : "false";
}

struct maps_Mut_String __to_Mut_String_Int(maps_Int i) {
    struct maps_Mut_String str;
    str.data = malloc(MAX_INT_LENGTH);
    str.length = snprintf(str.data, MAX_INT_LENGTH, "%i", i);

    return str;
}

struct maps_Mut_String __to_Mut_String_Float(maps_Float f) {
    assert(false && "not implemented");
}

void free_Mut_String(struct maps_Mut_String* str) {
    str->length = 0;
    free(str->data);
}
