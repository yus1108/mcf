#include "pch.h"
#include "evaluator.h"

mcf::Evaluator::Object::Object(const std::vector<TypeInfo>& primitiveTypes) noexcept
{
	const size_t size = primitiveTypes.size();
	for (size_t i = 0; i < size; ++i)
	{
		DefineType(primitiveTypes[i].Name, primitiveTypes[i]);
	}
}

mcf::Object::Pointer mcf::Evaluator::Object::Eval(_Notnull_ const mcf::AST::Node::Interface* node) noexcept
{
	DebugAssert(node != nullptr, u8"node가 nullptr이면 안됩니다.");

	mcf::Object::Pointer object;
	switch (node->GetNodeType())
	{
	case mcf::AST::Node::Type::EXPRESSION:
		DebugMessage(u8"구현 필요");
		break;

	case mcf::AST::Node::Type::INTERMEDIATE:
		DebugMessage(u8"구현 필요");
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
		
mcf::Object::Pointer mcf::Evaluator::Object::EvalProgram(_Notnull_ const mcf::AST::Program* program) noexcept
{
	DebugAssert(program != nullptr, u8"program가 nullptr이면 안됩니다.");

	mcf::Object::PointerVector objects;
	const size_t statementCount = program->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		objects.emplace_back(EvalStatement(program->GetUnsafeStatementPointerAt(i)));
	}
	return mcf::Object::Program::Make(std::move(objects));
}

mcf::Object::Pointer mcf::Evaluator::Object::EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement) noexcept
{
	DebugAssert(statement != nullptr, u8"statement가 nullptr이면 안됩니다.");

	mcf::Object::Pointer object = mcf::Object::Invalid::Make();
	constexpr const size_t STATEMENT_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (statement->GetStatementType())
	{
	case AST::Statement::Type::INCLUDE_LIBRARY: __COUNTER__;
		object = mcf::Object::IncludeLib::Make(static_cast<const mcf::AST::Statement::IncludeLibrary*>(statement)->GetLibPath());
		break;

	case AST::Statement::Type::TYPEDEF: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::EXTERN: __COUNTER__;
		object = EvalExternStatement(static_cast<const mcf::AST::Statement::Extern*>(statement));
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
		mcf::Object::Expression::Pointer expressionObject = EvalExpression(static_cast<const mcf::AST::Statement::Expression*>(statement)->GetUnsafeExpression());
		if (expressionObject->GetExpressionType() == mcf::Object::Expression::Type::INVALID)
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
		object = mcf::Object::Invalid::Make();
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. StatementType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Statement::CONVERT_TYPE_TO_STRING(statement->GetStatementType()), mcf::ENUM_INDEX(statement->GetStatementType()), statement->ConvertToString().c_str());
	}
	constexpr const size_t STATEMENT_TYPE_COUNT = __COUNTER__ - STATEMENT_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::AST::Statement::Type::COUNT) == STATEMENT_TYPE_COUNT, "statement type count is changed. this SWITCH need to be changed as well.");
	return std::move(object);
}

mcf::Object::Pointer mcf::Evaluator::Object::EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement) noexcept
{
	const mcf::AST::Intermediate::FunctionSignature* signature = statement->GetUnsafeSignaturePointer();
	const mcf::Evaluator::FunctionInfo functionInfo = EvalFunctionSignatureIntermediate(signature);
	if (functionInfo.IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::Invalid::Make();
	}

	const size_t paramCount = functionInfo.Params.Variables.size();
	std::vector<mcf::Evaluator::TypeInfo> params;
	for (size_t i = 0; i < paramCount; ++i)
	{
		DebugAssert(functionInfo.Params.Variables[i].IsValid(), u8"");
		DebugAssert(functionInfo.Params.Variables[i].DataType.IsValid(), u8"");
		DebugAssert(GetTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");

		params.emplace_back(functionInfo.Params.Variables[i].DataType);
	}

	DebugAssert(functionInfo.Name.empty() == false, u8"");
	if (DefineFunction(functionInfo.Name, functionInfo) == false )
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::Invalid::Make();
	}
	return mcf::Object::Extern::Make(functionInfo.Name, params, functionInfo.Params.HasVariadic);
}

mcf::Evaluator::FunctionInfo mcf::Evaluator::Object::EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate) const noexcept
{
	mcf::Evaluator::FunctionInfo functionInfo;
	functionInfo.Name = intermediate->GetName();
	if (functionInfo.Name.empty())
	{
		DebugMessage(u8"구현 필요");
		return mcf::Evaluator::FunctionInfo();
	}

	const bool isParamsValid = EvalFunctionParamsIntermediate(functionInfo.Params, intermediate->GetUnsafeFunctionParamsPointer());
	if (isParamsValid == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Evaluator::FunctionInfo();
	}

	functionInfo.ReturnType = EvalTypeSignatureIntermediate(intermediate->GetUnsafeReturnTypePointer());
	if (functionInfo.ReturnType.IsValid() == false || GetTypeInfo(functionInfo.ReturnType.Name).IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Evaluator::FunctionInfo();
	}

	return functionInfo;
}

