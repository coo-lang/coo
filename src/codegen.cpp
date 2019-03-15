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
	BasicBlock *retblock = BasicBlock::Create(TheContext, "retBlock", mainFunction, 0);

	/* Push a new variable/block context */
	Builder.SetInsertPoint(bblock);
	pushBlock(bblock);
	currentBlock()->returnBlock = retblock;
	currentBlock()->returnValue = Builder.CreateAlloca(Type::getInt32Ty(TheContext), 0, NULL, "");
	root.codeGen(*this); /* Emit bytecode for toplevel block*/

	// ret part
	Builder.CreateBr(retblock);
	Builder.SetInsertPoint(retblock);
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
static Type *typeOf(NIdentifier type) {
	if (type.name.compare("int") == 0) {
		return Type::getInt32Ty(TheContext);
	} else if (type.name.compare("[]int") == 0) {
		return Type::getInt32PtrTy(TheContext);
	} else if (type.name.compare("long") == 0) {
		return Type::getInt64Ty(TheContext);
	} else if (type.name.compare("[]long") == 0) {
		return Type::getInt64PtrTy(TheContext);
	} else if (type.name.compare("float") == 0) {
		return Type::getDoubleTy(TheContext);
	} else if (type.name.compare("[]float") == 0) {
		return Type::getDoublePtrTy(TheContext);
	} else if (type.name.compare("string") == 0) {
		return Type::getInt8PtrTy(TheContext);
	} else if (type.name.compare("[]string") == 0) {
		return Type::getInt8PtrTy(TheContext)->getPointerTo();
	} else if (type.name.compare("bool") == 0) {
		return Type::getInt1Ty(TheContext);
	} else if (type.name.compare("[]bool") == 0) {
		return Type::getInt1PtrTy(TheContext);
	} else if (type.name.compare("void") == 0) {
		return Type::getVoidTy(TheContext);
	}

	return Type::getVoidTy(TheContext);
}

static Type *typeOf(NIdentifier type, NIdentifier funcType, IdentifierList funcParams) {
	if (type.name.compare("func") == 0) {
		std::vector<Type*> argTypes;
		for (auto it = funcParams.begin(); it != funcParams.end(); it++) {
			cout << "function parameter argument: " << (**it).name << endl;
			/* fix: recursion function type (need a scalable type system) */
			argTypes.push_back(typeOf(**it));
		}
		FunctionType *ftype = FunctionType::get(typeOf(funcType), makeArrayRef(argTypes), false);
		return ftype->getPointerTo();
	}

	if (type.name.compare("int") == 0) {
		return Type::getInt32Ty(TheContext);
	} else if (type.name.compare("[]int") == 0) {
		return Type::getInt32PtrTy(TheContext);
	} else if (type.name.compare("long") == 0) {
		return Type::getInt64Ty(TheContext);
	} else if (type.name.compare("[]long") == 0) {
		return Type::getInt64PtrTy(TheContext);
	} else if (type.name.compare("float") == 0) {
		return Type::getDoubleTy(TheContext);
	} else if (type.name.compare("[]float") == 0) {
		return Type::getDoublePtrTy(TheContext);
	} else if (type.name.compare("string") == 0) {
		return Type::getInt8PtrTy(TheContext);
	} else if (type.name.compare("[]string") == 0) {
		return Type::getInt8PtrTy(TheContext)->getPointerTo();
	} else if (type.name.compare("bool") == 0) {
		return Type::getInt1Ty(TheContext);
	} else if (type.name.compare("[]bool") == 0) {
		return Type::getInt1PtrTy(TheContext);
	} else if (type.name.compare("void") == 0) {
		return Type::getVoidTy(TheContext);
	}

	return Type::getVoidTy(TheContext);
}

static Value* getArrayIndex(Value* array,  Value* index) {
	std::vector<Value*> indices;

	cout << "array type is: " << getTypeString(array) << endl;
	if (ends_with(getTypeString(array), "**")) {
		array = Builder.CreateLoad(array);
	} else {
		indices.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 0, false));
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
	if (functionAlias.find(name) != functionAlias.end()) {
		name = functionAlias[name];
	}

	cout << "this identifier type: " << getTypeString(context.locals()[name]) << endl;
	if (context.module->getFunction(name.c_str())) {
		return context.locals()[name];
	}

	if (index) {
		return Builder.CreateLoad(getArrayIndex(context.locals()[name], index->codeGen(context)), "");
	} else if (((AllocaInst *)context.locals()[name])->isArrayAllocation()) {
		return getArrayIndex(context.locals()[name], ConstantInt::get(Type::getInt64Ty(TheContext), 0, true));
	}

	return Builder.CreateLoad(context.locals()[name], "");
}

