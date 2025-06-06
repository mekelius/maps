#include "libmaps.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <errno.h>
#include <stdlib.h>

const size_t MAX_INT_LENGTH = 12;

void print_i_Void_Boolean(maps_Boolean b) {
    printf("%s", b ? "true" : "false");
}

void print_i_Void_Int(maps_Int i) {
    printf("%i", i);
}

void prints_i_Void_String(maps_String str) {
    printf("%s", str);
}

void printms_i_Void_MutString(struct maps_MutString* str) {
    prints_i_Void_String(to_String_MutString(str));
}

void print_i_Void_Float(maps_Float d) {
    printf("%f", d);
}

bool CT_to_Int_String(maps_String str, maps_Int* out) {
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

bool CT_to_Float_String(maps_String str, maps_Float* out) {
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
maps_Float to_Float_Int(maps_Int i) {
    return (maps_Float) i;
}

maps_String to_String_Boolean(maps_Boolean b) {
    return b ? "true" : "false";
}

maps_String to_String_MutString(struct maps_MutString* str) {
    return str->data;
}

struct maps_MutString* to_MutString_Int(maps_Int i) {
    struct maps_MutString* out = malloc(sizeof(struct maps_MutString));

    // Leave space for the null terminator
    out->data = malloc(MAX_INT_LENGTH + 1);
    out->mem_size = snprintf(out->data, MAX_INT_LENGTH, "%i", i) + 1;
    out->length = out->mem_size - 1;

    assert(out->mem_size >= 0 && "to_MutString_Int failed");

    return out;
}

struct maps_MutString* to_MutString_Float(maps_Float f) {
    assert(false && "not implemented");
}

void free_MutString(struct maps_MutString* str) {
    str->mem_size = 0;
    free(str->data);
}

struct maps_MutString* concat(struct maps_MutString* lhs, struct maps_MutString* rhs) {

    struct maps_MutString* out = malloc(sizeof(struct maps_MutString));

    out->mem_size = lhs->mem_size + rhs->mem_size - 1;
    out->length = lhs->length + rhs->length;
    out->data = malloc(out->mem_size);

    memcpy(out->data, lhs->data, lhs->mem_size - 1);
    memcpy(out->data + lhs->mem_size - 1, rhs->data, rhs->mem_size);

    return out;
}
