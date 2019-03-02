#include <iostream>

#include "common.h"

using namespace std;

void ast_error(std::string error_info)
{
    cerr << "[AST SEMANTIC ANALYSIS ERROR]";
    cerr << error_info << endl;
}