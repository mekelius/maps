#include "corelib.h"

#include <stdio.h>

#include <errno.h>
#include <stdlib.h>

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

bool const_String_to_Int(maps_String str, maps_Int* out) {
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

bool const_String_to_Float(maps_String str, maps_Float* out) {
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
