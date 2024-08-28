#include "pch.h"
#include "evaluator.h"

namespace mcf
{
	namespace Evaluator
	{
		namespace Internal
		{
			inline static const bool IsStringUInt64(const std::string& stringValue) noexcept
			{
				static std::string uInt64MaxString = std::to_string(UINT64_MAX);

				if (stringValue.size() > uInt64MaxString.size())
				{
					return false;
				}

				const size_t size = stringValue.size();
				for (size_t i = 0; i < size; i++)
				{
					if (mcf::Internal::IS_DIGIT(stringValue[i]) == false)
					{
						return false;
					}
				}

				if (stringValue.size() < uInt64MaxString.size())
				{
					return true;
				}

				for (size_t i = 0; i < size; i++)
				{
					if (stringValue[i] > uInt64MaxString[i])
					{
						return false;
					}

					if (stringValue[i] < uInt64MaxString[i])
					{
						return true;
					}
				}
				return true;
			}
		}
	}
}

void mcf::Evaluator::Allocator::AddSize(const size_t size) noexcept
{
	DebugAssert(size > 0, u8"사이즈는 0일 수 없습니다.");
	if (size > _sizeLeft)
	{
		if (size > _minimumAlignment)
		{
			const size_t mod = size % DEFAULT_ALIGNMENT;
			_minimumAlignment = mod > 0 ? (size - mod + DEFAULT_ALIGNMENT) : size;
		}

		if (_totalSize <= _minimumAlignment)
		{
			paddings.emplace_back(_minimumAlignment - _totalSize);
		}
		else
		{
			const size_t mod = _totalSize % _minimumAlignment;
			paddings.emplace_back(_minimumAlignment - mod);
		}
		_sizeLeft = _minimumAlignment;
	}
	else
	{
		paddings.emplace_back(0);
		_sizeLeft -= size;
		if (_sizeLeft == 0)
		{
			_sizeLeft = _minimumAlignment;
		}
	}
	sizes.emplace_back(size);
	_totalSize += paddings.back() + _minimumAlignment;
}

mcf::Evaluator::FunctionIRGenerator::FunctionIRGenerator(const mcf::Object::FunctionInfo& info) noexcept
{

	constexpr const mcf::IR::ASM::Register registerParams[] =
	{
		mcf::IR::ASM::Register::RCX,
		mcf::IR::ASM::Register::RDX,
		mcf::IR::ASM::Register::R8,
		mcf::IR::ASM::Register::R9
	};

	codes.emplace_back(mcf::IR::ASM::Proc::Make(info.Name));
	codes.emplace_back(mcf::IR::ASM::Push::Make(mcf::IR::ASM::Register::RBP));
	alignedStack += 0x10;

	const size_t paramCount = info.Params.Variables.size();
	size_t paramSizeAllocation = 0;
	size_t minimumParamSizeAllocation = ALIGNMENT;
	size_t currParamSizeAllocation = ALIGNMENT;
	for (size_t i = 0; i < paramCount && i < MCF_ARRAY_SIZE(registerParams); ++i)
	{
		paramSizeAllocation += info.Params.Variables[i].GetTypeSize();
	}
	alignedStack + 0x20; // 5th variable

	DebugMessage(u8"구현 필요");
}

const bool mcf::Evaluator::FunctionIRGenerator::AddLocalVariable(_Notnull_ const mcf::IR::Let* object) noexcept
{
	UNUSED(object);
	DebugMessage(u8"구현 필요");
	return false;
}

