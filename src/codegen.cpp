#include "ast.h"
#include "codegen.h"
#include "parser.hpp"
#include "ts.h"
#include "common.h"

using namespace std;

/* Compile AST into a module*/
void CodeGenContext::generateCode(NBlock& root) {
	cout << "Generating code...\n";

	/* Create top level interpreter function to call as entry*/
	vector<Type*> argTypes;
	FunctionType *ftype = FunctionType::get(Type::getInt32Ty(TheContext), makeArrayRef(argTypes), false);
	mainFunction = Function::Create(ftype, GlobalValue::ExternalLinkage, "main", module);
	BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", mainFunction, 0);

	/* Push a new variable/block context */
	Builder.SetInsertPoint(bblock);
	pushBlock(bblock);	// todo: need remove stack data structure
	root.codeGen(*this); /* Emit bytecode for toplevel block*/
	// Builder.CreateRetVoid();
	Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(TheContext), 0, true));
	popBlock();

	cout << "Code is generated.\n";
	/*Print the bytecode*/
	// legacy::PassManager pm;
	// pm.add(createPrintModulePass(outs()));
	// pm.run(*module);
}

/* Executes program main function*/
GenericValue CodeGenContext::runCode() {
	cout << "Running code...\n";
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	ExecutionEngine *ee = EngineBuilder(unique_ptr<Module>(module)).create();

	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
  	// ee->freeMachineCodeForFunction(mainFunction);
  	delete ee;
  	llvm_shutdown();
	cout << "Code was run.\n";
	return v;
}

/* Returns a LLVM type based on the identifier */
static Type *typeOf(const NIdentifier type) {
	if (type.name.compare("int") == 0) {
		return Type::getInt32Ty(TheContext);
	} else if (type.name.compare("long") == 0) {
		return Type::getInt64Ty(TheContext);
	} else if (type.name.compare("float") == 0) {
		return Type::getDoubleTy(TheContext);
	} else if (type.name.compare("string") == 0) {
		return Type::getInt8PtrTy(TheContext);
	} else if (type.name.compare("bool") == 0) {
		return Type::getInt1Ty(TheContext);
	}
	return Type::getVoidTy(TheContext);
}

static Value *getArrayIndex(Value* array,  Value* index) {
	std::vector<Value*> indices;

	cout << "array type is: " << getTypeString(array) << endl;
	if (getTypeString(array) != "i8**") {
		indices.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 0, false));
	} else {
		array = Builder.CreateLoad(array);
	}
	indices.push_back(index);

	return Builder.CreateInBoundsGEP(array, makeArrayRef(indices), "");
}

/* Code Generation */
Value* NInteger::codeGen(CodeGenContext& context) {
	cout << "Creating Integer: " << value << endl;
	return ConstantInt::get(Type::getInt32Ty(TheContext), value, true);
}

Value* NLong::codeGen(CodeGenContext& context) {
	cout << "Creating Integer: " << value << endl;
	return ConstantInt::get(Type::getInt64Ty(TheContext), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context) {
	cout << "Creating Double: " << value << endl;
	return ConstantFP::get(Type::getDoubleTy(TheContext), value);
}

Value* NBoolean::codeGen(CodeGenContext& context) {
	cout << "Create Boolean: " << value << endl;
	return ConstantInt::get(Type::getInt1Ty(TheContext), value, false);
}

Value* NString::codeGen(CodeGenContext& context) {
	cout << "Create String: " << value << endl;
	return Builder.CreateGlobalStringPtr(StringRef(value.c_str()));
}

Value* NIdentifier::codeGen(CodeGenContext& context) {
	cout << "Creating identifier reference: " << name << endl;
	if (context.locals().find(name) == context.locals().end()) {
		cerr << "undeclared variable " << name << endl;
		return NULL;
	}

	if (index != NULL) {
		return Builder.CreateLoad(getArrayIndex(context.locals()[name], index->codeGen(context)), "");
	}

	return Builder.CreateLoad(context.locals()[name], "");
}

Value* NMethodCall::codeGen(CodeGenContext& context) {
	Function *function = context.module->getFunction(id.name.c_str());
	if (function == NULL) {
		cerr << "no such function " << id.name << endl;
		// return status error ??
	}
	/* Execute expressions in arguments */
	std::vector<Value*> args;
	ExpressionList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		args.push_back((**it).codeGen(context));
	}
	/* Effectively call the method*/
	CallInst *call = Builder.CreateCall(function, makeArrayRef(args));

	cout << "Creating method call: " << id.name << endl;
	return call;
}

