#include "pch.h"
#include "evaluator.h"

mcf::Object::Pointer mcf::Evaluator::Object::Eval(_Notnull_ const mcf::AST::Node::Interface* node) const noexcept
{
	DebugAssert(node != nullptr, "node가 nullptr이면 안됩니다.");

	mcf::Object::Pointer object;
	switch (node->GetNodeType())
	{
	case mcf::AST::Node::Type::EXPRESSION:
		DebugMessage("구현 필요");
		break;

	case mcf::AST::Node::Type::INTERMEDIATE:
		DebugMessage("구현 필요");
		break;

	case mcf::AST::Node::Type::STATEMENT:
		object = EvalStatement(static_cast<const mcf::AST::Statement::Interface*>(node));
		break;

	case mcf::AST::Node::Type::PROGRAM:
		object = EvalProgram(static_cast<const mcf::AST::Program*>(node));
		break;

	default:
		DebugBreak("");
	}
	return std::move(object);
}
		
mcf::Object::Pointer mcf::Evaluator::Object::EvalProgram(_Notnull_ const mcf::AST::Program* program) const noexcept
{
	DebugAssert(program != nullptr, "program가 nullptr이면 안됩니다.");

	mcf::Object::PointerVector objects;
	const size_t statementCount = program->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		objects.emplace_back(EvalStatement(program->GetUnsafeStatementPointerAt(i)));
	}
	return mcf::Object::Program::Make(std::move(objects));
}

mcf::Object::Pointer mcf::Evaluator::Object::EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement) const noexcept
{
	DebugAssert(statement != nullptr, "statement가 nullptr이면 안됩니다.");

	mcf::Object::Pointer object;
	constexpr const size_t STATEMENT_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (statement->GetStatementType())
	{
	case AST::Statement::Type::INCLUDE_LIBRARY: __COUNTER__;
		object = mcf::Object::IncludeLib::Make(static_cast<const mcf::AST::Statement::IncludeLibrary*>(statement)->GetLibPath());
		break;

	case AST::Statement::Type::TYPEDEF: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Statement::Type::EXTERN: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Statement::Type::LET: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Statement::Type::BLOCK: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Statement::Type::RETURN: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Statement::Type::FUNC: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Statement::Type::MAIN: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Statement::Type::EXPRESSION: __COUNTER__;
		object = EvalExpression(static_cast<const mcf::AST::Statement::Expression*>(statement)->GetUnsafeExpression());
		break;

	case AST::Statement::Type::UNUSED: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	default:
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. StatementType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Statement::CONVERT_TYPE_TO_STRING(statement->GetStatementType()), mcf::ENUM_INDEX(statement->GetStatementType()), statement->ConvertToString().c_str());
	}
	constexpr const size_t STATEMENT_TYPE_COUNT = __COUNTER__ - STATEMENT_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::AST::Statement::Type::COUNT) == STATEMENT_TYPE_COUNT, "statement type count is changed. this SWITCH need to be changed as well.");
	return std::move(object);
}

mcf::Object::Pointer mcf::Evaluator::Object::EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression) const noexcept
{
	DebugAssert(expression != nullptr, "expression가 nullptr이면 안됩니다.");
	mcf::Object::Pointer object;
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case AST::Expression::Type::IDENTIFIER: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::INTEGER: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::STRING: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::PREFIX: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::GROUP: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::INFIX: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::CALL: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::INDEX: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::INITIALIZER: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	case AST::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		DebugMessage("구현 필요");
		break;

	default:
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->ConvertToString().c_str());
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::AST::Statement::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");
	return std::move(object);
}