const bool mcf::Evaluator::Object::EvalFunctionParamsIntermediate(_Out_ mcf::Evaluator::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate) const noexcept
{
	outParams = mcf::Evaluator::FunctionParams();
	const size_t size = intermediate->GetParamCount();
	for (size_t i = 0; i < size; ++i)
	{
		outParams.Variables.emplace_back(EvalVariavbleSignatureIntermediate(intermediate->GetUnsafeParamPointerAt(i)));
		if (outParams.Variables.back().IsValid() == false )
		{
			DebugMessage(u8"구현 필요");
			return false;
		}
	}
	outParams.HasVariadic = intermediate->HasVariadic();
	return true;
}

mcf::Evaluator::Variable mcf::Evaluator::Object::EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate) const noexcept
{
	mcf::Evaluator::Variable variable;
	variable.Name = intermediate->GetName();
	if (variable.Name.empty())
	{
		DebugMessage(u8"구현 필요");
		return mcf::Evaluator::Variable();
	}

	variable.DataType = EvalTypeSignatureIntermediate(intermediate->GetUnsafeTypeSignaturePointer());
	if (variable.DataType.IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Evaluator::Variable();
	}

	return variable;
}

mcf::Evaluator::TypeInfo mcf::Evaluator::Object::EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate) const noexcept
{
	mcf::Object::Pointer object = EvalExpression(intermediate->GetUnsafeSignaturePointer());
	DebugAssert(object->GetType() == mcf::Object::Type::EXPRESSION, u8"");
	mcf::Object::Expression::Interface* expressionObject = static_cast<mcf::Object::Expression::Interface*>(object.get());
	TypeInfo typeInfo;
	switch (expressionObject->GetExpressionType())
	{
	case mcf::Object::Expression::Type::TYPE_IDENTIFIER:
		typeInfo = static_cast<mcf::Object::Expression::TypeIdentifier*>(expressionObject)->GetTypeInfo();
		break;

	default:
		DebugBreak(u8"");
	}

	if (intermediate->IsUnsigned() && typeInfo.IsUnsigned)
	{
		DebugMessage(u8"구현 필요");
		return TypeInfo();
	}
	typeInfo.IsUnsigned = intermediate->IsUnsigned() ? intermediate->IsUnsigned() : typeInfo.IsUnsigned;
	return typeInfo;
}

mcf::Object::Expression::Pointer mcf::Evaluator::Object::EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression) const noexcept
{
	DebugAssert(expression != nullptr, u8"expression가 nullptr이면 안됩니다.");
	mcf::Object::Expression::Pointer object = mcf::Object::Expression::Invalid::Make();
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case AST::Expression::Type::IDENTIFIER: __COUNTER__;
		object = EvalIdentifierExpression(static_cast<const mcf::AST::Expression::Identifier*>(expression));
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
		DebugMessage(u8"구현 필요");
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

mcf::Object::Expression::Pointer mcf::Evaluator::Object::EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression) const noexcept
{
	const std::string name = expression->GetTokenLiteral();
	if (IsIdentifierRegistered(name) == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::Expression::Invalid::Make();
	}

	const TypeInfo typeInfo = GetTypeInfo(name);
	if (typeInfo.IsValid() == true)
	{
		return mcf::Object::Expression::TypeIdentifier::Make(typeInfo);
	}

	DebugMessage(u8"구현 필요");
	return mcf::Object::Expression::Pointer();
}

const bool mcf::Evaluator::Object::DefineType(const std::string& name, const TypeInfo& info) noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(info.IsValid(), u8"함수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		DebugMessage(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_typeInfoMap.emplace(name, info);

	return true;
}

const mcf::Evaluator::TypeInfo mcf::Evaluator::Object::GetTypeInfo(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _typeInfoMap.find(name);
	if (infoFound == _typeInfoMap.end())
	{
		return TypeInfo();
	}
	return infoFound->second;
}

const bool mcf::Evaluator::Object::DefineGlobalVariable(const std::string& name, const Variable& info) noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(info.IsValid(), u8"변수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		DebugMessage(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_globalVariables.emplace(name, info);

	return true;
}

const mcf::Evaluator::Variable mcf::Evaluator::Object::GetGlobalVariable(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _globalVariables.find(name);
	if (infoFound == _globalVariables.end())
	{
		return Variable();
	}
	return infoFound->second;
}

const bool mcf::Evaluator::Object::DefineFunction(const std::string& name, const FunctionInfo& info) noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(info.IsValid(), u8"함수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		DebugMessage(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_functionInfoMap.emplace(name, info);

	return true;
}

const mcf::Evaluator::FunctionInfo mcf::Evaluator::Object::GetFunction(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _functionInfoMap.find(name);
	if (infoFound == _functionInfoMap.end())
	{
		return FunctionInfo();
	}
	return infoFound->second;
}
