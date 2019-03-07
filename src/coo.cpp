#include <iostream>
#include <fstream>
#include "codegen.h"
#include "ast.h"
#include "objgen.h"


extern NBlock* programBlock;
extern int yyparse();

int main(int argc, char **argv)
{
	/**
	 * parse command line args
	*/
	std::string inFile = "";
	std::string outFile = "";
	if( argc == 3 ) {
		inFile = argv[1];
		outFile = argv[2];
	}
	else {
		std::cout << "Usage: ./coo [source_code_file_name] [target_file_name]\n";
		return 1;
	}

	// read source code
	freopen(inFile.c_str(), "r", stdin);

	// compiler front-end parse
	yyparse();

	// compiler back-end parse
	CodeGenContext context = CodeGenContext(inFile);
	context.generateCode(*programBlock);

	// redirect stdout to file
	std::cout << inFile << " compiling to llvm ir file: " << outFile + ".ll" << std::endl;
	FILE *fp = freopen((outFile + ".ll").c_str(),"w",stdout);
	context.module->print(outs(), nullptr);

	// context.runCode();
	ObjGen(context, outFile + ".o");
	fflush(fp); freopen("/dev/tty", "w", stdout);
	std::cout << "Object code wrote to " << outFile << ".o" << std::endl;

	return 0;
}
