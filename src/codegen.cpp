#include "ast.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

/* Compile AST into a module*/
void CodeGenContext::generateCode(NBlock& root) {
	cout << "Generating code...\n";

	/* Create top level interpreter function to call as entry*/
	vector<Type*> argTypes;
	FunctionType *ftype = FunctionType::get(Type::getVoidTy(TheContext), makeArrayRef(argTypes), false);
	mainFunction = Function::Create(ftype, GlobalValue::ExternalLinkage, "main", module);
	BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", mainFunction, 0);

	/* Push a new variable/block context */
	pushBlock(bblock);
	root.codeGen(*this); /* Emit bytecode for toplevel block*/
	ReturnInst::Create(TheContext, bblock);
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

Value* NIdentifier::codeGen(CodeGenContext& context) {
	cout << "Creating identifier reference: " << name << endl;
	if (context.locals().find(name) == context.locals().end()) {
		cerr << "undeclared variable " << name << endl;
		return NULL;
	}
	return new LoadInst(context.locals()[name], "", false, context.currentBlock());
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
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());

	cout << "Creating method call: " << id.name << endl;
	return call;

}

Value* NBinaryOperator::codeGen(CodeGenContext& context) {
	cout << "Creating binary operation " << op << endl;
	Instruction::BinaryOps instr;
	switch (op) {
		case TPLUS:
			instr = Instruction::Add;
			goto math;
		case TMINUS:
			instr = Instruction::Sub;
			goto math;
		case TMUL:
			instr = Instruction::Mul;
			goto math;
		case TDIV:
			instr = Instruction::SDiv;
			goto math;
		/* TODO comparison*/
	}
	return NULL;
math:
	return BinaryOperator::Create(instr, leftSide.codeGen(context), rightSide.codeGen(context), "",
			context.currentBlock());
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
	return new StoreInst(rightSide.codeGen(context), context.locals()[leftSide.name],
		false, context.currentBlock());
}

Value* NExpressionStatement::codeGen(CodeGenContext& context) {
	cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context) {
	cout << "Creating variable declaration " << type.name << " " << id.name << endl;

	AllocaInst *alloc = new AllocaInst(typeOf(type), 0, id.name.c_str(), context.currentBlock());
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
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
	BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", function, 0);

	context.pushBlock(bblock);

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).codeGen(context);
	}

	block.codeGen(context);
	ReturnInst::Create(TheContext, bblock);

	context.popBlock();
	cout << "Creating function: " << id.name << endl;
	return function;
}