Value* NMethodCall::codeGen(CodeGenContext& context) {
	Function *function = context.module->getFunction(id.name.c_str());
	if (function == NULL) {
		if (context.locals().find(id.name) == context.locals().end()) {
			cerr << "no such function " << id.name << endl;
			return NULL;
		}
		Value* function1 = Builder.CreateLoad(context.locals()[id.name]);
		std::vector<Value*> args;
		ExpressionList::const_iterator it;
		for (it = arguments.begin(); it != arguments.end(); it++) {
			args.push_back((**it).codeGen(context));
		}
		/* Effectively call the method*/
		CallInst *call = Builder.CreateCall(function1, makeArrayRef(args));
		cout << "Creating method call: " << id.name << endl;
		return call;
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
		if (typeid(**it).name() == "4NRet")  {
			Builder.CreateBr(context.currentBlock()->returnBlock);
			break;
		}
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

	Value* val = rightSide.codeGen(context);
	if (context.module->getFunction(val->getName())) {
		return context.locals()[val->getName()];
	}

	if (leftSide.index && context.locals()[leftSide.name]->getType()->isPtrOrPtrVectorTy()) {
		return Builder.CreateStore(val, getArrayIndex(context.locals()[leftSide.name], leftSide.index->codeGen(context)), false);
	} else {
		return Builder.CreateStore(val, context.locals()[leftSide.name], false);
	}
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
	// Builder.CreateBr(MergeBB);
	if (Builder.GetInsertBlock()->getTerminator() == NULL) {
		Builder.CreateBr(MergeBB);
	}
	// ThenBB = Builder.GetInsertBlock();

	// Emit else block.
	Builder.SetInsertPoint(ElseBB);
	elseBlock.codeGen(context);
	// Builder.CreateBr(MergeBB);
	if (Builder.GetInsertBlock()->getTerminator() == NULL) {
		Builder.CreateBr(MergeBB);
	}
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
	if (varDecl)
		varDecl->codeGen(context);

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
	auto ty = typeOf(type);
	if (arraySize > 0) {
		// array type
		Value* arraySizeValue = NInteger(arraySize).codeGen(context);
		auto arrayType = ArrayType::get(typeOf(type), arraySize);
		alloc = Builder.CreateAlloca(arrayType, arraySizeValue, id.name.c_str());

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
		if (type.name == "func") {
			// function type
			if (assignmentExpr == NULL) {
				ast_error("right value should be declare explicitly if func");
				return NULL;
			}
			Value* val = assignmentExpr->codeGen(context);
			alloc = new AllocaInst(val->getType(), 0, id.name.c_str(), (Instruction *)context.currentBlock()->returnValue);
 			functionAlias[id.name] = val->getName().str();
		} else {
			// primitive
			Value* val = nullptr;
			if (assignmentExpr == NULL) {
				if (type.name == "") {
					ast_error("cannot define variable without type declaration");
					return NULL;
				}
			} else {
				val = assignmentExpr->codeGen(context);
				// type inferring
				if (type.name != "" && getTypeString(val) != getTypeString(ty)) {
					ast_error("cannot cast " + getTypeString(val) + " to " + getTypeString(ty) + " !");
					return NULL;
				}
				// /* todo: better solution but need time to refactor*/
				ty = val->getType();
			}

			alloc = new AllocaInst(ty, 0, id.name.c_str(), (Instruction *)context.currentBlock()->returnValue);
			if (val)
				Builder.CreateStore(val, alloc, false);
		}
	}

	context.locals()[id.name] = alloc;
	return alloc;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context) {
	cout << "Generating function statement" << endl;
	std::vector<Type*> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		cout << "function argument: " << (**it).type.name << endl;
		argTypes.push_back(typeOf((**it).type, (**it).funcType, (**it).funcParams));
	}
	FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
	Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
	context.locals()[id.name] = function;

	BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", function);
	BasicBlock *retblock = BasicBlock::Create(TheContext, "retBlock", function);

	// store context before function
	auto *originBlock = Builder.GetInsertBlock();
	Builder.SetInsertPoint(bblock);
	context.pushBlock(bblock);
	context.currentBlock()->returnBlock = retblock;
	// return value initialize
	if (typeOf(type)->isVoidTy()) {
		context.currentBlock()->returnValue = Builder.CreateAlloca(Type::getInt32Ty(TheContext), 0, NULL, "");
	} else {
		context.currentBlock()->returnValue = Builder.CreateAlloca(typeOf(type), 0, NULL, "");
	}

	// arguments initialize
	it = arguments.begin();
	auto *arg = function->args().begin();
	for (; it != arguments.end() && arg != function->args().end(); it++, arg++) {
		AllocaInst *alloc = Builder.CreateAlloca(typeOf((**it).type, (**it).funcType, (**it).funcParams), 0, NULL, (**it).id.name.c_str());
		context.locals()[(**it).id.name] = alloc;
		Builder.CreateStore(arg, alloc);
	}

	// block generate
	block.codeGen(context);

	// return value
	if (Builder.GetInsertBlock()->getTerminator() == NULL) {
		Builder.CreateBr(retblock);
	}
	Builder.SetInsertPoint(retblock);
	if (typeOf(type)->isVoidTy()) {
		Builder.CreateRetVoid();
	} else {
		Builder.CreateRet(Builder.CreateLoad(context.currentBlock()->returnValue));
	}

	// restore context after function
	context.popBlock();
	Builder.SetInsertPoint(originBlock);
	cout << "Creating function: " << id.name << endl;
	return function;
}