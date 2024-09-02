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

			inline static void MemoryAllocator_AddSize(_Inout_ std::vector<size_t>& outSizes, _Inout_ std::vector<size_t>& outPaddings, _Inout_ std::vector<size_t>& outOffsets, 
				const size_t alignment, const size_t defaultAlignment, const size_t size) noexcept
			{
				DebugAssert(size > 0, u8"사이즈는 0일 수 없습니다.");
				DebugAssert(size < alignment, u8"사이즈는 alignment 보다 크면 안됩니다.");
				DebugAssert(outSizes.size() == outPaddings.size() && outPaddings.size() == outOffsets.size(), u8"모든 벡터들은 아이템의 갯수가 같아야 합니다.");

				const size_t offsetWithoutPadding = outOffsets.empty() ? 0 : (outOffsets.back() + outSizes.back());
				const size_t prevDefaultAlignedPadding = outOffsets.empty() ? alignment : (defaultAlignment - (offsetWithoutPadding % defaultAlignment));
				const size_t prevAlignedPadding = alignment - (offsetWithoutPadding % alignment);

				size_t estimatedOffset = offsetWithoutPadding;
				if (size > prevDefaultAlignedPadding)
				{
					estimatedOffset = offsetWithoutPadding + prevDefaultAlignedPadding;
				}

				if (estimatedOffset + size > offsetWithoutPadding + prevAlignedPadding)
				{
					estimatedOffset = offsetWithoutPadding + prevAlignedPadding;
				}

				if (offsetWithoutPadding != estimatedOffset)
				{
					outPaddings.back() = estimatedOffset - offsetWithoutPadding;
				}
				outOffsets.emplace_back(estimatedOffset);
				outSizes.emplace_back(size);
				outPaddings.emplace_back(alignment - ((estimatedOffset + size) % alignment));
			}
		}
	}
}

void mcf::Evaluator::MemoryAllocator::AddSize(const size_t size) noexcept
{
	DebugAssert(size > 0, u8"사이즈는 0일 수 없습니다.");
	if (size > _alignment)
	{
		Realign(size + DEFAULT_ALIGNMENT - (size % DEFAULT_ALIGNMENT));
	}
	Internal::MemoryAllocator_AddSize(_sizes, _paddings, _offsets, _alignment, DEFAULT_ALIGNMENT, size);
}

void mcf::Evaluator::MemoryAllocator::Realign(const size_t alignment) noexcept
{
	DebugAssert(alignment > 0, u8"Alignment는 0일 수 없습니다.");
	_alignment = alignment;
	std::vector<size_t> outSizes;
	std::vector<size_t> outPaddings;
	std::vector<size_t> outOffsets;

	const size_t prevSizesCount = _sizes.size();
	for (size_t i = 0; i < prevSizesCount; ++i)
	{
		Internal::MemoryAllocator_AddSize(outSizes, outPaddings, outOffsets, _alignment, DEFAULT_ALIGNMENT, _sizes[i]);
	}
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

	_beginCodes.emplace_back(mcf::IR::ASM::ProcBegin::Make(info.Name));
	_beginCodes.emplace_back(mcf::IR::ASM::Push::Make(mcf::IR::ASM::Register::RBP));
	_endCodes.emplace_back(mcf::IR::ASM::ProcEnd::Make(info.Name));
	_endCodes.emplace_back(mcf::IR::ASM::Ret::Make());
	_endCodes.emplace_back(mcf::IR::ASM::Pop::Make(mcf::IR::ASM::Register::RBP));

	const size_t paramCount = info.Params.Variables.size();
	const mcf::Object::TypeInfo paramType = info.LocalScope->FindTypeInfo("qword");
	DebugAssert(paramType.IsValid(), u8"");
	for (size_t i = 0; i < paramCount && i < MCF_ARRAY_SIZE(registerParams); ++i)
	{
		mcf::IR::ASM::Address target(paramType, mcf::IR::ASM::Register::RSP, PARAM_OFFSET + i * 0x8);
		_beginCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, registerParams[i]));
	}
}

