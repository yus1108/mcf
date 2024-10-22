#include "pch.h"
#include "compiler.h"

mcf::ASM::MASM64::Predefined::Predefined(const std::string& predefinedCode) noexcept
	: _predefinedCode(predefinedCode)
{
	MCF_DEBUG_ASSERT(_predefinedCode.empty() == false, u8"_predefinedCode가 비어있으면 안됩니다.");
}

const std::string mcf::ASM::MASM64::Predefined::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_predefinedCode.empty() == false, u8"_predefinedCode가 비어있으면 안됩니다.");
	return _predefinedCode;
}

mcf::ASM::MASM64::IncludeLib::IncludeLib(const std::string& code) noexcept
	: _code(code)
{
	MCF_DEBUG_ASSERT(_code.empty() == false, u8"유효하지 않은 _code입니다.");
}

const std::string mcf::ASM::MASM64::IncludeLib::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_code.empty() == false, u8"유효하지 않은 _code입니다.");
	return _code;
}

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

mcf::ASM::MASM64::Literal::Literal(const size_t index, const mcf::Object::Data& value) noexcept
	: _index(index)
	, _value(value)
{
	MCF_DEBUG_ASSERT(_value.second.empty() == false, u8"_value가 비어있으면 안됩니다.");
}

const std::string mcf::ASM::MASM64::Literal::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_value.second.empty() == false, u8"_value가 비어있으면 안됩니다.");
	std::string buffer = "?" + std::to_string(_index);
	switch (_value.first)
	{
	case 1:
		buffer += " byte";
		break;
	case 2:
		buffer += " word";
		break;
	case 4:
		buffer += " dword";
		break;
	case 8:
		buffer += " qword";
		break;
	default:
		MCF_DEBUG_BREAK(u8"사이즈가 올바르지 않습니다.");
		break;
	}

	const size_t valueCount = _value.second.size();
	for (size_t i = 0; i < valueCount; i++)
	{
		buffer += " " + std::to_string(_value.second[i]) + (i == valueCount - 1 ? "" : ",");
	}
	return buffer;
}

mcf::ASM::MASM64::GlobalVariable::GlobalVariable(const mcf::Object::Variable& variable, const mcf::Object::Data& value) noexcept
	: _variable(variable)
	, _value(value)
{
	MCF_DEBUG_ASSERT(_variable.IsValid(), u8"유효하지 않은 _info입니다.");
	MCF_DEBUG_ASSERT(_value.first <= _variable.DataType.IntrinsicSize, u8"값의 데이터 크기가 변수의 타입이 허용하는 데이터의 크기보다 큽니다.");
}

const std::string mcf::ASM::MASM64::GlobalVariable::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_variable.IsValid(), u8"유효하지 않은 _info입니다.");
	MCF_DEBUG_ASSERT(_value.first <= _variable.DataType.IntrinsicSize, u8"값의 데이터 크기가 변수의 타입이 허용하는 데이터의 크기보다 큽니다.");

	std::string buffer;
	buffer = _variable.Name + " " + _variable.DataType.Inspect();
	if (_value.second.empty())
	{
		buffer += " ?";
	}
	else
	{
		const size_t valueCount = _value.second.size();
		for (size_t i = 0; i < valueCount; i++)
		{
			buffer += " " + std::to_string(_value.second[i]) + (i == valueCount - 1 ? "" : ",");
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

	// add predefined code
	{
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("option casemap :none"));

		codes.emplace_back(mcf::ASM::MASM64::SectionCode::Make());
		_currentSection = mcf::ASM::MASM64::Compiler::Section::CODE;

		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("?CopyMemory proc"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tpush rsi"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tpush rdi"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tpush rcx"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tmov rsi, rcx"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tmov rdi, rdx"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tmov rcx, r8"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("?CopyMemory?L1:"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tmov al, byte ptr [rsi]"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tmov byte ptr [rdi], al"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tinc rsi"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tinc rdi"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tloop ?CopyMemory?L1"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tpop rcx"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tpop rdi"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tpop rsi"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("\tret"));
		codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("?CopyMemory endp"));
	}

	if (scopeTree->LiteralIndexMap.empty() == false)
	{
		codes.emplace_back(mcf::ASM::MASM64::SectionData::Make());
		_currentSection = mcf::ASM::MASM64::Compiler::Section::DATA;

		for (auto pairIter : scopeTree->LiteralIndexMap)
		{
			codes.emplace_back(mcf::ASM::MASM64::Literal::Make(pairIter.second.first, pairIter.second.second));
		}
	}

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
			if (CompileIncludeLib(codes, static_cast<const mcf::IR::IncludeLib*>(irObject), scopeTree) == false)
			{
				MCF_DEBUG_TODO(u8"컴파일에 실패하였습니다!");
			}
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

		case IR::Type::WHILE: __COUNTER__;
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

	codes.emplace_back(mcf::ASM::MASM64::Predefined::Make("END"));

	return std::move(codes);
}

const bool mcf::ASM::MASM64::Compiler::Object::CompileIncludeLib(_Out_ mcf::ASM::PointerVector& outCodes, _In_ const mcf::IR::IncludeLib* irCode, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	MCF_UNUSED(scopeTree);
	outCodes.emplace_back(mcf::ASM::MASM64::IncludeLib::Make(irCode->Inspect()));
	return true;
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
	{
		const mcf::IR::Expression::Integer* integer = static_cast<const mcf::IR::Expression::Integer*>(expressionIR);
		if (integer->IsNaturalInteger())
		{
			if (integer->IsUInt8())
			{
				data.first = data.first >= 1 ? data.first : 1;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetUInt8());
			}
			else if (integer->IsUInt16())
			{
				data.first = data.first >= 1 ? data.first : 2;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetUInt16());
			}
			else if (integer->IsUInt32())
			{
				data.first = data.first >= 1 ? data.first : 4;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetUInt32());
			}
			else if (integer->IsUInt64())
			{
				data.first = data.first >= 1 ? data.first : 8;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetUInt64());
			}
		}
		else
		{
			if (integer->IsInt8())
			{
				data.first = data.first >= 1 ? data.first : 1;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetInt8());
			}
			else if (integer->IsInt16())
			{
				data.first = data.first >= 1 ? data.first : 2;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetInt16());
			}
			else if (integer->IsInt32())
			{
				data.first = data.first >= 1 ? data.first : 4;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetInt32());
			}
			else if (integer->IsInt64())
			{
				data.first = data.first >= 1 ? data.first : 8;
				data.second.emplace_back(static_cast<const mcf::IR::Expression::Integer*>(expressionIR)->GetInt64());
			}
		}
		break;
	}

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
	{
		const mcf::IR::Expression::Initializer* initializer = static_cast<const mcf::IR::Expression::Initializer*>(expressionIR);
		const size_t keyCount = initializer->GetKeyExpressionCount();
		for (size_t i = 0; i < keyCount; ++i)
		{
			const mcf::Object::Data KeyData = EvaluateExpressionInCompileTime(initializer->GetUnsafeKeyExpressionPointerAt(i), scopeTree);
			MCF_DEBUG_ASSERT(i == 0 || data.first == KeyData.first, u8"사이즈가 일치 하지 않습니다!");
			data.first = data.first >= KeyData.first ? data.first : KeyData.first;
			data.second.insert(data.second.end(), KeyData.second.begin(), KeyData.second.end());
		}
		break;
	}

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expressionIR->GetExpressionType()), mcf::ENUM_INDEX(expressionIR->GetExpressionType()), expressionIR->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

	return data;
}
