#include "maps_builtins.h"

#include <stdio.h>

void print_Boolean(maps_Boolean b) {
    printf("%s", b ? "true" : "false");
}

void print_Int(maps_Int i) {
    printf("%i", i);
}

void print_String(maps_const_String str) {
    printf("%s", str);
}

void print_Float(maps_Float d) {
    printf("%f", d);
}