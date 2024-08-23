#include "pch.h"
#include "evaluator.h"

mcf::IR::Pointer mcf::Evaluator::Object::Eval(_Notnull_ const mcf::AST::Node::Interface* node, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	DebugAssert(node != nullptr, u8"node가 nullptr이면 안됩니다.");

	mcf::IR::Pointer object;
	switch (node->GetNodeType())
	{
	case mcf::AST::Node::Type::EXPRESSION:
		DebugMessage(u8"구현 필요");
		break;

	case mcf::AST::Node::Type::INTERMEDIATE:
		DebugMessage(u8"구현 필요");
		break;

	case mcf::AST::Node::Type::STATEMENT:
		object = EvalStatement(static_cast<const mcf::AST::Statement::Interface*>(node), scope);
		break;

	case mcf::AST::Node::Type::PROGRAM:
		object = EvalProgram(static_cast<const mcf::AST::Program*>(node), scope);
		break;

	default:
		DebugBreak("");
	}
	return std::move(object);
}
		
mcf::IR::Pointer mcf::Evaluator::Object::EvalProgram(_Notnull_ const mcf::AST::Program* program, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	DebugAssert(program != nullptr, u8"program가 nullptr이면 안됩니다.");

	mcf::IR::PointerVector objects;
	const size_t statementCount = program->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		objects.emplace_back(EvalStatement(program->GetUnsafeStatementPointerAt(i), scope));
	}
	return mcf::IR::Program::Make(std::move(objects));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	DebugAssert(statement != nullptr, u8"statement가 nullptr이면 안됩니다.");

	mcf::IR::Pointer object = mcf::IR::Invalid::Make();
	constexpr const size_t STATEMENT_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (statement->GetStatementType())
	{
	case AST::Statement::Type::INCLUDE_LIBRARY: __COUNTER__;
		object = mcf::IR::IncludeLib::Make(static_cast<const mcf::AST::Statement::IncludeLibrary*>(statement)->GetLibPath());
		break;

	case AST::Statement::Type::TYPEDEF: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::EXTERN: __COUNTER__;
		object = EvalExternStatement(static_cast<const mcf::AST::Statement::Extern*>(statement), scope);
		break;

	case AST::Statement::Type::LET: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::BLOCK: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::RETURN: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::FUNC: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::MAIN: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::EXPRESSION: __COUNTER__;
	{
		mcf::IR::Expression::Pointer expressionObject = EvalExpression(static_cast<const mcf::AST::Statement::Expression*>(statement)->GetUnsafeExpression(), scope);
		if (expressionObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
		{
			break;
		}
		object = std::move(expressionObject);
		break;
	}

	case AST::Statement::Type::UNUSED: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	default:
		object = mcf::IR::Invalid::Make();
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. StatementType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Statement::CONVERT_TYPE_TO_STRING(statement->GetStatementType()), mcf::ENUM_INDEX(statement->GetStatementType()), statement->ConvertToString().c_str());
	}
	constexpr const size_t STATEMENT_TYPE_COUNT = __COUNTER__ - STATEMENT_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::AST::Statement::Type::COUNT) == STATEMENT_TYPE_COUNT, "statement type count is changed. this SWITCH need to be changed as well.");
	return std::move(object);
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	const mcf::AST::Intermediate::FunctionSignature* signature = statement->GetUnsafeSignaturePointer();
	const mcf::Object::FunctionInfo functionInfo = EvalFunctionSignatureIntermediate(signature, scope);
	if (functionInfo.IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	const size_t paramCount = functionInfo.Params.Variables.size();
	std::vector<mcf::Object::TypeInfo> params;
	for (size_t i = 0; i < paramCount; ++i)
	{
		DebugAssert(functionInfo.Params.Variables[i].IsValid(), u8"");
		DebugAssert(functionInfo.Params.Variables[i].DataType.IsValid(), u8"");
		DebugAssert(scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");

		params.emplace_back(functionInfo.Params.Variables[i].DataType);
	}

	DebugAssert(functionInfo.Name.empty() == false, u8"");
	if (scope->DefineFunction(functionInfo.Name, functionInfo) == false )
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}
	return mcf::IR::Extern::Make(functionInfo.Name, params, (functionInfo.Params.VariadicIdentifier.empty() == false));
}

