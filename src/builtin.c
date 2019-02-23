#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void print(char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    puts("");
}