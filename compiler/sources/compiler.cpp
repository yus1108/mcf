#include "pch.h"
#include "compiler.h"

namespace mcf
{
	namespace Internal
	{
		static const std::string ConvertTypeInfoToString(const mcf::Object::TypeInfo& typeInfo) noexcept
		{
			MCF_DEBUG_ASSERT(typeInfo.IsValid(), u8"TypeInfo가 유효하지 않습니다.");
			if (typeInfo.IsVariadic)
			{
				return "VARARG";
			}

			std::string buffer = typeInfo.Name;
			return buffer;
		}
	}
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
	return _definedType.Name + " typedef " + Internal::ConvertTypeInfoToString(_sourceType);
}

mcf::ASM::PointerVector mcf::ASM::MASM64::Compiler::Object::GenerateCodes(_In_ const mcf::IR::Program* program, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept
{
	mcf::ASM::PointerVector codes;

	const size_t programCount = program->GetObjectCount();
	for (size_t i = 0; i < programCount; i++)
	{
		const mcf::IR::Interface* const irObject = program->GetUnsafeKeyObjectPointerAt(i);
		MCF_DEBUG_ASSERT(irObject != nullptr, "irObject는 nullptr이 아니어야 합니다!");

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
				MCF_DEBUG_TODO("컴파일에 실패하였습니다!");
			}
			break;

		case IR::Type::EXTERN: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::LET: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::FUNC: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
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
	if (_currentSection != mcf::ASM::MASM64::Compiler::Section::DATA)
	{
		outCodes.emplace_back(mcf::ASM::MASM64::SectionData::Make());
		_currentSection = mcf::ASM::MASM64::Compiler::Section::DATA;
	}
	outCodes.emplace_back(mcf::ASM::MASM64::Typedef::Make(irCode->GetDefinedType(), irCode->GetSourceType()));
	
	MCF_UNUSED(scopeTree);
	return true;
}