mcf::Object::FunctionInfo mcf::Evaluator::Object::EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::Object::FunctionInfo functionInfo;
	functionInfo.Name = intermediate->GetName();
	if (functionInfo.Name.empty())
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	const bool isParamsValid = EvalFunctionParamsIntermediate(functionInfo.Params, intermediate->GetUnsafeFunctionParamsPointer(), scope);
	if (isParamsValid == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	functionInfo.ReturnType = EvalTypeSignatureIntermediate(intermediate->GetUnsafeReturnTypePointer(), scope);
	if (functionInfo.ReturnType.IsValid() == false || scope->FindTypeInfo(functionInfo.ReturnType.Name).IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	return functionInfo;
}

const bool mcf::Evaluator::Object::EvalFunctionParamsIntermediate(_Out_ mcf::Object::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	outParams = mcf::Object::FunctionParams();
	const size_t size = intermediate->GetParamCount();
	for (size_t i = 0; i < size; ++i)
	{
		outParams.Variables.emplace_back(EvalVariavbleSignatureIntermediate(intermediate->GetUnsafeParamPointerAt(i), scope));
		if (outParams.Variables.back().IsValid() == false )
		{
			DebugMessage(u8"구현 필요");
			return false;
		}
	}
	outParams.VariadicIdentifier = intermediate->HasVariadic() ? intermediate->GetUnsafeVariadic()->GetIdentifier() : std::string();
	return true;
}

mcf::Object::Variable mcf::Evaluator::Object::EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::Object::Variable variable;
	variable.Name = intermediate->GetName();
	if (variable.Name.empty())
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::Variable();
	}

	variable.DataType = EvalTypeSignatureIntermediate(intermediate->GetUnsafeTypeSignaturePointer(), scope);
	if (variable.DataType.IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::Variable();
	}

	return variable;
}

mcf::Object::TypeInfo mcf::Evaluator::Object::EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::IR::Pointer object = EvalExpression(intermediate->GetUnsafeSignaturePointer(), scope);
	DebugAssert(object->GetType() == mcf::IR::Type::EXPRESSION, u8"");
	mcf::IR::Expression::Interface* expressionObject = static_cast<mcf::IR::Expression::Interface*>(object.get());
	mcf::Object::TypeInfo typeInfo;
	switch (expressionObject->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER:
		typeInfo = static_cast<mcf::IR::Expression::TypeIdentifier*>(expressionObject)->GetInfo();
		break;

	default:
		DebugBreak(u8"");
	}

	if (intermediate->IsUnsigned() && typeInfo.IsUnsigned)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::TypeInfo();
	}
	typeInfo.IsUnsigned = intermediate->IsUnsigned() ? intermediate->IsUnsigned() : typeInfo.IsUnsigned;
	return typeInfo;
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	DebugAssert(expression != nullptr, u8"expression가 nullptr이면 안됩니다.");
	mcf::IR::Expression::Pointer object = mcf::IR::Expression::Invalid::Make();
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case AST::Expression::Type::IDENTIFIER: __COUNTER__;
		object = EvalIdentifierExpression(static_cast<const mcf::AST::Expression::Identifier*>(expression), scope);
		break;

	case AST::Expression::Type::INTEGER: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Expression::Type::STRING: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Expression::Type::PREFIX: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Expression::Type::GROUP: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Expression::Type::INFIX: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Expression::Type::CALL: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Expression::Type::INDEX: __COUNTER__;
		object = EvalIndexExpression(static_cast<const mcf::AST::Expression::Index*>(expression), scope);
		break;

	case AST::Expression::Type::INITIALIZER: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	default:
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->ConvertToString().c_str());
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::AST::Statement::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");
	return std::move(object);
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	const std::string name = expression->GetTokenLiteral();
	if (scope->IsIdentifierRegistered(name) == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Expression::Invalid::Make();
	}

	const mcf::Object::TypeInfo typeInfo = scope->FindTypeInfo(name);
	if (typeInfo.IsValid() == true)
	{
		return mcf::IR::Expression::TypeIdentifier::Make(typeInfo);
	}

	const mcf::Object::VariableInfo variableInfo = scope->FindVariableInfo(name);
	if ( variableInfo.IsValid() == true)
	{
		if (variableInfo.IsGlobal)
		{
			return mcf::IR::Expression::GlobalVariableIdentifier::Make(variableInfo.Variable);
		}
		else
		{
			return mcf::IR::Expression::LocalVariableIdentifier::Make(variableInfo.Variable);
		}
	}

	const mcf::Object::FunctionInfo functionInfo = scope->FindFunction(name);
	if (functionInfo.IsValid() == true)
	{
		return mcf::IR::Expression::FunctionIdentifier::Make(functionInfo);
	}

	DebugMessage(u8"구현 필요");
	return mcf::IR::Expression::Pointer();
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIndexExpression(_Notnull_ const mcf::AST::Expression::Index* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	UNUSED(expression, scope);



	mcf::IR::Expression::Pointer expressionObject = EvalExpression( static_cast<const mcf::AST::Statement::Expression*>(statement)->GetUnsafeExpression(), scope );
	if ( expressionObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID )
	{
		break;
	}
	object = std::move( expressionObject );


	DebugMessage(u8"구현 필요");
	return mcf::IR::Expression::Pointer();
}
