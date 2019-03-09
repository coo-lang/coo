#ifndef COOCOMPILER_TS_H
#define COOCOMPILER_TS_H

std::string getTypeString(llvm::Value* value);
std::string getTypeString(llvm::Type* type);
NIdentifier typeInferring(llvm::Value* value);

#endif