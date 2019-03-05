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

Value* NBinaryOperator::codeGen(CodeGenContext& context) {
	cout << "Creating binary operation " << op << endl;
	Instruction::BinaryOps instr;
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
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TMINUS:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateSub(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFSub(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TMUL:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateMul(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFMul(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TDIV:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateSDiv(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFDiv(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TCEQ:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateICmpEQ(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOEQ(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TCNE:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateICmpNE(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpONE(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TCLT:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateICmpSLT(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOLT(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TCLE:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateICmpSLE(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOLE(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TCGT:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateICmpSGT(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOGT(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		case TCGE:
			if (getTypeString(left) == "i32" || getTypeString(left) == "i64")
				return Builder.CreateICmpSGE(left, right);
			if (getTypeString(left) == "double")
				return Builder.CreateFCmpOGE(left, right);
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
		default:
			ast_error("unsupport calculate for" + getTypeString(left));
			break;
	}
	return NULL;
}

Value* NBlock::codeGen(CodeGenContext& context) {
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		cout << "Generating code for " << typeid(**it).name() << endl;
		last = (**it).codeGen(context);
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
	return Builder.CreateStore(rightSide.codeGen(context), context.locals()[leftSide.name], false);
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
	// TheFunction->getBasicBlockList().push_back(ThenBB);
	Builder.SetInsertPoint(ThenBB);
	Value *ThenV = thenBlock.codeGen(context);
	if (!ThenV)
		return nullptr;
	Builder.CreateBr(MergeBB);
	// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
	ThenBB = Builder.GetInsertBlock();

	// Emit else block.
	// TheFunction->getBasicBlockList().push_back(ElseBB);
	Builder.SetInsertPoint(ElseBB);
	Value *ElseV = elseBlock.codeGen(context);
	if (!ElseV)
		return nullptr;
	Builder.CreateBr(MergeBB);
	// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
	ElseBB = Builder.GetInsertBlock();

	// Emit merge block.
	// TheFunction->getBasicBlockList().push_back(MergeBB);
	Builder.SetInsertPoint(MergeBB);
	// TODO: PHI FIX
	// PHINode *PN = Builder.CreatePHI(ThenV->getType(), 2, "iftmp");

	// PN->addIncoming(ThenV, ThenBB);
	// PN->addIncoming(ElseV, ElseBB);
	// return PN;

	return NULL;
}

Value* NForStatement::codeGen(CodeGenContext& context) {
	cout << "Generating for statement" << endl;

	// start
	start.codeGen(context);

	// body and step
	Function *TheFunction = Builder.GetInsertBlock()->getParent();
	BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
	Builder.CreateBr(LoopBB);
	Builder.SetInsertPoint(LoopBB);
	block.codeGen(context);
	step.codeGen(context);

	// endcond
	Value* endCond = end.codeGen(context);
	endCond = Builder.CreateICmpNE(endCond,
		ConstantInt::get(Type::getInt1Ty(TheContext), 0, true), "loopcond");

	// after
	BasicBlock *AfterBB =
      	BasicBlock::Create(TheContext, "afterloop", TheFunction);

	// br
	Builder.CreateCondBr(endCond, LoopBB, AfterBB);
	Builder.SetInsertPoint(AfterBB);

	return NULL;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context) {
	cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* NRet::codeGen(CodeGenContext& context) {
	cout << "Generating ret for " << typeid(expression).name() << endl;

	return Builder.CreateRet(expression.codeGen(context));
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context) {
	cout << "Creating variable declaration " << type.name << " " << id.name << endl;

	AllocaInst *alloc = Builder.CreateAlloca(typeOf(type), 0, NULL, id.name.c_str());
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

	auto *originBlock = Builder.GetInsertBlock();
	Builder.SetInsertPoint(bblock);
	context.pushBlock(bblock);

	it = arguments.begin();
	auto *arg = function->args().begin();
	for (; it != arguments.end() && arg != function->args().end(); it++, arg++) {
		// (**it).codeGen(context);
		AllocaInst *alloc = Builder.CreateAlloca(typeOf((**it).type), 0, NULL, (**it).id.name.c_str());
		context.locals()[(**it).id.name] = alloc;
		Builder.CreateStore(arg, alloc);
	}

	block.codeGen(context);
	// Builder.CreateRet(bblock);

	ReturnInst::Create(TheContext, Builder.GetInsertBlock());

	context.popBlock();
	Builder.SetInsertPoint(originBlock);
	cout << "Creating function: " << id.name << endl;
	return function;
}