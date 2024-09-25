#include "pch.h"
#include "compiler.h"

mcf::ASM::MASM64::Typedef::Typedef(const mcf::Object::TypeInfo& definedType, const mcf::Object::TypeInfo& sourceType) noexcept
	: _definedType(definedType)
	, _sourceType(sourceType)
{
	MCF_DEBUG_ASSERT(_definedType.IsValid(), u8"유효하지 않은 _definedType입니다.");
	MCF_DEBUG_ASSERT(_sourceType.IsValid(), u8"유효하지 않은 _sourceType입니다.");
}

const std::string mcf::ASM::MASM64::Typedef::ConvertToString(void) const noexcept
{
	return _definedType.Name + " typedef " + _sourceType.Inspect();
}

mcf::ASM::MASM64::Proto::Proto(const std::string& externFunction) noexcept
	: _externFunction(externFunction)
{
	MCF_DEBUG_ASSERT(_externFunction.empty() == false, u8"_externFunction가 비어있으면 안됩니다.");
}

const std::string mcf::ASM::MASM64::Proto::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_externFunction.empty() == false, u8"_externFunction가 비어있으면 안됩니다.");
	return _externFunction;
}

mcf::ASM::MASM64::GlobalVariable::GlobalVariable(const mcf::Object::Variable& variable, const mcf::Object::Data& value) noexcept
	: _variable(variable)
	, _value(value)
{
	MCF_DEBUG_ASSERT(_variable.IsValid(), u8"유효하지 않은 _info입니다.");
}

const std::string mcf::ASM::MASM64::GlobalVariable::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_variable.IsValid(), u8"유효하지 않은 _info입니다.");

	std::string buffer;
	buffer = _variable.Name + " " + _variable.DataType.Inspect();
	if (_value.empty())
	{
		buffer += " ?";
	}
	else
	{
		const size_t valueCount = _value.size();
		for (size_t i = 0; i < valueCount; i++)
		{
			buffer += " " + std::to_string(static_cast<unsigned int>(_value[i])) + ",";
		}
	}
	return buffer;
}

mcf::ASM::MASM64::Proc::Proc(const std::string& functionDefinition) noexcept
	: _functionDefinition(functionDefinition)
{
	MCF_DEBUG_ASSERT(_functionDefinition.empty() == false, u8"_functionDefinition가 비어있으면 안됩니다.");
}

const std::string mcf::ASM::MASM64::Proc::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_functionDefinition.empty() == false, u8"_functionDefinition가 비어있으면 안됩니다.");
	return _functionDefinition;
}

mcf::ASM::PointerVector mcf::ASM::MASM64::Compiler::Object::GenerateCodes(_In_ const mcf::IR::Program* program, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	mcf::ASM::PointerVector codes;

	const size_t programCount = program->GetObjectCount();
	for (size_t i = 0; i < programCount; i++)
	{
		const mcf::IR::Interface* const irObject = program->GetUnsafeKeyObjectPointerAt(i);
		MCF_DEBUG_ASSERT(irObject != nullptr, u8"irObject는 nullptr이 아니어야 합니다!");

		constexpr const size_t IR_TYPE_COUNT_BEGIN = __COUNTER__;
		switch (irObject->GetType())
		{
		case IR::Type::EXPRESSION: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::ASM: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::INCLUDELIB: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::TYPEDEF: __COUNTER__;
			if (CompileTypedef(codes, static_cast<const mcf::IR::Typedef*>(irObject), scopeTree) == false)
			{
				MCF_DEBUG_TODO(u8"컴파일에 실패하였습니다!");
			}
			break;

		case IR::Type::EXTERN: __COUNTER__;
			if (CompileExtern(codes, static_cast<const mcf::IR::Extern*>(irObject), scopeTree) == false)
			{
				MCF_DEBUG_TODO(u8"컴파일에 실패하였습니다!");
			}
			break;

		case IR::Type::LET: __COUNTER__;
			if (CompileLet(codes, static_cast<const mcf::IR::Let*>(irObject), scopeTree) == false)
			{
				MCF_DEBUG_TODO(u8"컴파일에 실패하였습니다!");
			}
			break;

		case IR::Type::FUNC: __COUNTER__;
			if (CompileFunc(codes, static_cast<const mcf::IR::Func*>(irObject), scopeTree) == false)
			{
				MCF_DEBUG_TODO(u8"컴파일에 실패하였습니다!");
			}
			break;

		case IR::Type::UNUSEDIR: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::RETURN: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::PROGRAM: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		default:
			MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. GetType=%s(%zu) Inspect=`%s`",
				mcf::IR::CONVERT_TYPE_TO_STRING(irObject->GetType()), mcf::ENUM_INDEX(irObject->GetType()), irObject->Inspect().c_str());
			break;
		}
		constexpr const size_t IR_TYPE_COUNT = __COUNTER__ - IR_TYPE_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::IR::Type::COUNT) == IR_TYPE_COUNT, "IR type count is changed. this SWITCH need to be changed as well.");
	}

	return std::move(codes);
}

