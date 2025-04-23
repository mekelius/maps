#include "maps_builtins.h"

#include <stdio.h>

void print_Boolean(bool b) {
    puts(b ? "true" : "false");
}

void print_Int(int i) {
    printf("%i", i);
}

void print_String(const char* str) {
    puts(str);
}

void print_Float(double d) {
    printf("%f", d);
}

// TODO: how to print structs etc.