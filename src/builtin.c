#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void println(char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    puts("");
}