Value* NUnaryOperator::codeGen(CodeGenContext& context) {
	cout << "Creating unary operation " << op << endl;
	Value* right = rightSide.codeGen(context);

	switch (op) {
		case TMINUS:
			if (getTypeString(right) == "i32")
				return Builder.CreateSub(ConstantInt::get(Type::getInt32Ty(TheContext), 0, true), right);
			if (getTypeString(right) == "i64")
				return Builder.CreateSub(ConstantInt::get(Type::getInt64Ty(TheContext), 0, true), right);
			if (getTypeString(right) == "double")
				return Builder.CreateFSub(ConstantFP::get(Type::getDoubleTy(TheContext), 0.0), right);
			ast_error("unsupport calculate for " + getTypeString(right));
			break;
		default:
			ast_error("unsupport calculate for calculator: " + op);
			break;
	}

	return NULL;
}

Value* NBinaryOperator::codeGen(CodeGenContext& context) {
	cout << "Creating binary operation " << op << endl;
	Value* left = leftSide.codeGen(context);
	Value* right = rightSide.codeGen(context);

	if (getTypeString(left) != getTypeString(right)) {
		cerr << "[ERROR]variables type aren't equal: left is "
			<< getTypeString(left) << ", right is " << getTypeString(right) << endl;
		return NULL;
	}

	switch (op) {
		case TPLUS:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateAdd(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFAdd(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TMINUS:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateSub(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFSub(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TMUL:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateMul(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFMul(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TDIV:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateSDiv(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFDiv(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TCEQ:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64" || getTypeString(left) == "i1")
				return Builder.CreateICmpEQ(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOEQ(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TCNE:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64" || getTypeString(left) == "i1")
				return Builder.CreateICmpNE(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpONE(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TCLT:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64" || getTypeString(left) == "i1")
				return Builder.CreateICmpSLT(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOLT(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TCLE:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64" || getTypeString(left) == "i1")
				return Builder.CreateICmpSLE(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOLE(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TCGT:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64" || getTypeString(left) == "i1")
				return Builder.CreateICmpSGT(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOGT(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		case TCGE:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64" || getTypeString(left) == "i1")
				return Builder.CreateICmpSGE(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOGE(left, right);
			ast_error("unsupport calculate for " + getTypeString(left));
			break;
		default:
			ast_error("unsupport calculate for calculator: " + op);
			break;
	}
	return NULL;
}

Value* NBlock::codeGen(CodeGenContext& context) {
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		cout << "Generating code for ===== " << typeid(**it).name() << endl;
		last = (**it).codeGen(context);
		// break block generating if ret statement
		if (typeid(**it).name() == "4NRet") break;
	}
	cout << "Creating block" << endl;
	return last;
}

Value* NAssignment::codeGen(CodeGenContext& context) {
	cout << "Creating assignment for " << leftSide.name << endl;
	if (context.locals().find(leftSide.name) == context.locals().end()) {
		cerr << "undeclared variable " << leftSide.name << endl;
		return NULL;
	}

	if (leftSide.index && context.locals()[leftSide.name]->getType()->isPtrOrPtrVectorTy()) {
		return Builder.CreateStore(rightSide.codeGen(context), getArrayIndex(context.locals()[leftSide.name], leftSide.index->codeGen(context)), false);
	} else {
		return Builder.CreateStore(rightSide.codeGen(context), context.locals()[leftSide.name], false);
	}

	return NULL;
}

Value* NIfStatement::codeGen(CodeGenContext& context) {
	cout << "Generating if statement" << endl;

	Value* condV = Builder.CreateICmpNE(condition.codeGen(context), ConstantInt::get(Type::getInt1Ty(TheContext), 0, true), "ifcond");

	Function *TheFunction = Builder.GetInsertBlock()->getParent();
	BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
  	BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else", TheFunction);
  	BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont", TheFunction);

	Builder.CreateCondBr(condV, ThenBB, ElseBB);

	// Emit then value.
	Builder.SetInsertPoint(ThenBB);
	thenBlock.codeGen(context);
	Builder.CreateBr(MergeBB);
	// ThenBB = Builder.GetInsertBlock();

	// Emit else block.
	Builder.SetInsertPoint(ElseBB);
	elseBlock.codeGen(context);
	Builder.CreateBr(MergeBB);
	// ElseBB = Builder.GetInsertBlock();

	// Emit merge block.
	Builder.SetInsertPoint(MergeBB);

	return NULL;
}

Value* NForStatement::codeGen(CodeGenContext& context) {
	cout << "Generating for statement" << endl;

	// start
	if (start)
		start->codeGen(context);

	// body and after block
	Function *TheFunction = Builder.GetInsertBlock()->getParent();
	BasicBlock *endCondBB = BasicBlock::Create(TheContext, "endcondBB", TheFunction);
	BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loopBB", TheFunction);
	BasicBlock *AfterBB = BasicBlock::Create(TheContext, "afterloopBB", TheFunction);

	Builder.CreateBr(endCondBB);
	// endcond and conditional br
	Builder.SetInsertPoint(endCondBB);
	Value* endCond = end->codeGen(context);
	endCond = Builder.CreateICmpNE(endCond,
		ConstantInt::get(Type::getInt1Ty(TheContext), 0, true), "endcond");
	Builder.CreateCondBr(endCond, LoopBB, AfterBB);

	// body and step generate
	Builder.SetInsertPoint(LoopBB);
	block.codeGen(context);
	if (step)
		step->codeGen(context);
	Builder.CreateBr(endCondBB);

	// after loop
	Builder.SetInsertPoint(AfterBB);

	return NULL;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context) {
	cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* NRet::codeGen(CodeGenContext& context) {
	cout << "Generating ret for " << typeid(expression).name() << endl;

	Builder.CreateStore(expression.codeGen(context), context.currentBlock()->returnValue);

	return NULL;
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context) {
	cout << "Creating variable declaration " << type.name << " " << id.name << endl;

	AllocaInst *alloc;
	if (arraySize > 0) {
		Value* arraySizeValue = NInteger(arraySize).codeGen(context);
		auto arrayType = ArrayType::get(typeOf(type), arraySize);
		alloc = Builder.CreateAlloca(arrayType, arraySizeValue);

		// array value initializing
		std::vector<Value*> values;
		ExpressionList::const_iterator it;
		for (int i = 0; i < arrayValue.size(); i++) {
			std::vector<Value*> indices;
			indices.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 0, true));
			indices.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), i, true));
			auto idx = Builder.CreateInBoundsGEP(alloc, makeArrayRef(indices), "");

			Builder.CreateStore((*arrayValue[i]).codeGen(context), idx);
		}
	} else {
		alloc = Builder.CreateAlloca(typeOf(type), 0, NULL, id.name.c_str());
	}

	context.locals()[id.name] = alloc;
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		assn.codeGen(context);
	}
	return alloc;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context) {
	cout << "Generating function statement" << endl;
	std::vector<Type*> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		argTypes.push_back(typeOf((**it).type));
	}
	FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
	Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
	BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", function);

	// store context before function
	auto *originBlock = Builder.GetInsertBlock();
	Builder.SetInsertPoint(bblock);
	context.pushBlock(bblock);

	// return value initialize
	context.currentBlock()->returnValue = Builder.CreateAlloca(typeOf(type), 0, NULL, "");

	// arguments initialize
	it = arguments.begin();
	auto *arg = function->args().begin();
	for (; it != arguments.end() && arg != function->args().end(); it++, arg++) {
		AllocaInst *alloc = Builder.CreateAlloca(typeOf((**it).type), 0, NULL, (**it).id.name.c_str());
		context.locals()[(**it).id.name] = alloc;
		Builder.CreateStore(arg, alloc);
	}

	// block generate
	block.codeGen(context);

	// return value
	Builder.CreateRet(Builder.CreateLoad(context.currentBlock()->returnValue));

	// restore context after function
	context.popBlock();
	Builder.SetInsertPoint(originBlock);
	cout << "Creating function: " << id.name << endl;
	return function;
}