mcf::IR::PointerVector mcf::Evaluator::FunctionIRGenerator::GenerateIRCode(void) const noexcept
{
	DebugMessage(u8"구현 필요");
	return mcf::IR::PointerVector();
}

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
		object = EvalLetStatement(static_cast<const mcf::AST::Statement::Let*>(statement), scope);
		break;

	case AST::Statement::Type::BLOCK: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::RETURN: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::FUNC: __COUNTER__;
		object = EvalFuncStatement( static_cast<const mcf::AST::Statement::Func*>(statement), scope );
		break;

	case AST::Statement::Type::MAIN: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case AST::Statement::Type::EXPRESSION: __COUNTER__;
	{
		mcf::IR::Expression::Pointer expressionObject = EvalExpression(static_cast<const mcf::AST::Statement::Expression*>(statement)->GetUnsafeExpression(), scope);
		if (expressionObject.get() == nullptr || expressionObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
		{
			DebugMessage(u8"구현 필요");
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
	mcf::Object::FunctionInfo functionInfo = EvalFunctionSignatureIntermediate(signature, scope);
	scope->MakeLocalScopeToFunctionInfo(functionInfo);
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

		if (functionInfo.Params.Variables[i].DataType.IsVariadic)
		{
			continue;
		}

		DebugAssert(scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");
		params.emplace_back(functionInfo.Params.Variables[i].DataType);
	}

	DebugAssert(functionInfo.Name.empty() == false, u8"");
	if (scope->DefineFunction(functionInfo.Name, functionInfo) == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}
	return mcf::IR::Extern::Make(functionInfo.Name, params, functionInfo.Params.HasVariadic());
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalLetStatement(_Notnull_ const mcf::AST::Statement::Let* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	mcf::Object::Variable variable = EvalVariavbleSignatureIntermediate(statement->GetUnsafeSignaturePointer(), scope);
	if (variable.IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	mcf::Object::VariableInfo info = scope->DefineVariable(variable.Name, variable);
	if (info.IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	const mcf::AST::Expression::Interface* expression = statement->GetUnsafeExpressionointer();
	if (expression == nullptr)
	{
		return mcf::IR::Let::Make(info, nullptr);
	}

	mcf::IR::Expression::Pointer expressionObject = EvalExpression(statement->GetUnsafeExpressionointer(), scope);
	if (expressionObject.get() == nullptr)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	if (ValidateVariableTypeAndValue(info, expressionObject.get()) == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	return mcf::IR::Let::Make(info, std::move(expressionObject));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalFuncStatement(_Notnull_ const mcf::AST::Statement::Func* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	const mcf::AST::Intermediate::FunctionSignature* signature = statement->GetUnsafeSignaturePointer();
	mcf::Object::FunctionInfo functionInfo = EvalFunctionSignatureIntermediate(signature, scope);
	scope->MakeLocalScopeToFunctionInfo(functionInfo);
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
		if (functionInfo.Params.Variables[i].DataType.IsVariadic)
		{
			continue;
		}

		DebugAssert(scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");
		params.emplace_back(functionInfo.Params.Variables[i].DataType);
	}

	if ( scope->DefineFunction( functionInfo.Name, functionInfo ) == false )
	{
		DebugMessage( u8"구현 필요" );
		return mcf::IR::Invalid::Make();
	}

	const mcf::AST::Statement::Block* functionBlock = statement->GetUnsafeBlockPointer();
	mcf::IR::PointerVector objects = EvalFunctionBlockStatement(functionInfo, functionBlock);
	if (objects.empty() == true)
	{
		DebugMessage( u8"구현 필요" );
		return mcf::IR::Invalid::Make();
	}

	return mcf::IR::Func::Make(functionInfo.Name, params, functionInfo.Params.HasVariadic(), std::move(objects));
}

mcf::IR::PointerVector mcf::Evaluator::Object::EvalFunctionBlockStatement(const mcf::Object::FunctionInfo& info, _Notnull_ const mcf::AST::Statement::Block* statement) noexcept
{
	UNUSED(info, statement);
	mcf::Evaluator::FunctionIRGenerator generator(info);
	
	mcf::IR::PointerVector objects;
	const size_t statementCount = statement->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		mcf::IR::Pointer object = EvalStatement(statement->GetUnsafeStatementPointerAt(i), info.LocalScope);
		if (object.get() == nullptr)
		{
			DebugMessage(u8"구현 필요");
			return mcf::IR::PointerVector();
		}

		constexpr const size_t IR_TYPE_COUNT_BEGIN = __COUNTER__;
		switch (object->GetType())
		{
		case IR::Type::LET: __COUNTER__;
			generator.AddLocalVariable(static_cast<mcf::IR::Let*>(object.get()));
			DebugMessage(u8"구현 필요");
			break;

		case IR::Type::EXPRESSION: __COUNTER__;
			DebugMessage(u8"구현 필요");
			break;

		case IR::Type::ASM: __COUNTER__; [[fallthrough]];
		case IR::Type::INCLUDELIB: __COUNTER__; [[fallthrough]];
		case IR::Type::EXTERN: __COUNTER__; [[fallthrough]];
		case IR::Type::FUNC: __COUNTER__; [[fallthrough]];
		case IR::Type::PROGRAM: __COUNTER__; [[fallthrough]];
		default:
			objects.clear();
			DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. IRType=%s(%zu) ConvertedString=`%s`",
				mcf::IR::CONVERT_TYPE_TO_STRING(object->GetType()), mcf::ENUM_INDEX(object->GetType()), object->Inspect().c_str());
		}
		constexpr const size_t IR_TYPE_COUNT = __COUNTER__ - IR_TYPE_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::IR::Type::COUNT) == IR_TYPE_COUNT, "IR type count is changed. this SWITCH need to be changed as well.");
	}

	objects = generator.GenerateIRCode();

	DebugMessage(u8"구현 필요");
	return std::move(objects);
}

mcf::Object::FunctionInfo mcf::Evaluator::Object::EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept
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

	if (intermediate->IsReturnTypeVoid())
	{
		return functionInfo;
	}

	functionInfo.ReturnType = EvalTypeSignatureIntermediate(intermediate->GetUnsafeReturnTypePointer(), scope);
	if (functionInfo.ReturnType.IsValid() == false || scope->FindTypeInfo(functionInfo.ReturnType.Name).IsValid() == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	if (functionInfo.ReturnType.HasUnknownArrayIndex())
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	return functionInfo;
}

const bool mcf::Evaluator::Object::EvalFunctionParamsIntermediate(_Out_ mcf::Object::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept
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

		if (outParams.Variables.back().DataType.HasUnknownArrayIndex())
		{
			DebugMessage(u8"구현 필요");
			return false;
		}
	}

	if (intermediate->HasVariadic())
	{
		mcf::Object::Variable variadic;
		variadic.Name = intermediate->GetUnsafeVariadic()->GetIdentifier();
		variadic.DataType.IsVariadic = true;
		outParams.Variables.emplace_back(variadic);
	}
	return true;
}

mcf::Object::Variable mcf::Evaluator::Object::EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept
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

mcf::Object::TypeInfo mcf::Evaluator::Object::EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept
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

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept
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
		object = EvalIntegerExpression(static_cast<const mcf::AST::Expression::Integer*>(expression), scope);
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
		object = EvalInitializerExpression(static_cast<const mcf::AST::Expression::Initializer*>(expression), scope);
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

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept
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

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIntegerExpression(_Notnull_ const mcf::AST::Expression::Integer* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept
{
	UNUSED(scope);

	std::string stringValue = expression->GetTokenLiteral();
	if (Internal::IsStringUInt64(stringValue) == false)
	{
		DebugMessage(u8"구현 필요");
		return mcf::IR::Expression::Invalid::Make();
	}
#if defined(_WIN32)
	unsigned __int64 value = std::stoull(stringValue);
#else
#error Failing compilation
#endif
	return mcf::IR::Expression::Integer::Make(value);
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIndexExpression(_Notnull_ const mcf::AST::Expression::Index* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept
{
	const mcf::AST::Expression::Interface* leftExpression = expression->GetUnsafeLeftExpressionPointer();
	mcf::IR::Expression::Pointer leftObject = EvalExpression(leftExpression, scope);
	DebugAssert(leftObject->GetType() == mcf::IR::Type::EXPRESSION, u8"구현 필요");

	const mcf::AST::Expression::Interface* indexExpression = expression->GetUnsafeIndexExpressionPointer();

	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (leftObject->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__;
	{
		mcf::Object::TypeInfo info = static_cast<const mcf::IR::Expression::TypeIdentifier*>(leftObject.get())->GetInfo();
		if (indexExpression == nullptr)
		{
			info.ArraySizeList.emplace_back(0);
			return mcf::IR::Expression::TypeIdentifier::Make(info);
		}
		mcf::IR::Expression::Pointer indexObject = EvalExpression(indexExpression, scope);
		DebugAssert(indexObject->GetType() == mcf::IR::Type::EXPRESSION, u8"구현 필요");

		mcf::Object::TypeInfo arrayTypeInfo = MakeArrayTypeInfo(info, indexObject.get());
		if (arrayTypeInfo.IsValid() == false)
		{
			DebugMessage(u8"구현 필요");
			return mcf::IR::Expression::Invalid::Make();
		}
		return mcf::IR::Expression::TypeIdentifier::Make(arrayTypeInfo);
	}

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
	default:
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Expression::CONVERT_TYPE_TO_STRING(leftExpression->GetExpressionType()), mcf::ENUM_INDEX(leftExpression->GetExpressionType()), leftExpression->ConvertToString().c_str());
	}
	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT = __COUNTER__ - LEFT_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == LEFT_EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

	return mcf::IR::Expression::Pointer();
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalInitializerExpression(_Notnull_ const mcf::AST::Expression::Initializer* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept
{
	mcf::IR::Expression::PointerVector keyVector;
	const size_t expressionCount = expression->GetKeyExpressionCount();
	for (size_t i = 0; i < expressionCount; i++)
	{
		keyVector.emplace_back(EvalExpression(expression->GetUnsafeKeyExpressionPointerAt(i), scope));
		if (keyVector.back().get() == nullptr || keyVector.back()->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
		{
			DebugMessage(u8"구현 필요");
			return mcf::IR::Expression::Invalid::Make();
		}
	}
	return mcf::IR::Expression::Initializer::Make(std::move(keyVector));
}

mcf::Object::TypeInfo mcf::Evaluator::Object::MakeArrayTypeInfo(_Notnull_ mcf::Object::TypeInfo info, _Notnull_ const mcf::IR::Expression::Interface* index) const noexcept
{
	DebugAssert(info.IsValid(), u8"구현 필요");

	constexpr const size_t INDEX_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (index->GetExpressionType())
	{
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; 
	{
		const mcf::IR::Expression::Integer* integerObject = static_cast<const mcf::IR::Expression::Integer*>(index);
		if (integerObject->IsUnsignedValue() == false)
		{
			DebugMessage(u8"구현 필요");
			return mcf::Object::TypeInfo();
		}
		info.ArraySizeList.emplace_back(integerObject->GetUInt64());
		break;
	}

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	default:
		info = mcf::Object::TypeInfo();
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(index->GetExpressionType()), mcf::ENUM_INDEX(index->GetExpressionType()), index->Inspect().c_str());
	}
	constexpr const size_t INDEX_EXPRESSION_TYPE_COUNT = __COUNTER__ - INDEX_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == INDEX_EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

	return info;
}

const bool mcf::Evaluator::Object::ValidateVariableTypeAndValue(_Notnull_ mcf::Object::VariableInfo info, _Notnull_ const mcf::IR::Expression::Interface* value) const noexcept
{
	constexpr const size_t ASSIGNED_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (value->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
	{
		if (info.Variable.DataType.IsArrayType())
		{
			DebugMessage(u8"구현 필요");
			return false;
		}

		const mcf::IR::Expression::Integer* integerObject = static_cast<const mcf::IR::Expression::Integer*>(value);
		if (info.Variable.DataType.IsUnsigned == false && integerObject->IsUnsignedValue() == false)
		{
			DebugMessage(u8"구현 필요");
			return false;
		}

		if (integerObject->GetSize() > info.Variable.DataType.IntrinsicSize)
		{
			DebugMessage(u8"구현 필요");
			return false;
		}
		break;
	}

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
	{
		if (info.Variable.DataType.IsArrayType() == false)
		{
			DebugMessage(u8"구현 필요");
			return false;
		}

		mcf::Object::VariableInfo newInfo = info;
		newInfo.Variable.DataType.ArraySizeList.pop_back();

		const mcf::IR::Expression::Initializer* initializerObject = static_cast<const mcf::IR::Expression::Initializer*>(value);
		const size_t expressionCount = initializerObject->GetKeyExpressionCount();
		for (size_t i = 0; i < expressionCount; i++)
		{
			
			if (ValidateVariableTypeAndValue(newInfo, initializerObject->GetUnsafeKeyExpressionPointerAt(i)) == false)
			{
				DebugMessage(u8"구현 필요");
				return false;
			}
		}
		break;
	}

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	default:
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(value->GetExpressionType()), mcf::ENUM_INDEX(value->GetExpressionType()), value->Inspect().c_str());
	}
	constexpr const size_t ASSIGNED_EXPRESSION_TYPE_COUNT = __COUNTER__ - ASSIGNED_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == ASSIGNED_EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");
	return true;
}
