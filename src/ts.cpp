#include <iostream>
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include "ast.h"
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

NIdentifier typeInferring(Value* value) {
	std::string t = getTypeString(value);

	if (t == "i32") {
		return NIdentifier("int");
	} else if (t == "i64") {
		return NIdentifier("long");
	} else if (t == "double") {
		return NIdentifier("float");;
	} else if (t == "i8*") {
		return NIdentifier("string");
	} else if (t == "i1") {
		return NIdentifier("bool");
	}

	return NIdentifier("void");
}