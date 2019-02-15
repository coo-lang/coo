#include <iostream>
#include "codegen.h"
#include "ast.h"
#include "objgen.h"


extern NBlock* programBlock;
extern int yyparse();

int main(int argc, char **argv)
{
	yyparse();
	std::cout << programBlock << std::endl;

	CodeGenContext context;
	context.generateCode(*programBlock);
	context.runCode();
	// ObjGen(context);

	return 0;
}
