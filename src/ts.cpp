
#include <iostream>
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include "ts.h"

using namespace llvm;

std::string getTypeString(Value* value) {
	std::string type_str;
	llvm::raw_string_ostream rso(type_str);
	value->getType()->print(rso);
	rso.flush();
    return type_str;
}

std::string getTypeString(Type* type) {
	std::string type_str;
	llvm::raw_string_ostream rso(type_str);
	type->print(rso);
	rso.flush();
    return type_str;
}