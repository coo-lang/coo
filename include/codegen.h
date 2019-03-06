#include <stack>
#include <typeinfo>
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

class NBlock;
static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);

class CodeGenBlock {
public:
	BasicBlock *block;
	Value* returnValue;
	std::map<std::string, Value*> locals;
};

class CodeGenContext {
	std::stack<CodeGenBlock *> blocks;
	Function *mainFunction;

public:
	Module *module;
	CodeGenContext(std::string sourceFileName) {
		module = new Module(sourceFileName, TheContext);
		register_printf(module);
	}

	void register_printf(llvm::Module *module) {
		std::vector<llvm::Type*> printf_arg_types;
		printf_arg_types.push_back(llvm::Type::getInt8PtrTy(module->getContext()));

		llvm::FunctionType* printf_type =
			llvm::FunctionType::get(
				llvm::Type::getVoidTy(module->getContext()), printf_arg_types, true);

		llvm::Function *func = llvm::Function::Create(
					printf_type, llvm::Function::ExternalLinkage,
					llvm::Twine("print"),
					module
			);
		func->setCallingConv(llvm::CallingConv::C);
	}

	void generateCode(NBlock& root);
	GenericValue runCode();
	std::map<std::string, Value*>& locals() { return blocks.top()->locals; }
	CodeGenBlock* currentBlock() { return blocks.top(); }
	void pushBlock(BasicBlock *block) { blocks.push(new CodeGenBlock()); blocks.top()->block = block; }
	void popBlock() { CodeGenBlock *top = blocks.top(); blocks.pop(); delete top; }
};