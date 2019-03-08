#include <iostream>
#include <vector>
#include <string>
#include <llvm/IR/Value.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class Node {
public:
	virtual ~Node() { }
	virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
public:
	int value;
	NInteger(int value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NLong : public NExpression {
public:
	long long value;
	NLong(long long value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NDouble : public NExpression {
public:
	double value;
	NDouble(double value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBoolean : public NExpression {
public:
	bool value;
	NBoolean(bool value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NString : public NExpression {
public:
	std::string value;
	NString(std::string value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIdentifier : public NExpression {
public:
	std::string name;
	NIdentifier(const std::string& name) : name(name) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
public:
	const NIdentifier& id;
	ExpressionList arguments;
	NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	NMethodCall(const NIdentifier& id) : id(id) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NArrayIndex : public NExpression {
public:
	const NIdentifier& id;
	NExpression& index;
	NArrayIndex(const NIdentifier& id, NExpression& index) :
		id(id), index(index) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NUnaryOperator : public NExpression {
public:
	int op;
	NExpression& rightSide;
	NUnaryOperator(int op, NExpression& rightSide) :
		op(op), rightSide(rightSide) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator : public NExpression {
public:
	int op;
	NExpression& leftSide;
	NExpression& rightSide;
	NBinaryOperator(NExpression& leftSide, int op, NExpression& rightSide) :
		leftSide(leftSide), rightSide(rightSide), op(op) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NExpression {
public:
	StatementList statements;
	NBlock() { }
	NBlock(StatementList& statements) : statements(statements) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment : public NExpression {
public:
	const NIdentifier& leftSide;
	NExpression& rightSide;
	NAssignment(const NIdentifier& leftSide, NExpression& rightSide) :
		leftSide(leftSide), rightSide(rightSide) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIfStatement : public NStatement {
public:
	NExpression& condition;
	NBlock thenBlock;
	NBlock elseBlock;
	NIfStatement(NExpression& condition, NBlock thenBlock) :
		condition(condition), thenBlock(thenBlock) {}
	NIfStatement(NExpression& condition, NBlock thenBlock, NBlock elseBlock) :
		condition(condition), thenBlock(thenBlock), elseBlock(elseBlock) {}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NForStatement : public NStatement {
public:
	NExpression* start;
	NExpression* end;
	NExpression* step;
	NBlock block;
	NForStatement(NExpression* start, NExpression* end, NExpression* step, NBlock block) :
		start(start), end(end), step(step), block(block) {}
	NForStatement(NExpression* end, NExpression* step, NBlock block) :
		end(end), step(step), block(block) {}
	NForStatement(NExpression* end, NBlock block) :
		end(end), block(block) {}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExpressionStatement : public NStatement {
public:
	NExpression& expression;
	NExpressionStatement(NExpression& expression) : expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NRet : public NStatement {
public:
	NExpression& expression;
	NRet(NExpression& expression) : expression(expression) {}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration : public NStatement {
public:
	const NIdentifier& type;
	NIdentifier& id;
	NExpression *assignmentExpr;
	int arraySize;
	ExpressionList arrayValue;
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id) : type(type), id(id), arraySize(0) { }
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id, int arraySize) : type(type), id(id), arraySize(arraySize) { }
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id, int arraySize, ExpressionList arrayValue)
		: type(type), id(id), arraySize(arraySize), arrayValue(arrayValue) { }
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id, NExpression *assignmentExpr) :
		type(type), id(id), assignmentExpr(assignmentExpr), arraySize(0) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NFunctionDeclaration : public NStatement {
public:
	const NIdentifier& type;
	const NIdentifier& id;
	VariableList arguments;
	NBlock& block;
	NFunctionDeclaration(const NIdentifier& type, const NIdentifier& id, VariableList& arguments,
		NBlock& block) : type(type), id(id), arguments(arguments), block(block) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};