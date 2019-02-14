all: parser

clean:
	rm parser.cpp parser.hpp parser scanner.cpp

parser.cpp: parser.y
	bison -d -o $@ $^
	
parser.hpp: parser.cpp

scanner.cpp: scanner.l parser.hpp
	flex -o $@ $^

parser: parser.cpp scanner.cpp coo.cpp codegen.cpp
	clang++ -o $@ $^ `llvm-config --cxxflags --ldflags --system-libs --libs all`