void mcf::Evaluator::FunctionIRGenerator::AddLetStatement(_Notnull_ const mcf::IR::Let* object) noexcept
{
	mcf::Object::VariableInfo variableInfo = object->GetInfo();
	DebugAssert(variableInfo.IsValid(), u8"variableInfo가 유효하지 않습니다.");
	DebugAssert(variableInfo.IsGlobal == false, u8"함수안의 변수는 로컬이어야 하는데 글로벌로 등록이 되어 있습니다.");
	mcf::Object::Variable variable = variableInfo.Variable;
	DebugAssert(variable.IsVariadic() == false, u8"변수 선언시 variadic은 불가능 합니다.");
	const size_t variableIndex = _localMemory.GetCount();
	_localMemory.AddSize(variable.GetTypeSize());

	const mcf::IR::Expression::Interface* assignExpression = object->GetUnsafeAssignExpressionPointer();
	DebugAssert(assignExpression != nullptr, u8"assignExpression가 nullptr이면 안됩니다.");
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (assignExpression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
	{
		if (variable.DataType.IsArrayType() == true)
		{
			DebugMessage(u8"구현 필요");
			return;
		}

		mcf::IR::ASM::Address target(variable.DataType, mcf::IR::ASM::Register::RSP, _localMemory.GetOffset(variableIndex));
		switch (variable.DataType.GetSize())
		{
		case sizeof(__int32):
		{
			if (variable.DataType.IsUnsigned)
			{
				DebugMessage(u8"구현 필요");
			}
			else
			{
				_localAssignCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, static_cast<const mcf::IR::Expression::Integer*>(assignExpression)->GetInt32()));
			}
			break;
		}

		case sizeof(__int64) :
		{
			if (variable.DataType.IsUnsigned)
			{
				_localAssignCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, static_cast<const mcf::IR::Expression::Integer*>(assignExpression)->GetUInt64()));
			}
			else
			{
				DebugMessage(u8"구현 필요");
			}
			break;
		}

		default:
			DebugMessage(u8"구현 필요");
			break;
		}
		
		break;
	}

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	default:
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(assignExpression->GetExpressionType()), mcf::ENUM_INDEX(assignExpression->GetExpressionType()), assignExpression->Inspect().c_str());
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

}

mcf::IR::PointerVector mcf::Evaluator::FunctionIRGenerator::GenerateIRCode(void) noexcept
{
	mcf::IR::PointerVector finalCodes;
	const size_t beginCodeCount = _beginCodes.size();
	for (size_t i = 0; i < beginCodeCount; i++)
	{
		finalCodes.emplace_back(std::move(_beginCodes[i]));
	}
	_beginCodes.clear();

	if (_localMemory.IsEmpty() == false)
	{
		const size_t reservedMemory = _localMemory.GetTotalSize();
		finalCodes.emplace_back(mcf::IR::ASM::Sub::Make(mcf::IR::ASM::Register::RSP, static_cast<unsigned __int64>(reservedMemory)));

		const size_t localAssignCodeCount = _localAssignCodes.size();
		for (size_t i = 0; i < localAssignCodeCount; i++)
		{
			finalCodes.emplace_back(std::move(_localAssignCodes[i]));
		}

		finalCodes.emplace_back(mcf::IR::ASM::Add::Make(mcf::IR::ASM::Register::RSP, static_cast<unsigned __int64>(reservedMemory)));
	}

	const size_t endCodeCount = _endCodes.size();
	for (size_t i = 0; i < endCodeCount; i++)
	{
		finalCodes.emplace_back(std::move(_endCodes[endCodeCount - 1 - i]));
	}
	_endCodes.clear();

	return std::move(finalCodes);
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
	for (size_t i = 0; i < paramCount; ++i)
	{
		DebugAssert(functionInfo.Params.Variables[i].IsValid(), u8"");
		DebugAssert(functionInfo.Params.Variables[i].DataType.IsValid(), u8"");
		if (functionInfo.Params.Variables[i].DataType.IsVariadic)
		{
			continue;
		}

		DebugAssert(scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");
	}

	if (scope->DefineFunction(functionInfo.Name, functionInfo) == false)
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

	return mcf::IR::Func::Make(std::move(objects));
}

mcf::IR::PointerVector mcf::Evaluator::Object::EvalFunctionBlockStatement(const mcf::Object::FunctionInfo& info, _Notnull_ const mcf::AST::Statement::Block* statement) noexcept
{
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
			generator.AddLetStatement(static_cast<mcf::IR::Let*>(object.get()));
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
