clang -o builtin.o -c builtin.c
./coo test/examples/printf.coo test/output/printf.ll
clang -o print builtin.o output.o
./print