#include <iostream>
#include "codegen.h"
#include "ast.h"

using namespace std;

extern NBlock* programBlock;
extern int yyparse();

int main(int argc, char **argv)
{
	yyparse();
	cout << programBlock << endl;

	CodeGenContext context;
	context.generateCode(*programBlock);
	context.runCode();

	return 0;
}