const bool mcf::ASM::MASM64::Compiler::Object::CompileTypedef(_Out_ mcf::ASM::PointerVector& outCodes, _In_ const mcf::IR::Typedef* irCode, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	MCF_UNUSED(scopeTree);
	outCodes.emplace_back(mcf::ASM::MASM64::Typedef::Make(irCode->GetDefinedType(), irCode->GetSourceType()));
	return true;
}

const bool mcf::ASM::MASM64::Compiler::Object::CompileLet(_Out_ mcf::ASM::PointerVector& outCodes, _In_ const mcf::IR::Let* irCode, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	MCF_UNUSED(scopeTree);
	MCF_DEBUG_ASSERT(irCode->GetInfo().IsValid(), u8"Let 명령문의 데이터가 유효하지 않습니다!");
	MCF_DEBUG_ASSERT(irCode->GetInfo().IsGlobal, u8"Let 명령문은 global variable 이어야만 합니다!");

	if (_currentSection != mcf::ASM::MASM64::Compiler::Section::DATA)
	{
		outCodes.emplace_back(mcf::ASM::MASM64::SectionData::Make());
		_currentSection = mcf::ASM::MASM64::Compiler::Section::DATA;
	}

	const mcf::IR::Expression::Interface* assignExpression = irCode->GetUnsafeAssignExpressionPointer();
	mcf::Object::Data value = EvaluateExpressionInCompileTime(assignExpression, scopeTree);
	outCodes.emplace_back(mcf::ASM::MASM64::GlobalVariable::Make(irCode->GetInfo().Variable, value));
	return true;
}

const bool mcf::ASM::MASM64::Compiler::Object::CompileFunc(_Out_ mcf::ASM::PointerVector& outCodes, _In_ const mcf::IR::Func* irCode, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	MCF_UNUSED(irCode, scopeTree);

	if (_currentSection != mcf::ASM::MASM64::Compiler::Section::CODE)
	{
		outCodes.emplace_back(mcf::ASM::MASM64::SectionCode::Make());
		_currentSection = mcf::ASM::MASM64::Compiler::Section::CODE;
	}

	outCodes.emplace_back(mcf::ASM::MASM64::Proc::Make(irCode->Inspect()));
	return true;
}

const bool mcf::ASM::MASM64::Compiler::Object::CompileExtern(_Out_ mcf::ASM::PointerVector& outCodes, _In_ const mcf::IR::Extern* irCode, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	MCF_UNUSED(irCode, scopeTree);
	outCodes.emplace_back(mcf::ASM::MASM64::Proto::Make(irCode->Inspect()));
	return true;
}

const mcf::Object::Data mcf::ASM::MASM64::Compiler::Object::EvaluateExpressionInCompileTime(_In_ const mcf::IR::Expression::Interface* expressionIR, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	MCF_UNUSED(scopeTree);

	if (expressionIR == nullptr)
	{
		return mcf::Object::Data();
	}

	mcf::Object::Data data;
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expressionIR->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		data.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetUInt64());
		break;

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expressionIR->GetExpressionType()), mcf::ENUM_INDEX(expressionIR->GetExpressionType()), expressionIR->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

	return data;
}
