#include <iostream>
#include <string>

#include "common.h"

using namespace std;

void ast_error(std::string error_info)
{
    std::cerr << "[AST SEMANTIC ANALYSIS ERROR]";
    std::cerr << error_info << std::endl;
}

bool ends_with (std::string fullString, std::string ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}