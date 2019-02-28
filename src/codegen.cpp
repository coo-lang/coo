#include "ast.h"
#include "codegen.h"
#include "parser.hpp"
#include "ts.h"

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
		return Type::getVoidTy(TheContext);
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

	cout << getTypeString(left) << endl;
	cout << getTypeString(right) << endl;
	// std::string type_str;
	// llvm::raw_string_ostream rso(type_str);
	// left->getType()->print(rso);
	// rso.flush();
	// cout << type_str << "==" << endl;
	// printf("left type is ");
	// (*(*left).getType()).print(outs());
	// printf("left type is ");
	// printf("\n");
	// printf("right type is ");
	// (*(*right).getType()).print(outs());
	// printf("\n");

	// if ()

	switch (op) {
		case TPLUS:
			return Builder.CreateAdd(leftSide.codeGen(context), rightSide.codeGen(context));
			instr = Instruction::Add;
			goto math;
		case TMINUS:
			return Builder.CreateSub(leftSide.codeGen(context), rightSide.codeGen(context));
			instr = Instruction::Sub;
			goto math;
		case TMUL:
			return Builder.CreateMul(leftSide.codeGen(context), rightSide.codeGen(context));
			instr = Instruction::Mul;
			goto math;
		case TDIV:
			return Builder.CreateSDiv(leftSide.codeGen(context), rightSide.codeGen(context));
			instr = Instruction::SDiv;
			goto math;
		/* TODO comparison*/
		/* TODO default */
	}
	return NULL;
math:
	;
	// TODO
	// return BinaryOperator::Create(instr, leftSide.codeGen(context), rightSide.codeGen(context), "",
	// 		context.currentBlock());
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

Value* NExpressionStatement::codeGen(CodeGenContext& context) {
	cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
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

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).codeGen(context);
	}

	block.codeGen(context);
	// Builder.CreateRet(bblock);

	ReturnInst::Create(TheContext, bblock);

	context.popBlock();
	Builder.SetInsertPoint(originBlock);
	cout << "Creating function: " << id.name << endl;
	return function;
}