#include "pch.h"
#include "evaluator.h"

namespace mcf
{
	namespace Evaluator
	{
		namespace Internal
		{
			constexpr const size_t FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT = 4;
			constexpr const mcf::IR::ASM::Register FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[mcf::ENUM_COUNT<mcf::IR::ASM::RegisterSize>()][FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT] =
			{
				{
					mcf::IR::ASM::Register::INVALID,
					mcf::IR::ASM::Register::INVALID,
					mcf::IR::ASM::Register::INVALID,
					mcf::IR::ASM::Register::INVALID,
				},
								{
					mcf::IR::ASM::Register::CL,
					mcf::IR::ASM::Register::DL,
					mcf::IR::ASM::Register::R8B,
					mcf::IR::ASM::Register::R9B,
				},
				{
					mcf::IR::ASM::Register::CX,
					mcf::IR::ASM::Register::DX,
					mcf::IR::ASM::Register::R8W,
					mcf::IR::ASM::Register::R9W,
				},
				{
					mcf::IR::ASM::Register::ECX,
					mcf::IR::ASM::Register::EDX,
					mcf::IR::ASM::Register::R8D,
					mcf::IR::ASM::Register::R9D,
				},
				{
					mcf::IR::ASM::Register::RCX,
					mcf::IR::ASM::Register::RDX,
					mcf::IR::ASM::Register::R8,
					mcf::IR::ASM::Register::R9,
				},
			};
			constexpr const size_t MINIMUM_FUNCTION_PARAM_STACK_SIZE = FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT * sizeof(size_t);

			static const bool IsStringConvertibleToInt64(const std::string& stringValue) noexcept
			{
				static std::string uInt64MaxString = std::to_string(INT64_MAX);

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

			static const bool IsStringConvertibleToUInt64(const std::string& stringValue) noexcept
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

			static void MemoryAllocator_AddSize(_Inout_ std::vector<size_t>& outSizes, _Inout_ std::vector<size_t>& outPaddings, _Inout_ std::vector<size_t>& outOffsets, 
				const size_t alignment, const size_t defaultAlignment, const size_t size) noexcept
			{
				MCF_DEBUG_ASSERT(0 < size, u8"사이즈는 0일 수 없습니다.");
				MCF_DEBUG_ASSERT(size <= alignment, u8"사이즈는 alignment 보다 크면 안됩니다.");
				MCF_DEBUG_ASSERT(outSizes.size() == outPaddings.size() && outPaddings.size() == outOffsets.size(), u8"모든 벡터들은 아이템의 갯수가 같아야 합니다.");

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
				outPaddings.emplace_back((estimatedOffset % alignment == 0 && size == alignment) ? 0 : (alignment - ((estimatedOffset + size) % alignment)));
			}

			static mcf::Object::Data ConvertStringToData(const std::string& stringLiteral) noexcept
			{
				mcf::Object::Data data;
				data.first = 1;

				const size_t literalLength = stringLiteral.length();
				bool isEscapeChar = false;
				for (size_t i = 0; i < literalLength; i++)
				{
					if (stringLiteral[i] == '\\')
					{
						isEscapeChar = true;
						continue;
					}

					if (isEscapeChar)
					{
						switch (stringLiteral[i])
						{
						case '0':
							data.second.emplace_back(static_cast<unsigned __int8>(0));
							break;
						case 'n':
							data.second.emplace_back(static_cast<unsigned __int8>('\n'));
							break;
						case 't':
							data.second.emplace_back(static_cast<unsigned __int8>('\t'));
							break;
						case 'v':
							data.second.emplace_back(static_cast<unsigned __int8>('\v'));
							break;
						case 'b':
							data.second.emplace_back(static_cast<unsigned __int8>('\b'));
							break;
						case 'r':
							data.second.emplace_back(static_cast<unsigned __int8>('\r'));
							break;
						case 'f':
							data.second.emplace_back(static_cast<unsigned __int8>('\f'));
							break;
						case 'a':
							data.second.emplace_back(static_cast<unsigned __int8>('\a'));
							break;
						case '\'':
							data.second.emplace_back(static_cast<unsigned __int8>('\''));
							break;
						case '"':
							data.second.emplace_back(static_cast<unsigned __int8>('\"'));
							break;
						case '\\':
							data.second.emplace_back(static_cast<unsigned __int8>('\\'));
							break;
						case '?':
							data.second.emplace_back(static_cast<unsigned __int8>('\?'));
							break;
						default:
							break;
						}
						isEscapeChar = false;
						continue;
					}
					data.second.emplace_back(static_cast<unsigned __int8>(stringLiteral[i]));
				}

				data.second.emplace_back(static_cast<unsigned __int8>(0));
				return data;
			}
		}
	}
}

mcf::Evaluator::FunctionCallIRGenerator::FunctionCallIRGenerator(const mcf::Object::FunctionInfo& info) noexcept
	: _info(info)
	, _currParamIndex(0)
{
	MCF_DEBUG_ASSERT(_info.IsValid(), u8"인자로 받은 함수 정보가 유효하지 않습니다.");
	const size_t paramCount = _info.Params.Variables.size();
	for (size_t i = 0; i < paramCount; ++i)
	{
		if (_info.Params.Variables[i].IsVariadic())
		{
			_isNeedToAddVariadicMemory = true;
			break;
		}
		_localMemory.AddSize(_info.Params.Variables[i].GetTypeSize());
	}
}

void mcf::Evaluator::FunctionCallIRGenerator::AddVariadicMemory(_Notnull_ const mcf::IR::Expression::Interface* expression) noexcept
{
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		_localMemory.AddSize(mcf::IR::Expression::Interface::GetDataTypeFromExpression(expression).GetSize());
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "this determines the actual size of variadic params when calling the function");
}

void mcf::Evaluator::FunctionCallIRGenerator::FinishAddingVariadicMemory(void) noexcept
{
	MCF_DEBUG_ASSERT(_info.Params.HasVariadic() == true && _isNeedToAddVariadicMemory == true, u8"variadic 변수 메모리 할당이 필요하지 않습니다.");
	_isNeedToAddVariadicMemory = false;
}

const bool mcf::Evaluator::FunctionCallIRGenerator::AddParameter(_Notnull_ const mcf::IR::Expression::Interface* expression, _Notnull_ const FunctionIRGenerator* functionGenerator) noexcept
{
	MCF_DEBUG_ASSERT(_isNeedToAddVariadicMemory == false, u8"variadic 변수 메모리 할당이 완료되지 않았습니다.");
	MCF_DEBUG_ASSERT(_info.Params.IsVoid() == false, u8"함수 정의에 인자가 없습니다.");

	const bool isVariadicParam = _info.Params.HasVariadic() && _info.Params.GetVariadicIndex() <= _currParamIndex;
	if (isVariadicParam == false && mcf::Evaluator::Object::ValidateVariableTypeAndValue(_info.Params.Variables[_currParamIndex], expression) == false)
	{
		MCF_DEBUG_TODO(u8"함수 정의의 인자 타입과 호출시의 인자 타입이 다릅니다.");
		return false;
	}

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::GlobalVariableIdentifier* globalExpression = static_cast<const mcf::IR::Expression::GlobalVariableIdentifier*>(expression);
		const mcf::Object::Variable& variable = globalExpression->GetVariable();
		if (variable.DataType.IsArrayType() || variable.DataType.IsStruct)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}

		if (_currParamIndex < Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT)
		{
			const mcf::IR::ASM::RegisterSize registerSize = mcf::IR::ASM::GET_REGISTER_SIZE_BY_VALUE(variable.GetTypeSize());
			_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[mcf::ENUM_INDEX(registerSize)][_currParamIndex++], globalExpression));
		}
		else
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		return true;
	}

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::Object::Variable& variable = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(expression)->GetVariable();
		const mcf::IR::ASM::Address address(variable.DataType, mcf::IR::ASM::Register::RSP, GetReservedMemory() + functionGenerator->GetLocalVariableOffset(variable.Name));

		if (variable.DataType.IsArrayType() || variable.DataType.IsStruct)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}

		if (_currParamIndex < Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT)
		{
			const mcf::IR::ASM::RegisterSize registerSize = mcf::IR::ASM::GET_REGISTER_SIZE_BY_VALUE(variable.GetTypeSize());
			_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[mcf::ENUM_INDEX(registerSize)][_currParamIndex++], address));
		}
		else
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		return true;
	}

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STRING: __COUNTER__; 
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;
	
	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
	{
		const mcf::IR::Expression::StaticCast* staticCastExpression = static_cast<const mcf::IR::Expression::StaticCast*>(expression);
		const mcf::Object::TypeInfo& originalValueType = staticCastExpression->GetOriginalDatType();
		const mcf::Object::TypeInfo& castedValueType = staticCastExpression->GetCastedDatType();
		if (originalValueType.IsArrayType() && castedValueType.IsIntegerType())
		{
			return AddUnsafePointerParameterInternal(staticCastExpression->GetUnsafeOriginalExpressionPointer(), functionGenerator);
		}
		MCF_DEBUG_TODO(u8"구현 필요");
		break;
	}

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;
		
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "this implements assembly code for adding parameter for calling function");

	return false;
}

const bool mcf::Evaluator::FunctionCallIRGenerator::AddParameter(const mcf::IR::ASM::SizeOf& source) noexcept
{
	MCF_DEBUG_ASSERT(_isNeedToAddVariadicMemory == false, u8"variadic 변수 메모리 할당이 완료되지 않았습니다.");
	MCF_DEBUG_ASSERT(_info.Params.IsVoid() == false, u8"함수 정의에 인자가 없습니다.");
	if (_currParamIndex < Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT)
	{
		_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[mcf::ENUM_INDEX(mcf::IR::ASM::RegisterSize::QWORD)][_currParamIndex++], source));
	}
	else
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}
	return true;
}

const bool mcf::Evaluator::FunctionCallIRGenerator::AddUnsafePointerParameter(_Notnull_ const mcf::IR::Expression::String* stringExpression) noexcept
{
	MCF_DEBUG_ASSERT(_isNeedToAddVariadicMemory == false, u8"variadic 변수 메모리 할당이 완료되지 않았습니다.");
	MCF_DEBUG_ASSERT(_info.Params.IsVoid() == false, u8"함수 정의에 인자가 없습니다.");
	AddPointerParameterInternal(mcf::IR::ASM::UnsafePointerAddress(stringExpression));
	return true;
}

const bool mcf::Evaluator::FunctionCallIRGenerator::AddUnsafePointerParameter(const mcf::IR::ASM::UnsafePointerAddress& source) noexcept
{
	MCF_DEBUG_ASSERT(_isNeedToAddVariadicMemory == false, u8"variadic 변수 메모리 할당이 완료되지 않았습니다.");
	MCF_DEBUG_ASSERT(_info.Params.IsVoid() == false, u8"함수 정의에 인자가 없습니다.");
	AddPointerParameterInternal(mcf::IR::ASM::UnsafePointerAddress(source, GetReservedMemory()));
	return true;
}

void mcf::Evaluator::FunctionCallIRGenerator::AddGeneratedIRCode(_Out_ mcf::IR::ASM::PointerVector& outVector) noexcept
{
	MCF_DEBUG_ASSERT(_isNeedToAddVariadicMemory == false, u8"variadic 변수 메모리 할당이 완료되지 않았습니다.");
	outVector.emplace_back(mcf::IR::ASM::Sub::Make(mcf::IR::ASM::Register::RSP, GetReservedMemory()));
	
	const size_t localCodeCount = _localCodes.size();
	for (size_t i = 0; i < localCodeCount; ++i)
	{
		outVector.emplace_back(std::move(_localCodes[localCodeCount - 1 - i]));
	}
	outVector.emplace_back(mcf::IR::ASM::Call::Make(_info.Name));

	outVector.emplace_back(mcf::IR::ASM::Add::Make(mcf::IR::ASM::Register::RSP, GetReservedMemory()));

	_localCodes.clear();
	_localMemory.Clear();
	_currParamIndex = 0;
}

const size_t mcf::Evaluator::FunctionCallIRGenerator::GetReservedMemory(void) const noexcept
{
	const size_t localMemorySize = _localMemory.GetTotalSize();
	return localMemorySize < Internal::MINIMUM_FUNCTION_PARAM_STACK_SIZE ? Internal::MINIMUM_FUNCTION_PARAM_STACK_SIZE : localMemorySize;
}

void mcf::Evaluator::FunctionCallIRGenerator::AddPointerParameterInternal(const mcf::IR::ASM::UnsafePointerAddress& source) noexcept
{
	if (_currParamIndex < Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT)
	{
		_localCodes.emplace_back(mcf::IR::ASM::Lea::Make(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[mcf::ENUM_INDEX(mcf::IR::ASM::RegisterSize::QWORD)][_currParamIndex++], source));
	}
	else
	{
		MCF_DEBUG_TODO(u8"구현 필요");
	}
}

const bool mcf::Evaluator::FunctionCallIRGenerator::AddUnsafePointerParameterInternal(_Notnull_ const mcf::IR::Expression::Interface* expression, _Notnull_ const FunctionIRGenerator* functionGenerator) noexcept
{
	MCF_DEBUG_ASSERT(_isNeedToAddVariadicMemory == false, u8"variadic 변수 메모리 할당이 완료되지 않았습니다.");
	MCF_DEBUG_ASSERT(_info.Params.IsVoid() == false, u8"함수 정의에 인자가 없습니다.");

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::Object::Variable& variable = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(expression)->GetVariable();
		const mcf::IR::ASM::UnsafePointerAddress address(mcf::IR::ASM::Register::RSP, GetReservedMemory() + functionGenerator->GetLocalVariableOffset(variable.Name));
		if (_currParamIndex < Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT)
		{
			_localCodes.emplace_back(mcf::IR::ASM::Lea::Make(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[mcf::ENUM_INDEX(mcf::IR::ASM::RegisterSize::QWORD)][_currParamIndex++], address));
		}
		else
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		return true;
	}

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		AddUnsafePointerParameter(mcf::IR::ASM::UnsafePointerAddress(static_cast<const mcf::IR::Expression::String*>(expression)));
		return true;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "internal function hacks assembly codes of adding params for calling function");
	return false;
}

void mcf::Evaluator::MemoryAllocator::AddSize(const size_t size) noexcept
{
	MCF_DEBUG_ASSERT(size > 0, u8"사이즈는 0일 수 없습니다.");
	if (size > _alignment)
	{
		Realign(size + DEFAULT_ALIGNMENT - (size % DEFAULT_ALIGNMENT));
	}
	Internal::MemoryAllocator_AddSize(_sizes, _paddings, _offsets, _alignment, DEFAULT_ALIGNMENT, size);
}

const bool mcf::Evaluator::MemoryAllocator::IsEmpty(void) const noexcept
{
	MCF_DEBUG_ASSERT(_sizes.size() == _paddings.size() && _paddings.size() == _offsets.size(), u8"모든 벡터들의 아이템 숫자가 같아야 합니다.");
	return _sizes.empty();
}

void mcf::Evaluator::MemoryAllocator::Realign(const size_t alignment) noexcept
{
	MCF_DEBUG_ASSERT(alignment > 0, u8"Alignment는 0일 수 없습니다.");
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

bool mcf::Evaluator::FunctionIRGenerator::ConvertStatements(
	_Inout_ mcf::Evaluator::FunctionIRGenerator& inOutGenerator, 
	_Notnull_ const mcf::IR::PointerVector* statementsPointer, 
	_Notnull_ const mcf::Object::Scope* scope,
	const std::string& breakLabel /* = NOT_ALLOW_BREAK_LABEL() */) noexcept
{
	bool hasReturn = false;
	const size_t statementCount = statementsPointer->size();
	for (size_t i = 0; i < statementCount; i++)
	{
		if (hasReturn == true)
		{
			MCF_DEBUG_ASSERT(inOutGenerator.IsReturnTypeVoid() == false, u8"리턴문이 들어왔을때 함수의 리턴타입이 있는지 확인을 했어야 합니다.");
			MCF_DEBUG_TODO(u8"리턴문 이후에 다른 명령문은 들어올 수 없습니다.");
			return false;
		}

		const mcf::IR::Interface* object = (*statementsPointer)[i].get();
		MCF_DEBUG_ASSERT(object != nullptr, u8"object가 유효하지 않습니다.");

		constexpr const size_t IR_TYPE_COUNT_BEGIN = __COUNTER__;
		switch (object->GetType())
		{
		case IR::Type::LET: __COUNTER__;
			if (inOutGenerator.AddLetStatement(static_cast<const mcf::IR::Let*>(object), scope) == false)
			{
				MCF_DEBUG_TODO(u8"함수 정의에 실패하였습니다. 원인: Let문 코드 변환 오류.");
				return false;
			}
			break;

		case IR::Type::EXPRESSION: __COUNTER__;
			if (inOutGenerator.AddExpressionStatement(static_cast<const mcf::IR::Expression::Interface*>(object), scope) == false)
			{
				MCF_DEBUG_TODO(u8"함수 정의에 실패하였습니다. 원인: 표현문 코드 변환 오류.");
				return false;
			}
			break;

		case IR::Type::UNUSEDIR: __COUNTER__;
			break;

		case IR::Type::RETURN: __COUNTER__;
			if (inOutGenerator.IsReturnTypeVoid() == true)
			{
				MCF_DEBUG_TODO(u8"리턴 타입이 없는 함수에는 리턴이 들어올 수 없습니다.");
				return false;
			}
			inOutGenerator.AddReturnStatement(static_cast<const mcf::IR::Return*>(object));
			hasReturn = true;
			break;

		case IR::Type::WHILE: __COUNTER__;
			if (inOutGenerator.AddWhileStatement(static_cast<const mcf::IR::While*>(object)) == false)
			{
				return false;
			}
			break;

		case IR::Type::BREAK: __COUNTER__;
			if (breakLabel == NOT_ALLOW_BREAK_LABEL())
			{
				MCF_DEBUG_TODO(u8"구현 필요");
				return false;
			}
			inOutGenerator.AddBreakStatement(breakLabel);
			break;

		case IR::Type::ASM: __COUNTER__; [[fallthrough]];
		case IR::Type::INCLUDELIB: __COUNTER__; [[fallthrough]];
		case IR::Type::TYPEDEF: __COUNTER__; [[fallthrough]];
		case IR::Type::EXTERN: __COUNTER__; [[fallthrough]];
		case IR::Type::FUNC: __COUNTER__; [[fallthrough]];
		case IR::Type::PROGRAM: __COUNTER__; [[fallthrough]];
		default:
			MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. IRType=%s(%zu) ConvertedString=`%s`",
				mcf::IR::CONVERT_TYPE_TO_STRING(object->GetType()), mcf::ENUM_INDEX(object->GetType()), object->Inspect().c_str());
			return false;
		}
		constexpr const size_t IR_TYPE_COUNT = __COUNTER__ - IR_TYPE_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::IR::Type::COUNT) == IR_TYPE_COUNT, "Convert IR statements to ASM codes.");
	}

	if (inOutGenerator.IsReturnTypeVoid() == hasReturn)
	{
		MCF_DEBUG_ASSERT(inOutGenerator.IsReturnTypeVoid() == false, u8"리턴문이 들어왔을때 함수의 리턴타입이 있는지 확인을 했어야 합니다.");
		MCF_DEBUG_TODO(u8"리턴문이 들어오지 않았습니다.");
		return false;
	}

	if (scope->IsAllVariablesUsed() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}

	return true;
}

mcf::Evaluator::FunctionIRGenerator::FunctionIRGenerator(const mcf::Object::FunctionInfo& info) noexcept
	: _returnType(info.ReturnType)
	, _name(info.Name)
{
	// 함수 관련 초기화 코드를 추가합니다.
	_beginCodes.emplace_back(mcf::IR::ASM::ProcBegin::Make(info.Name));
	_beginCodes.emplace_back(mcf::IR::ASM::Push::Make(mcf::IR::ASM::Register::RBP));
	_endCodes.emplace_back(mcf::IR::ASM::ProcEnd::Make(info.Name));
	_endCodes.emplace_back(mcf::IR::ASM::Ret::Make());
	_endCodes.emplace_back(mcf::IR::ASM::Pop::Make(mcf::IR::ASM::Register::RBP));

	// 함수 인자 초기화 관련 코드를 처리합니다.
	const size_t paramCount = info.Params.Variables.size();
	const mcf::Object::TypeInfo paramType = mcf::Object::TypeInfo::MakePrimitive(true, "qword", sizeof(unsigned __int64));
	MCF_DEBUG_ASSERT(paramType.IsValid(), u8"");
	for (size_t i = 0; i < paramCount && i < Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTER_COUNT; ++i)
	{
		const size_t offset = PARAM_OFFSET + i * 0x8;
		mcf::IR::ASM::Address target(paramType, mcf::IR::ASM::Register::RSP, offset);
		_paramOffsetMap[info.Params.Variables[i].Name] = offset;
		_beginCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[mcf::ENUM_INDEX(mcf::IR::ASM::RegisterSize::QWORD)][i]));
	}
}

mcf::IR::ASM::PointerVector mcf::Evaluator::FunctionIRGenerator::GenerateIRCode(void) noexcept
{
	mcf::IR::ASM::PointerVector finalCodes;
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
		_endCodes.emplace_back(mcf::IR::ASM::Add::Make(mcf::IR::ASM::Register::RSP, static_cast<unsigned __int64>(reservedMemory)));
	}

	if (_localCodes.empty() == false)
	{
		const size_t localAssignCodeCount = _localCodes.size();
		for (size_t i = 0; i < localAssignCodeCount; i++)
		{
			finalCodes.emplace_back(std::move(_localCodes[i]));
		}
	}

	const size_t endCodeCount = _endCodes.size();
	for (size_t i = 0; i < endCodeCount; i++)
	{
		finalCodes.emplace_back(std::move(_endCodes[endCodeCount - 1 - i]));
	}
	_endCodes.clear();

	return std::move(finalCodes);
}

const mcf::IR::ASM::Register mcf::Evaluator::FunctionIRGenerator::EvaluateExpressionAndGetRegisterForMoving(
	const mcf::IR::ASM::Register availableRegisters[sizeof(__int64)], 
	_Notnull_ const mcf::IR::Expression::Interface* expression) noexcept
{
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(expression);
		const size_t typeSize = localVariableIdentifier->GetVariable().DataType.GetSize();
		const size_t typeBytes = typeSize / sizeof(__int8);
		MCF_DEBUG_ASSERT(typeSize % sizeof(__int8) == 0 && sizeof(__int8) <= typeSize && typeBytes <= sizeof(__int64), u8"유효하지 않은 데이터 크기가 들어왔습니다.");
		if (availableRegisters[typeBytes] == mcf::IR::ASM::Register::INVALID)
		{
			MCF_DEBUG_TODO(u8"기본 타입중에 처리할 수 있는 크기가 아닙니다.");
			break;
		}
		return availableRegisters[typeBytes];
	}

	case IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::INTEGER: __COUNTER__;
	{
		const mcf::IR::Expression::Integer* integerExpression = static_cast<const mcf::IR::Expression::Integer*>(expression);
		if (integerExpression->IsNaturalInteger())
		{
			if (integerExpression->IsUInt8())
			{
				return availableRegisters[sizeof(__int8)];
			} 
			else if (integerExpression->IsUInt16())
			{
				return availableRegisters[sizeof(__int16)];
			}
			else if (integerExpression->IsUInt32())
			{
				return availableRegisters[sizeof(__int32)];
			}
			else if (integerExpression->IsUInt64())
			{
				return availableRegisters[sizeof(__int64)];
			}
			else
			{
				MCF_DEBUG_TODO(u8"구현 필요");
				break;
			}
		}
		else
		{
			if (integerExpression->IsInt8())
			{
				return availableRegisters[sizeof(__int8)];
			}
			else if (integerExpression->IsInt16())
			{
				return availableRegisters[sizeof(__int16)];
			}
			else if (integerExpression->IsInt32())
			{
				return availableRegisters[sizeof(__int32)];
			}
			else if (integerExpression->IsInt64())
			{
				return availableRegisters[sizeof(__int64)];
			}
			else
			{
				MCF_DEBUG_TODO(u8"구현 필요");
				break;
			}
		}
		break;
	}

	case IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		break;

	case IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "Return register evaluated by expression for `mov` instruction.");
	return mcf::IR::ASM::Register::INVALID;
}

mcf::IR::ASM::Pointer mcf::Evaluator::FunctionIRGenerator::MoveExpressionToRegister(const mcf::IR::ASM::Register targetRegister, _Notnull_ const mcf::IR::Expression::Interface* expression) noexcept
{
	MCF_DEBUG_ASSERT(targetRegister != mcf::IR::ASM::Register::INVALID, u8"targetRegister가 유효하지 않습니다.");

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(expression);
		const size_t typeSize = localVariableIdentifier->GetVariable().DataType.GetSize();
		const size_t typeBytes = typeSize / sizeof(__int8);
		MCF_DEBUG_ASSERT(typeSize % sizeof(__int8) == 0 && sizeof(__int8) <= typeSize && typeBytes <= sizeof(__int64), u8"유효하지 않은 데이터 크기가 들어왔습니다.");
		if (mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(targetRegister) < typeBytes)
		{
			MCF_DEBUG_TODO(u8"타겟 레지스터보다 표현식의 타입 크기가 더 큽니다.");
			break;
		}

		const std::string varName = localVariableIdentifier->GetVariable().Name;
		auto paramOffsetPairIter = _paramOffsetMap.find(varName);
		if (paramOffsetPairIter != _paramOffsetMap.end())
		{
			const size_t offset = _localMemory.GetTotalSize() + paramOffsetPairIter->second;
			const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetVariable().DataType, mcf::IR::ASM::Register::RSP, offset);
			return mcf::IR::ASM::Mov::Make(targetRegister, sourceAddress);
		}
		else
		{
			auto localVariableIndexPairIter = _localVariableIndicesMap.find(varName);
			MCF_DEBUG_ASSERT(localVariableIndexPairIter != _localVariableIndicesMap.end(), u8"해당 이름의 지역변수를 찾을 수 없습니다. Name=`%s`", varName.c_str());
			const size_t offset = _localMemory.GetOffset(localVariableIndexPairIter->second);
			const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetVariable().DataType, mcf::IR::ASM::Register::RSP, offset);
			return mcf::IR::ASM::Mov::Make(targetRegister, sourceAddress);
		}
	}

	case IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::INTEGER: __COUNTER__;
	{
		const mcf::IR::Expression::Integer* integerExpression = static_cast<const mcf::IR::Expression::Integer*>(expression);
		if (integerExpression->IsNaturalInteger())
		{
			if (integerExpression->IsUInt8())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetUInt8());
			}
			else if (integerExpression->IsUInt16())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetUInt16());
			}
			else if (integerExpression->IsUInt32())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetUInt32());
			}
			else if (integerExpression->IsUInt64())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetUInt64());
			}
			else
			{
				MCF_DEBUG_TODO(u8"구현 필요");
				break;
			}
		}
		else
		{
			if (integerExpression->IsInt8())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetInt8());
			}
			else if (integerExpression->IsInt16())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetInt16());
			}
			else if (integerExpression->IsInt32())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetInt32());
			}
			else if (integerExpression->IsInt64())
			{
				return mcf::IR::ASM::Mov::Make(targetRegister, integerExpression->GetInt64());
			}
			else
			{
				MCF_DEBUG_TODO(u8"구현 필요");
				break;
			}
		}
		break;
	}

	case IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		break;

	case IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "implement mov with the given expression.");
	return mcf::IR::ASM::Invalid::Make();
}

const bool mcf::Evaluator::FunctionIRGenerator::AddCondition(
	_Notnull_ const mcf::IR::Expression::Interface* condition,
	_Notnull_ const mcf::Object::Scope* scope,
	const std::string& labelTrue,
	const std::string& labelFalse) noexcept
{
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (condition->GetExpressionType())
	{
	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		if (AddConditionalExpression(static_cast<const mcf::IR::Expression::Conditional*>(condition), labelTrue, labelFalse) == false)
		{
			return false;
		}
		break;

	case IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::INTEGER: __COUNTER__;
		if (static_cast<const mcf::IR::Expression::Integer*>(condition)->IsZero())
		{
			_localCodes.emplace_back(mcf::IR::ASM::Jmp::Make(labelFalse));
		}
		break;

	case IR::Expression::Type::CALL: __COUNTER__;
	{
		constexpr mcf::IR::ASM::Register leftRegisters[] =
		{
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::AL,
			mcf::IR::ASM::Register::AX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::EAX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::RAX
		};

		const mcf::IR::Expression::Call* callObject = static_cast<const mcf::IR::Expression::Call*>(condition);
		MCF_DEBUG_ASSERT(callObject->GetInfo().IsValid(), u8"call 표현식이 유효하지 않습니다.");
		MCF_DEBUG_ASSERT(callObject->GetInfo().ReturnType.IsValid(), u8"call 표현식의 반환값 타입이 유효하지 않습니다.");
		const size_t returnTypeSize = callObject->GetInfo().ReturnType.GetSize();
		if (returnTypeSize > MCF_ARRAY_SIZE(leftRegisters) || leftRegisters[returnTypeSize] == mcf::IR::ASM::Register::INVALID)
		{
			MCF_DEBUG_TODO(u8"call 표현식의 반환값 타입의 크기가 기본 데이터 크기여야 합니다.");
			return false;
		}

		if (AddExpressionStatement(condition, scope) == false)
		{
			return false;
		}

		mcf::IR::ASM::Pointer jumpIfFalse = mcf::IR::Expression::Conditional::GenerateJumpIf(Token::Type::EQUAL, labelFalse);
		if (jumpIfFalse.get() == nullptr || jumpIfFalse->GetASMType() == mcf::IR::ASM::Type::INVALID)
		{
			MCF_DEBUG_TODO(u8"구현 필요.");
			return false;
		}

		switch (leftRegisters[returnTypeSize])
		{
		case mcf::IR::ASM::Register::AL:
			_localCodes.emplace_back(mcf::IR::ASM::Cmp::Make(leftRegisters[returnTypeSize], static_cast<__int8>(0)));
			break;

		case mcf::IR::ASM::Register::AX:
			_localCodes.emplace_back(mcf::IR::ASM::Cmp::Make(leftRegisters[returnTypeSize], static_cast<__int16>(0)));
			break;

		case mcf::IR::ASM::Register::EAX:
			_localCodes.emplace_back(mcf::IR::ASM::Cmp::Make(leftRegisters[returnTypeSize], static_cast<__int32>(0)));
			break;

		case mcf::IR::ASM::Register::RAX:
			_localCodes.emplace_back(mcf::IR::ASM::Cmp::Make(leftRegisters[returnTypeSize], static_cast<__int64>(0)));
			break;

		default:
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		_localCodes.emplace_back(std::move(jumpIfFalse));
		break;
	}

	case IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(condition->GetExpressionType()), mcf::ENUM_INDEX(condition->GetExpressionType()), condition->Inspect().c_str());
		return false;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "implement condition codes using expression.");
	return true;
}

const bool mcf::Evaluator::FunctionIRGenerator::AddConditionalExpression(
	_Notnull_ const mcf::IR::Expression::Conditional* object,
	const std::string& labelTrue, 
	const std::string& labelFalse) noexcept
{
	constexpr mcf::IR::ASM::Register leftRegisters[] =
	{
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::AL,
		mcf::IR::ASM::Register::AX,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::EAX,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::RAX
	};

	const mcf::IR::Expression::Interface* left = object->GetUnsafeLeftExpression();
	mcf::IR::ASM::Register leftRegister = EvaluateExpressionAndGetRegisterForMoving(leftRegisters, left);
	const size_t leftRegisterSize = mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(leftRegister);
	if (leftRegister == mcf::IR::ASM::Register::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요.");
		return false;
	}

	constexpr mcf::IR::ASM::Register rightRegisters[] =
	{
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::BL,
		mcf::IR::ASM::Register::BX,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::EBX,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::INVALID,
		mcf::IR::ASM::Register::RBX
	};

	const mcf::IR::Expression::Interface* right = object->GetUnsafeRightExpression();
	mcf::IR::ASM::Register rightRegister = EvaluateExpressionAndGetRegisterForMoving(rightRegisters, right);
	const size_t rightRegisterSize = mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(rightRegister);
	if (rightRegister == mcf::IR::ASM::Register::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요.");
		return false;
	}

	const size_t targetRegisterSize = leftRegisterSize > rightRegisterSize ? leftRegisterSize : rightRegisterSize;

	leftRegister = mcf::IR::ASM::CHANGE_REGISTER_BY_SIZE(leftRegister, targetRegisterSize);
	mcf::IR::ASM::Pointer leftASM = MoveExpressionToRegister(leftRegister, left);
	if (leftASM.get() == nullptr || leftASM->GetASMType() == mcf::IR::ASM::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요.");
		return false;
	}

	mcf::Token::Data token = object->GetInfixOperator();
	if (token.Type == mcf::Token::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요.");
		return false;
	}

	mcf::IR::ASM::Pointer jumpIfTrue = mcf::IR::Expression::Conditional::GenerateJumpIf(token.Type, labelTrue);
	if (jumpIfTrue.get() == nullptr || jumpIfTrue->GetASMType() == mcf::IR::ASM::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요.");
		return false;
	}

	rightRegister = mcf::IR::ASM::CHANGE_REGISTER_BY_SIZE(rightRegister, targetRegisterSize);
	mcf::IR::ASM::Pointer rightASM = MoveExpressionToRegister(rightRegister, right);
	if (rightASM.get() == nullptr || rightASM->GetASMType() == mcf::IR::ASM::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요.");
		return false;
	}

	_localCodes.emplace_back(std::move(leftASM));
	_localCodes.emplace_back(std::move(rightASM));
	_localCodes.emplace_back(mcf::IR::ASM::Cmp::Make(leftRegister, rightRegister));
	_localCodes.emplace_back(std::move(jumpIfTrue));
	_localCodes.emplace_back(mcf::IR::ASM::Jmp::Make(labelFalse));

	return true;
}

const bool mcf::Evaluator::FunctionIRGenerator::AddAssignExpressionWithLocalVariable(
	const mcf::Object::Variable& leftOperand,
	_Notnull_ const mcf::IR::Expression::Interface* rightExpression,
	_Notnull_ const mcf::Object::Scope* scope) noexcept
{
	MCF_DEBUG_ASSERT(scope->FindVariableInfo(leftOperand.Name).IsGlobal == false, u8"leftOperand는 지역 변수여야 합니다.");

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (rightExpression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(rightExpression);
		if (leftOperand.DataType != localVariableIdentifier->GetVariable().DataType)
		{
			MCF_DEBUG_TODO(u8"타입이 일치 하지 않습니다.");
			break;
		}

		if (leftOperand.DataType.IsStringCompatibleType() || leftOperand.DataType.IsArrayType() == true)
		{
			MCF_DEBUG_TODO(u8"배열 타입 구현 필요");
			break;
		}

		if (leftOperand.DataType.IsStruct == true)
		{
			MCF_DEBUG_TODO(u8"구조체 타입 구현 필요");
			break;
		}

		constexpr mcf::IR::ASM::Register registers[] =
		{
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::AL,
			mcf::IR::ASM::Register::AX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::EAX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::RAX
		};

		const size_t targetTypeSize = leftOperand.DataType.GetSize();
		const size_t targetTypeBytes = targetTypeSize / sizeof(__int8);
		MCF_DEBUG_ASSERT(targetTypeSize % sizeof(__int8) == 0 && sizeof(__int8) <= targetTypeSize && targetTypeBytes <= sizeof(__int64), u8"유효하지 않은 데이터 크기가 들어왔습니다.");
		if (registers[targetTypeBytes] == mcf::IR::ASM::Register::INVALID)
		{
			MCF_DEBUG_TODO(u8"기본 타입중에 처리할 수 있는 크기가 아닙니다.");
			break;
		}

		const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetVariable().DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(localVariableIdentifier->GetVariable().Name));
		_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(registers[targetTypeBytes], sourceAddress));

		const mcf::IR::ASM::Address targetAddress(leftOperand.DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
		_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(targetAddress, registers[targetTypeBytes]));
		return true;
	}
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
	{
		if (leftOperand.DataType.IsIntegerType() == false)
		{
			MCF_DEBUG_TODO(u8"정수 타입이 아닌 값에 정수를 대입하려 하고 있습니다. 타입[%s]", leftOperand.DataType.Inspect().c_str());
			break;
		}

		const mcf::IR::Expression::Integer* integerExpression = static_cast<const mcf::IR::Expression::Integer*>(rightExpression);
		if (integerExpression->IsCompatible(leftOperand.DataType) == false)
		{
			MCF_DEBUG_TODO(u8"정수 값과 호환 불가능한 타입입니다. 타입[%s]", leftOperand.DataType.Inspect().c_str());
			break;
		}

		const mcf::IR::ASM::Address target(leftOperand.DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
		switch (leftOperand.DataType.GetSize())
		{
			case sizeof(__int8) :
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt8()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt8()));
				break;

			case sizeof(__int16) :
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt16()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt16()));
				break;

			case sizeof(__int32) :
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt32()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt32()));
				break;

			case sizeof(__int64) :
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt64()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt64()));
				break;

			default:
				MCF_DEBUG_BREAK(u8"integerExpression->IsCompatible(variable.DataType)에서 처리되어야 합니다.");
				break;
		}
		return true;
	}

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
	{
		if (leftOperand.DataType.IsStringCompatibleType() == false)
		{
			MCF_DEBUG_TODO(u8"문자열 호환 가능한 타입(바이트 배열 타입)이 아닌 변수에 문자열을 대입하려 하고 있습니다. 타입[%s]", leftOperand.DataType.Inspect().c_str());
			break;
		}

		const mcf::IR::Expression::String* stringExpression = static_cast<const mcf::IR::Expression::String*>(rightExpression);
		const mcf::IR::ASM::UnsafePointerAddress targetAddress(mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
		FunctionCallIRGenerator generator(scope->FindInternalFunction(mcf::Object::InternalFunctionType::COPY_MEMORY));
		generator.AddUnsafePointerParameter(stringExpression);
		generator.AddUnsafePointerParameter(targetAddress);
		generator.AddParameter(mcf::IR::ASM::SizeOf(stringExpression));
		generator.AddGeneratedIRCode(_localCodes);
		return true;
	}

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
	{
		const mcf::IR::Expression::Arithmetic* arithmeticExpression = static_cast<const mcf::IR::Expression::Arithmetic*>(rightExpression);
		const mcf::IR::ASM::Register sourceRegister = AddArithmeticExpression(arithmeticExpression, scope);
		if (sourceRegister == mcf::IR::ASM::Register::INVALID)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			break;
		}

		const mcf::IR::ASM::Address targetAddress(leftOperand.DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
		_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(targetAddress, sourceRegister));

		return true;
	}

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"Assign 표현식 코드 변환중 오른쪽 표현식에 예상치 못한 값이 들어왔습니다. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(rightExpression->GetExpressionType()), mcf::ENUM_INDEX(rightExpression->GetExpressionType()), rightExpression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "this implements assembly codes for right expression of assign expression in function definition");
	return false;
}

const bool mcf::Evaluator::FunctionIRGenerator::AddAssignExpression(_Notnull_ const mcf::IR::Expression::Assign* expression, _Notnull_ const mcf::Object::Scope* scope) noexcept
{
	const mcf::IR::Expression::Interface* leftOperand = expression->GetUnsafeLeftExpression();
	if (leftOperand == nullptr)
	{
		MCF_DEBUG_TODO(u8"leftOperand가 유효하지 않습니다.");
		return false;
	}

	const mcf::IR::Expression::Interface* rightOperand = expression->GetUnsafeRightExpression();
	if (rightOperand == nullptr)
	{
		MCF_DEBUG_TODO(u8"rightOperand가 유효하지 않습니다.");
		return false;
	}

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (leftOperand->GetExpressionType())
	{
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(leftOperand);
		mcf::Object::Variable localVariable = localVariableIdentifier->GetVariable();
		if (localVariable.IsValid() == false)
		{
			MCF_DEBUG_TODO(u8"leftOperand가 유효하지 않습니다.");
			break;
		}

		if (AddAssignExpressionWithLocalVariable(localVariable, rightOperand, scope) == false)
		{
			MCF_DEBUG_TODO(u8"대입 연산 평가에 실패하였습니다.");
			break;
		}
		return true;
	}

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::CALL: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"Assign 명령문 코드 변환중 왼쪽 표현식에 예상치 못한 값이 들어왔습니다. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(leftOperand->GetExpressionType()), mcf::ENUM_INDEX(leftOperand->GetExpressionType()), leftOperand->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "this implements assembly codes for left expression of assign expression in function definition");
	return false;
}

const mcf::IR::ASM::Register mcf::Evaluator::FunctionIRGenerator::AddArithmeticExpressionWithLocalVariable(
	const mcf::Object::Variable& leftOperand,
	_Notnull_ const mcf::IR::Expression::Interface* rightExpression,
	_Notnull_ const mcf::Object::Scope* scope) noexcept
{
	MCF_DEBUG_ASSERT(scope->FindVariableInfo(leftOperand.Name).IsGlobal == false, u8"leftOperand는 지역 변수여야 합니다.");

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (rightExpression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(rightExpression);
		if (leftOperand.DataType != localVariableIdentifier->GetVariable().DataType)
		{
			MCF_DEBUG_TODO(u8"타입이 일치 하지 않습니다.");
			break;
		}

		if (leftOperand.DataType.IsStringCompatibleType() || leftOperand.DataType.IsArrayType() == true)
		{
			MCF_DEBUG_TODO(u8"배열 타입 구현 필요");
			break;
		}

		if (leftOperand.DataType.IsStruct == true)
		{
			MCF_DEBUG_TODO(u8"구조체 타입 구현 필요");
			break;
		}

		constexpr mcf::IR::ASM::Register registers[] =
		{
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::AL,
			mcf::IR::ASM::Register::AX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::EAX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::RAX
		};

		const size_t targetTypeSize = leftOperand.DataType.GetSize();
		const size_t targetTypeBytes = targetTypeSize / sizeof(__int8);
		MCF_DEBUG_ASSERT(targetTypeSize % sizeof(__int8) == 0 && sizeof(__int8) <= targetTypeSize && targetTypeBytes <= sizeof(__int64), u8"유효하지 않은 데이터 크기가 들어왔습니다.");
		if (registers[targetTypeBytes] == mcf::IR::ASM::Register::INVALID)
		{
			MCF_DEBUG_TODO(u8"기본 타입중에 처리할 수 있는 크기가 아닙니다.");
			break;
		}

		MCF_DEBUG_TODO(u8"구현 필요.");
		break;
	}
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
	{
		if (leftOperand.DataType.IsIntegerType() == false)
		{
			MCF_DEBUG_TODO(u8"정수 타입이 아닌 값에 정수를 대입하려 하고 있습니다. 타입[%s]", leftOperand.DataType.Inspect().c_str());
			break;
		}

		const mcf::IR::Expression::Integer* integerExpression = static_cast<const mcf::IR::Expression::Integer*>(rightExpression);
		if (integerExpression->IsCompatible(leftOperand.DataType) == false)
		{
			MCF_DEBUG_TODO(u8"정수 값과 호환 불가능한 타입입니다. 타입[%s]", leftOperand.DataType.Inspect().c_str());
			break;
		}

		switch (leftOperand.DataType.GetSize())
		{
			case sizeof(__int8) :
			{
				const mcf::IR::ASM::Register target = mcf::IR::ASM::Register::AL;
				const mcf::IR::ASM::Address leftAddress(leftOperand.DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
				_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, leftAddress));
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Add::Make(target, integerExpression->GetUInt8()) : mcf::IR::ASM::Add::Make(target, integerExpression->GetInt8()));
				return target;
			}

			case sizeof(__int16) :
			{
				const mcf::IR::ASM::Register target = mcf::IR::ASM::Register::AX;
				const mcf::IR::ASM::Address leftAddress(leftOperand.DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
				_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, leftAddress));
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Add::Make(target, integerExpression->GetUInt16()) : mcf::IR::ASM::Add::Make(target, integerExpression->GetInt16()));
				return target;
			}

			case sizeof(__int32) :
			{
				const mcf::IR::ASM::Register target = mcf::IR::ASM::Register::EAX;
				const mcf::IR::ASM::Address leftAddress(leftOperand.DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
				_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, leftAddress));
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Add::Make(target, integerExpression->GetUInt32()) : mcf::IR::ASM::Add::Make(target, integerExpression->GetInt32()));
				return target;
			}

			case sizeof(__int64) :
			{
				const mcf::IR::ASM::Register target = mcf::IR::ASM::Register::RAX;
				const mcf::IR::ASM::Address leftAddress(leftOperand.DataType, mcf::IR::ASM::Register::RSP, GetLocalVariableOffset(leftOperand.Name));
				_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, leftAddress));
				_localCodes.emplace_back(leftOperand.DataType.IsUnsigned ? mcf::IR::ASM::Add::Make(target, integerExpression->GetUInt64()) : mcf::IR::ASM::Add::Make(target, integerExpression->GetInt64()));
				return target;
			}

			default:
				MCF_DEBUG_BREAK(u8"integerExpression->IsCompatible(variable.DataType)에서 처리되어야 합니다.");
				break;
		}
	}

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"Assign 표현식 코드 변환중 오른쪽 표현식에 예상치 못한 값이 들어왔습니다. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(rightExpression->GetExpressionType()), mcf::ENUM_INDEX(rightExpression->GetExpressionType()), rightExpression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "this implements assembly codes for right expression of assign expression in function definition");
	return mcf::IR::ASM::Register::INVALID;
}

const mcf::IR::ASM::Register mcf::Evaluator::FunctionIRGenerator::AddArithmeticExpression(_Notnull_ const mcf::IR::Expression::Arithmetic* expression, _Notnull_ const mcf::Object::Scope* scope) noexcept
{
	const mcf::IR::Expression::Interface* leftOperand = expression->GetUnsafeLeftExpression();
	if (leftOperand == nullptr)
	{
		MCF_DEBUG_TODO(u8"leftOperand가 유효하지 않습니다.");
		return mcf::IR::ASM::Register::INVALID;
	}

	const mcf::IR::Expression::Interface* rightOperand = expression->GetUnsafeRightExpression();
	if (rightOperand == nullptr)
	{
		MCF_DEBUG_TODO(u8"rightOperand가 유효하지 않습니다.");
		return mcf::IR::ASM::Register::INVALID;
	}

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (leftOperand->GetExpressionType())
	{
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(leftOperand);
		mcf::Object::Variable localVariable = localVariableIdentifier->GetVariable();
		if (localVariable.IsValid() == false)
		{
			MCF_DEBUG_TODO(u8"localVariable가 유효하지 않습니다.");
			break;
		}
		return AddArithmeticExpressionWithLocalVariable(localVariable, rightOperand, scope);
	}

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::CALL: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"Assign 명령문 코드 변환중 왼쪽 표현식에 예상치 못한 값이 들어왔습니다. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(leftOperand->GetExpressionType()), mcf::ENUM_INDEX(leftOperand->GetExpressionType()), leftOperand->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "this implements assembly codes for left expression of arithmetic expression in function definition");
	return mcf::IR::ASM::Register::INVALID;
}

const bool mcf::Evaluator::FunctionIRGenerator::AddLetStatement(_Notnull_ const mcf::IR::Let * object, _Notnull_ const mcf::Object::Scope * scope) noexcept
{
	mcf::Object::VariableInfo targetVariableInfo = object->GetInfo();
	MCF_DEBUG_ASSERT(targetVariableInfo.IsValid(), u8"variableInfo가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(targetVariableInfo.IsGlobal == false, u8"함수안의 변수는 로컬이어야 하는데 글로벌로 등록이 되어 있습니다.");
	mcf::Object::Variable targetVariable = targetVariableInfo.Variable;
	MCF_DEBUG_ASSERT(targetVariable.IsVariadic() == false, u8"변수 선언시 variadic은 불가능 합니다.");
	_localVariableIndicesMap[targetVariable.Name] = _localMemory.GetCount();
	MCF_DEBUG_ASSERT(targetVariable.DataType.HasUnknownArrayIndex() == false, u8"unknown 배열은 이 함수에 들어오기전에 처리되어야 합니다.");
	_localMemory.AddSize(targetVariable.GetTypeSize());

	const mcf::IR::Expression::Interface* assignExpression = object->GetUnsafeAssignExpressionPointer();
	if (assignExpression == nullptr)
	{
		return true;
	}

	if (AddAssignExpressionWithLocalVariable(targetVariable, assignExpression, scope) == false)
	{
		MCF_DEBUG_TODO(u8"대입 연산 평가에 실패하였습니다.");
		return false;
	}
	return true;
}

const bool mcf::Evaluator::FunctionIRGenerator::AddExpressionStatement(_Notnull_ const mcf::IR::Expression::Interface* object, _Notnull_ const mcf::Object::Scope* scope) noexcept
{
	MCF_UNUSED(scope);
	constexpr const size_t EXPRESSION_OBJECT_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (object->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
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

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
	{
		const mcf::IR::Expression::Call* callObject = static_cast<const mcf::IR::Expression::Call*>(object);
		const mcf::Object::FunctionInfo& functionInfo = callObject->GetInfo();
		FunctionCallIRGenerator callIRGenerator(functionInfo);

		if (functionInfo.Params.IsVoid() == false)
		{
			const size_t paramCount = callObject->GetParamCount();
			if (functionInfo.Params.HasVariadic())
			{
				for (size_t i = functionInfo.Params.GetVariadicIndex(); i < paramCount; i++)
				{
					callIRGenerator.AddVariadicMemory(callObject->GetUnsafeParamPointerAt(i));
				}
				callIRGenerator.FinishAddingVariadicMemory();
			}

			for (size_t i = 0; i < paramCount; i++)
			{
				callIRGenerator.AddParameter(callObject->GetUnsafeParamPointerAt(i), this);
			}
		}
		callIRGenerator.AddGeneratedIRCode(_localCodes);
		return true;
	}

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__;
	{
		const mcf::IR::Expression::Assign* assignObject = static_cast<const mcf::IR::Expression::Assign*>(object);
		if (AddAssignExpression(assignObject, scope) == false)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			break;
		}
		return true;
	}

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(object->GetExpressionType()), mcf::ENUM_INDEX(object->GetExpressionType()), object->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_OBJECT_TYPE_COUNT = __COUNTER__ - EXPRESSION_OBJECT_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_OBJECT_TYPE_COUNT, "Implments assembly code for expression object in function ");
	return false;
}

void mcf::Evaluator::FunctionIRGenerator::AddReturnStatement(_Notnull_ const mcf::IR::Return* object) noexcept
{
	MCF_DEBUG_ASSERT(_returnType.IsValid(), u8"반환 타입이 유효하지 않습니다.");

	const mcf::IR::Expression::Interface* retrunExpression = object->GetUnsafeReturnExpressionPointer();
	MCF_DEBUG_ASSERT(retrunExpression != nullptr, u8"retrunExpression가 nullptr이면 안됩니다.");

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (retrunExpression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(retrunExpression);
		if (_returnType != localVariableIdentifier->GetVariable().DataType)
		{
			MCF_DEBUG_TODO(u8"타입이 일치 하지 않습니다.");
			return;
		}

		if (_returnType.IsStringCompatibleType() || _returnType.IsArrayType() == true)
		{
			MCF_DEBUG_TODO(u8"배열 타입 구현 필요");
			return;
		}

		if (_returnType.IsStruct == true)
		{
			MCF_DEBUG_TODO(u8"구조체 타입 구현 필요");
			return;
		}

		constexpr mcf::IR::ASM::Register returnRegisters[] =
		{
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::AL,
			mcf::IR::ASM::Register::AX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::EAX,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::INVALID,
			mcf::IR::ASM::Register::RAX
		};

		const size_t returnTypeSize = _returnType.GetSize();
		const size_t returnTypeBytes = returnTypeSize / sizeof(__int8);
		MCF_DEBUG_ASSERT(returnTypeSize % sizeof(__int8) == 0 && sizeof(__int8) <= returnTypeSize && returnTypeBytes <= sizeof(__int64), u8"유효하지 않은 데이터 크기가 들어왔습니다.");
		if (returnRegisters[returnTypeBytes] != mcf::IR::ASM::Register::INVALID)
		{
			const std::string varName = localVariableIdentifier->GetVariable().Name;
			auto paramOffsetPairIter = _paramOffsetMap.find(varName);
			if (paramOffsetPairIter != _paramOffsetMap.end())
			{
				const size_t offset = _localMemory.GetTotalSize() + paramOffsetPairIter->second;
				const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetVariable().DataType, mcf::IR::ASM::Register::RSP, offset);
				_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(returnRegisters[returnTypeBytes], sourceAddress ));
			}
			else
			{
				auto localVariableIndexPairIter = _localVariableIndicesMap.find(varName);
				MCF_DEBUG_ASSERT(localVariableIndexPairIter != _localVariableIndicesMap.end(), u8"해당 이름의 지역변수를 찾을 수 없습니다. Name=`%s`", varName.c_str());
				const size_t offset = _localMemory.GetOffset(localVariableIndexPairIter->second);
				const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetVariable().DataType, mcf::IR::ASM::Register::RSP, offset);
				_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(returnRegisters[returnTypeBytes], sourceAddress ));
			}
			break;
		}

		MCF_DEBUG_TODO(u8"기본 타입중에 처리할 수 있는 크기가 아닙니다.");
		break;
	}
	
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
	{
		if (_returnType.IsIntegerType() == false)
		{
			MCF_DEBUG_TODO(u8"정수 타입이 아닌 값에 정수를 대입하려 하고 있습니다. 타입[%s]", _returnType.Inspect().c_str());
			return;
		}

		const mcf::IR::Expression::Integer* integerExpression = static_cast<const mcf::IR::Expression::Integer*>(retrunExpression);
		if (integerExpression->IsCompatible(_returnType) == false)
		{
			MCF_DEBUG_TODO(u8"정수 값과 호환 불가능한 타입입니다. 타입[%s]", _returnType.Inspect().c_str());
			return;
		}

		switch (_returnType.GetSize())
		{
		case sizeof(__int8):
			if (integerExpression->IsZero())
			{
				_localCodes.emplace_back(mcf::IR::ASM::Xor::Make(mcf::IR::ASM::Register::AL, mcf::IR::ASM::Register::AL));
				break;
			}
			_localCodes.emplace_back(_returnType.IsUnsigned ? mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AL, integerExpression->GetUInt8()) : mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AL, integerExpression->GetInt8()));
			break;

		case sizeof(__int16):
			if (integerExpression->IsZero())
			{
				_localCodes.emplace_back(mcf::IR::ASM::Xor::Make(mcf::IR::ASM::Register::AX, mcf::IR::ASM::Register::AX));
				break;
			}
			_localCodes.emplace_back(_returnType.IsUnsigned ? mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AX, integerExpression->GetUInt16()) : mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AX, integerExpression->GetInt16()));
			break;

		case sizeof(__int32):
			if (integerExpression->IsZero())
			{
				_localCodes.emplace_back(mcf::IR::ASM::Xor::Make(mcf::IR::ASM::Register::EAX, mcf::IR::ASM::Register::EAX));
				break;
			}
			_localCodes.emplace_back(_returnType.IsUnsigned ? mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::EAX, integerExpression->GetUInt32()) : mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::EAX, integerExpression->GetInt32()));
			break;

		case sizeof(__int64) :
			if (integerExpression->IsZero())
			{
				_localCodes.emplace_back(mcf::IR::ASM::Xor::Make(mcf::IR::ASM::Register::RAX, mcf::IR::ASM::Register::RAX));
				break;
			}
			_localCodes.emplace_back(_returnType.IsUnsigned ? mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::RAX, integerExpression->GetUInt64()) : mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::EAX, integerExpression->GetInt64()));
			break;

		default:
			MCF_DEBUG_BREAK(u8"integerExpression->IsCompatible(_returnType)에서 처리되어야 합니다.");
			break;
		}
		break;
	}

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
	{
		if (_returnType.IsStringCompatibleType() == false)
		{
			MCF_DEBUG_TODO(u8"문자열 호환 가능한 타입(바이트 배열 타입)이 아닌 변수에 문자열을 대입하려 하고 있습니다. 타입[%s]", _returnType.Inspect().c_str());
			return;
		}
		_localCodes.emplace_back(mcf::IR::ASM::Lea::Make(mcf::IR::ASM::Register::RAX, mcf::IR::ASM::UnsafePointerAddress(static_cast<const mcf::IR::Expression::String*>(retrunExpression))));
		break;
	}

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO( u8"구현 필요." );
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING( retrunExpression->GetExpressionType()), mcf::ENUM_INDEX( retrunExpression->GetExpressionType()), retrunExpression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "Implement assembly code for return statement in function definition here");
}

const bool mcf::Evaluator::FunctionIRGenerator::AddWhileStatement(_Notnull_ const mcf::IR::While* object) noexcept
{
	const mcf::IR::Expression::Interface* condition = object->GetUnsafeConditionExpressionPointer();
	MCF_DEBUG_ASSERT(condition != nullptr, u8"condition이 nullptr면 안됩니다.");

	const mcf::Object::Scope* blockScope = object->GetBlockScope();
	MCF_DEBUG_ASSERT(blockScope != nullptr, u8"blockScope가 nullptr면 안됩니다.");

	const std::string labelBegin = CreateLabelName();
	const std::string labelBlock = CreateLabelName();
	const std::string labelEnd = CreateLabelName();
	_localCodes.emplace_back(mcf::IR::ASM::Label::Make(labelBegin));
	if (AddCondition(static_cast<const mcf::IR::Expression::Conditional*>(condition), blockScope->GetUnsafeParentScopePointer(), labelBlock, labelEnd) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}

	_localCodes.emplace_back(mcf::IR::ASM::Label::Make(labelBlock));
	if (ConvertStatements(*this, object->GetBlockPointer(), blockScope, labelEnd) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}

	_localCodes.emplace_back(mcf::IR::ASM::Jmp::Make(labelBegin));
	_localCodes.emplace_back(mcf::IR::ASM::Label::Make(labelEnd));
	return true;
}

void mcf::Evaluator::FunctionIRGenerator::AddBreakStatement(const std::string& labelBreak) noexcept
{
	_localCodes.emplace_back(mcf::IR::ASM::Jmp::Make(labelBreak));
}

const std::string mcf::Evaluator::FunctionIRGenerator::CreateLabelName(void) noexcept
{
	return "?" + _name + "_L" + std::to_string(_labelCount++);
}

const size_t mcf::Evaluator::FunctionIRGenerator::GetLocalVariableOffset(const std::string& variableName) const noexcept
{
	size_t offset;
	auto sourceParamOffsetPairIter = _paramOffsetMap.find(variableName);
	if (sourceParamOffsetPairIter != _paramOffsetMap.end())
	{
		offset = _localMemory.GetTotalSize() + sourceParamOffsetPairIter->second;
	}
	else
	{
		auto localVariableIndexPairIter = _localVariableIndicesMap.find(variableName);
		MCF_DEBUG_ASSERT(localVariableIndexPairIter != _localVariableIndicesMap.end(), u8"해당 이름의 지역변수를 찾을 수 없습니다. Name=`%s`", variableName.c_str());
		offset = _localMemory.GetOffset(localVariableIndexPairIter->second);
	}
	return offset;
}

const std::string mcf::Evaluator::FunctionIRGenerator::INTERNAL_COPY_MEMORY_NAME = "?CopyMemory";

const bool mcf::Evaluator::Object::ValidateVariableTypeAndValue(const mcf::Object::Variable& variable, _Notnull_ const mcf::IR::Expression::Interface* value) noexcept
{
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (value->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::GlobalVariableIdentifier* globalVariableIdentifier = static_cast<const mcf::IR::Expression::GlobalVariableIdentifier*>(value);
		if (variable.DataType != globalVariableIdentifier->GetVariable().DataType)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		break;
	}

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(value);
		if (variable.DataType != localVariableIdentifier->GetVariable().DataType)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		break;
	}

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		return static_cast<const mcf::IR::Expression::Integer*>(value)->IsCompatible(variable.DataType);

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
	{
		const mcf::IR::Expression::String* stringExpression = static_cast<const mcf::IR::Expression::String*>(value);
		return variable.DataType.IsStringCompatibleType() && variable.DataType.HasUnknownArrayIndex() == false && stringExpression->GetSize() <= variable.DataType.GetSize();
	}

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
	{
		if (variable.DataType.IsArrayType() == false)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}

		mcf::Object::Variable arrayItem = variable;
		arrayItem.DataType.ArraySizeList.pop_back();

		const mcf::IR::Expression::Initializer* initializerObject = static_cast<const mcf::IR::Expression::Initializer*>(value);
		const size_t expressionCount = initializerObject->GetKeyExpressionCount();
		for (size_t i = 0; i < expressionCount; i++)
		{
			if (ValidateVariableTypeAndValue(arrayItem, initializerObject->GetUnsafeKeyExpressionPointerAt(i)) == false)
			{
				MCF_DEBUG_TODO(u8"구현 필요");
				return false;
			}
		}
		break;
	}

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
	{
		const mcf::Object::TypeInfo castedDataType = static_cast<const mcf::IR::Expression::StaticCast*>(value)->GetCastedDatType();
		if (variable.DataType != castedDataType)
		{
			MCF_DEBUG_TODO(u8"캐스팅된 값의 타입과 변수의 타입이 일치하지 않습니다.");
			return false;
		}
		break;
	}
		

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(value->GetExpressionType()), mcf::ENUM_INDEX(value->GetExpressionType()), value->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "validate variable and value");
	return true;
}

const bool mcf::Evaluator::Object::ValidateExpressionTypes(_Notnull_ const mcf::IR::Expression::Interface* left, _Notnull_ const mcf::IR::Expression::Interface* right) noexcept
{
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (left->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::GlobalVariableIdentifier* globalVariableIdentifier = static_cast<const mcf::IR::Expression::GlobalVariableIdentifier*>(left);
		if (ValidateVariableTypeAndValue(globalVariableIdentifier->GetVariable(), right) == false)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		break;
	}

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(left);
		if (ValidateVariableTypeAndValue(localVariableIdentifier->GetVariable(), right) == false)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		break;
	}

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
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

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(left->GetExpressionType()), mcf::ENUM_INDEX(left->GetExpressionType()), left->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "validate left expression and right expression");
	return true;
}
		
mcf::IR::Program::Pointer mcf::Evaluator::Object::EvalProgram(_Notnull_ const mcf::AST::Program* program, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	mcf::IR::PointerVector objects;
	const size_t statementCount = program->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		objects.emplace_back(EvalStatement(program->GetUnsafeStatementPointerAt(i), scope, false));
	}
	return mcf::IR::Program::Make(std::move(objects));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement, _Notnull_ mcf::Object::Scope* scope, const bool isBreakAllowed) noexcept
{
	mcf::IR::Pointer object = mcf::IR::Invalid::Make();
	constexpr const size_t STATEMENT_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (statement->GetStatementType())
	{
	case AST::Statement::Type::INCLUDE_LIBRARY: __COUNTER__;
		object = mcf::IR::IncludeLib::Make(static_cast<const mcf::AST::Statement::IncludeLibrary*>(statement)->GetLibPath());
		break;

	case AST::Statement::Type::TYPEDEF: __COUNTER__;
		object = EvalTypedefStatement(static_cast<const mcf::AST::Statement::Typedef*>(statement), scope);
		break;

	case AST::Statement::Type::EXTERN: __COUNTER__;
		object = EvalExternStatement(static_cast<const mcf::AST::Statement::Extern*>(statement), scope);
		break;

	case AST::Statement::Type::LET: __COUNTER__;
		object = EvalLetStatement(static_cast<const mcf::AST::Statement::Let*>(statement), scope);
		break;

	case AST::Statement::Type::BLOCK: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case AST::Statement::Type::RETURN: __COUNTER__;
		object = EvalReturnStatement(static_cast<const mcf::AST::Statement::Return*>(statement), scope);
		break;

	case AST::Statement::Type::FUNC: __COUNTER__;
		object = EvalFuncStatement(static_cast<const mcf::AST::Statement::Func*>(statement), scope);
		break;

	case AST::Statement::Type::MAIN: __COUNTER__;
		object = EvalMainStatement(static_cast<const mcf::AST::Statement::Main*>(statement), scope);
		break;

	case AST::Statement::Type::EXPRESSION: __COUNTER__;
	{
		mcf::IR::Expression::Pointer expressionObject = EvalExpression(static_cast<const mcf::AST::Statement::Expression*>(statement)->GetUnsafeExpression(), scope);
		if (expressionObject.get() == nullptr || expressionObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
		{
			return mcf::IR::Invalid::Make();
		}
		object = std::move(expressionObject);
		break;
	}

	case AST::Statement::Type::ASSIGN_EXPRESSION: __COUNTER__;
		object = EvalAssignExpressionStatement(static_cast<const mcf::AST::Statement::AssignExpression*>(statement), scope);
		break;

	case AST::Statement::Type::UNUSED: __COUNTER__;
		object = EvalUnusedStatement(static_cast<const mcf::AST::Statement::Unused*>(statement), scope);
		break;

	case AST::Statement::Type::WHILE: __COUNTER__;
		object = EvalWhileStatement(static_cast<const mcf::AST::Statement::While*>(statement), scope);
		break;

	case AST::Statement::Type::BREAK: __COUNTER__;
		object = EvalBreakStatement(static_cast<const mcf::AST::Statement::Break*>(statement), scope, isBreakAllowed);
		if (object.get() == nullptr || object->GetType() == mcf::IR::Type::INVALID)
		{
			return mcf::IR::Invalid::Make();
		}
		break;

	default:
		object = mcf::IR::Invalid::Make();
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. StatementType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Statement::CONVERT_TYPE_TO_STRING(statement->GetStatementType()), mcf::ENUM_INDEX(statement->GetStatementType()), statement->ConvertToString().c_str());
		break;
	}
	constexpr const size_t STATEMENT_TYPE_COUNT = __COUNTER__ - STATEMENT_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::AST::Statement::Type::COUNT) == STATEMENT_TYPE_COUNT, "statement type count is changed. this SWITCH need to be changed as well.");
	return std::move(object);
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalTypedefStatement(_Notnull_ const mcf::AST::Statement::Typedef* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	mcf::Object::Variable variable = EvalVariavbleSignatureIntermediate(statement->GetUnsafeSignaturePointer(), scope);
	mcf::Object::TypeInfo typeToDefine = variable.DataType;
	typeToDefine.Name = variable.Name;

	if (scope->DefineType(typeToDefine.Name, typeToDefine) == false)
	{
		MCF_DEBUG_TODO(u8"타입 정의에 실패 하였습니다.");
		return mcf::IR::Invalid::Make();
	}
	return mcf::IR::Typedef::Make(typeToDefine, variable.DataType);
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	const mcf::AST::Intermediate::FunctionSignature* signature = statement->GetUnsafeSignaturePointer();
	mcf::Object::FunctionInfo functionInfo = EvalFunctionSignatureIntermediate(signature, scope);
	scope->MakeLocalScopeToFunctionInfo(functionInfo);
	if (functionInfo.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

#if defined(_DEBUG)
	const size_t paramCount = functionInfo.Params.Variables.size();
	for (size_t i = 0; i < paramCount; ++i)
	{
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].IsValid(), u8"functionInfo.Params.Variables[%zu]가 유효하지 않습니다.", i );
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].DataType.IsValid(), u8"functionInfo.Params.Variables[%zu].DataType가 유효하지 않습니다.", i );
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].DataType.HasUnknownArrayIndex() == false, u8"unknown 배열이 있으면 안됩니다. functionInfo.Params.Variables[%zu].DataType", i);
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].DataType.IsVariadic || scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"변수 타입은 variadic 이거나 scope에 등록되어 있어야 합니다. functionInfo.Params.Variables[%zu].DataType", i);
	}
#endif

	MCF_DEBUG_ASSERT(functionInfo.Name.empty() == false, u8"");
	if (scope->DefineFunction(functionInfo.Name, functionInfo) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}
	return mcf::IR::Extern::Make(functionInfo.Name, functionInfo.Params.Variables);
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalLetStatement(_Notnull_ const mcf::AST::Statement::Let* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	mcf::Object::Variable variable = EvalVariavbleSignatureIntermediate(statement->GetUnsafeSignaturePointer(), scope);
	if (variable.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	mcf::Object::VariableInfo info = scope->DefineVariable(variable.Name, variable);
	if (info.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	const mcf::AST::Expression::Interface* expression = statement->GetUnsafeExpressionPointer();
	if (expression == nullptr)
	{
		return mcf::IR::Let::Make(info, nullptr);
	}

	mcf::IR::Expression::Pointer expressionObject = EvalExpression(statement->GetUnsafeExpressionPointer(), scope);
	if (expressionObject.get() == nullptr)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	if (info.Variable.DataType.HasUnknownArrayIndex())
	{
		DetermineUnknownArrayIndex(info.Variable, expressionObject.get(), scope);
	}

	if (ValidateVariableTypeAndValue(info.Variable, expressionObject.get()) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	return mcf::IR::Let::Make(info, std::move(expressionObject));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalUnusedStatement(_Notnull_ const mcf::AST::Statement::Unused* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	const size_t identifierCount = statement->GetIdentifiersCount();
	for (size_t i = 0; i < identifierCount; ++i)
	{
		const mcf::AST::Expression::Identifier* identifier = statement->GetUnsafeIdentifierPointerAt(i);
		MCF_DEBUG_ASSERT(identifier != nullptr, u8"identifier가 nullptr이면 안됩니다.");
		std::string tokenLiteral = identifier->GetTokenLiteral();
		if (scope->UseVariableInfo(tokenLiteral) == false)
		{
			MCF_DEBUG_TODO(u8"해당 이름의 변수를 찾을 수 없습니다.");
			return mcf::IR::Invalid::Make();
		}
	}
	return mcf::IR::Unused::Make();
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalReturnStatement(_Notnull_ const mcf::AST::Statement::Return* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	if (scope->IsGlobalScope())
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	if (scope->IsFunctionScope() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	const mcf::AST::Expression::Interface* returnExpression = statement->GetUnsafeReturnValueExpressionPointer();
	MCF_DEBUG_ASSERT(returnExpression != nullptr, u8"returnExpression가 nullptr이면 안됩니다.");
	mcf::IR::Expression::Pointer returnObject = EvalExpression(returnExpression, scope);
	if (returnObject.get() == nullptr || returnObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}
	return mcf::IR::Return::Make(std::move(returnObject));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalFuncStatement(_Notnull_ const mcf::AST::Statement::Func* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	const mcf::AST::Intermediate::FunctionSignature* signature = statement->GetUnsafeSignaturePointer();
	if (signature->GetName() == mcf::AST::Statement::Main::NAME)
	{
		MCF_DEBUG_TODO(u8"메인 함수는 func문이 아닌 main문으로 작성 해야 합니다.");
		return mcf::IR::Invalid::Make();
	}

	mcf::Object::FunctionInfo functionInfo = EvalFunctionSignatureIntermediate(signature, scope);
	scope->MakeLocalScopeToFunctionInfo(functionInfo);
	if (functionInfo.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	const size_t paramCount = functionInfo.Params.Variables.size();
	for (size_t i = 0; i < paramCount; ++i)
	{
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].IsValid(), u8"");
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].DataType.IsValid(), u8"");
		if (functionInfo.Params.Variables[i].DataType.IsVariadic)
		{
			continue;
		}

		MCF_DEBUG_ASSERT(scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");
	}

	if (scope->DefineFunction(functionInfo.Name, functionInfo) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	const mcf::AST::Statement::Block* functionBlock = statement->GetUnsafeBlockPointer();
	mcf::IR::ASM::PointerVector objects = EvalFunctionBlockStatement(functionInfo, functionBlock);
	if (objects.empty() == true)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	return mcf::IR::Func::Make(std::move(objects));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalMainStatement(_Notnull_ const mcf::AST::Statement::Main* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	MCF_DEBUG_ASSERT(mcf::AST::Statement::Main::NAME.empty() == false, u8"FunctionSignature에는 반드시 함수 이름이 있어야 합니다.");
	mcf::Object::FunctionInfo functionInfo = BuildFunctionInfo(mcf::AST::Statement::Main::NAME, statement->GetUnsafeFunctionParamsPointer(), statement->IsReturnVoid() ? nullptr : statement->GetUnsafeReturnTypePointer(), scope);
	scope->MakeLocalScopeToFunctionInfo(functionInfo);
	if (functionInfo.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	// TODO #35: 메인 함수에서는 특정 인자들만 받을 수 있도록 개선
	const size_t paramCount = functionInfo.Params.Variables.size();
	for (size_t i = 0; i < paramCount; ++i)
	{
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].IsValid(), u8"");
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].DataType.IsValid(), u8"");
		if (functionInfo.Params.Variables[i].DataType.IsVariadic)
		{
			continue;
		}

		MCF_DEBUG_ASSERT(scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");
	}

	if (scope->DefineFunction(functionInfo.Name, functionInfo) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	const mcf::AST::Statement::Block* functionBlock = statement->GetUnsafeBlockPointer();
	mcf::IR::ASM::PointerVector objects = EvalFunctionBlockStatement(functionInfo, functionBlock);
	if (objects.empty() == true)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	return mcf::IR::Func::Make(std::move(objects));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalAssignExpressionStatement(_Notnull_ const mcf::AST::Statement::AssignExpression* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	mcf::IR::Expression::Pointer leftObject = EvalExpression(statement->GetUnsafeLeftExpression(), scope);
	if (leftObject.get() == nullptr || leftObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	mcf::IR::Expression::Pointer rightObject = EvalExpression(statement->GetUnsafeRightExpression(), scope);
	if (rightObject.get() == nullptr || rightObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	return mcf::IR::Expression::Assign::Make(std::move(leftObject), std::move(rightObject));
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalWhileStatement(_Notnull_ const mcf::AST::Statement::While* statement, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	mcf::IR::Expression::Pointer conditionObject = EvalExpression(statement->GetUnsafeConditionPointer(), scope);
	if (conditionObject.get() == nullptr)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (conditionObject->GetExpressionType())
	{
	case IR::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		break;

	case IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::CALL: __COUNTER__;
		if (static_cast<const mcf::IR::Expression::Call*>(conditionObject.get())->GetInfo().IsReturnTypeVoid())
		{
			MCF_DEBUG_TODO(u8"리턴 타입이 없는 함수는 while문의 조건 표현식에서 호출될수 없습니다.");
			return mcf::IR::Invalid::Make();
		}
		break;

	case IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(conditionObject->GetExpressionType()), mcf::ENUM_INDEX(conditionObject->GetExpressionType()), conditionObject->Inspect().c_str());
		return mcf::IR::Invalid::Make();
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "validate whether the expression is conditional expression.");

	mcf::Object::Scope* blockScope = nullptr;
	if (scope->MakeLocalScope(&blockScope, false) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}

	mcf::IR::PointerVector blockObject = EvalBlockStatement(statement->GetUnsafeBlockPointer(), blockScope, true);
	return mcf::IR::While::Make(std::move(conditionObject), std::move(blockObject), blockScope);
}

mcf::IR::Pointer mcf::Evaluator::Object::EvalBreakStatement(_Notnull_ const mcf::AST::Statement::Break* statement, _Notnull_ mcf::Object::Scope* scope, const bool isBreakAllowed) noexcept
{
	MCF_UNUSED(statement, scope);

	if (isBreakAllowed == false)
	{
		MCF_DEBUG_TODO(u8"break문이 허용 되지 않는곳에 사용되었습니다.");
		return mcf::IR::Invalid::Make();
	}

	return mcf::IR::Break::Make();
}

mcf::IR::PointerVector mcf::Evaluator::Object::EvalBlockStatement(_Notnull_ const mcf::AST::Statement::Block* statement, _Notnull_ mcf::Object::Scope* scope, const bool isBreakAllowed) noexcept
{
	mcf::IR::PointerVector objects;
	const size_t statementCount = statement->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		mcf::IR::Pointer object = EvalStatement(statement->GetUnsafeStatementPointerAt(i), scope, isBreakAllowed);
		if (object.get() == nullptr || object->GetType() == mcf::IR::Type::INVALID)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return mcf::IR::PointerVector();
		}
		objects.emplace_back(std::move(object));
	}
	return objects;
}

mcf::IR::ASM::PointerVector mcf::Evaluator::Object::EvalFunctionBlockStatement(const mcf::Object::FunctionInfo& info, _Notnull_ const mcf::AST::Statement::Block* statement) noexcept
{
	mcf::IR::PointerVector objects;
	const size_t statementCount = statement->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		mcf::IR::Pointer object = EvalStatement(statement->GetUnsafeStatementPointerAt(i), info.LocalScope, false);
		if (object.get() == nullptr)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return mcf::IR::ASM::PointerVector();
		}
		objects.emplace_back(std::move(object));
	}

	mcf::Evaluator::FunctionIRGenerator generator(info);
	if (FunctionIRGenerator::ConvertStatements(generator, &objects, info.LocalScope) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::ASM::PointerVector();
	}

	return std::move(generator.GenerateIRCode());
}

mcf::Object::FunctionInfo mcf::Evaluator::Object::EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	MCF_DEBUG_ASSERT(intermediate->GetName().empty() == false, u8"FunctionSignature에는 반드시 함수 이름이 있어야 합니다.");
	return BuildFunctionInfo(intermediate->GetName(), intermediate->GetUnsafeFunctionParamsPointer(), intermediate->IsReturnTypeVoid() ? nullptr : intermediate->GetUnsafeReturnTypePointer(), scope);
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
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}

		if (outParams.Variables.back().DataType.HasUnknownArrayIndex())
		{
			MCF_DEBUG_TODO(u8"함수의 인자 값으로 unknown 배열이 들어오면 안됩니다.");
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

mcf::Object::Variable mcf::Evaluator::Object::EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::Object::Variable variable;
	variable.Name = intermediate->GetName();
	if (variable.Name.empty())
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::Object::Variable();
	}

	variable.DataType = EvalTypeSignatureIntermediate(intermediate->GetUnsafeTypeSignaturePointer(), scope);
	if (variable.DataType.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::Object::Variable();
	}

	variable.IsUsed = false;

	return variable;
}

mcf::Object::TypeInfo mcf::Evaluator::Object::EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::IR::Pointer object = EvalExpression(intermediate->GetUnsafeSignaturePointer(), scope);
	MCF_DEBUG_ASSERT(object->GetType() == mcf::IR::Type::EXPRESSION, u8"");
	mcf::IR::Expression::Interface* expressionObject = static_cast<mcf::IR::Expression::Interface*>(object.get());
	mcf::Object::TypeInfo typeInfo;
	switch (expressionObject->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER:
		typeInfo = static_cast<mcf::IR::Expression::TypeIdentifier*>(expressionObject)->GetInfo();
		break;

	default:
		MCF_DEBUG_TODO(u8"");
		break;
	}

	if (intermediate->IsUnsigned() && typeInfo.IsUnsigned)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::Object::TypeInfo();
	}
	typeInfo.IsUnsigned = intermediate->IsUnsigned() ? intermediate->IsUnsigned() : typeInfo.IsUnsigned;
	return typeInfo;
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
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
		object = EvalStringExpression(static_cast<const mcf::AST::Expression::String*>(expression), scope);
		break;

	case AST::Expression::Type::PREFIX: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case AST::Expression::Type::GROUP: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case AST::Expression::Type::INFIX: __COUNTER__;
		object = EvalInfixExpression(static_cast<const mcf::AST::Expression::Infix*>(expression), scope);
		break;

	case AST::Expression::Type::CALL: __COUNTER__;
		object = EvalCallExpression(static_cast<const mcf::AST::Expression::Call*>(expression), scope);
		break;

	case AST::Expression::Type::AS: __COUNTER__;
		object = EvalAsExpression(static_cast<const mcf::AST::Expression::As*>(expression), scope);
		if (object.get() == nullptr || object->GetExpressionType() != mcf::IR::Expression::Type::STATIC_CAST)
		{
			MCF_DEBUG_TODO(u8"정적 캐스팅에 실패 하였습니다.");
			break;
		}
		break;

	case AST::Expression::Type::INDEX: __COUNTER__;
		object = EvalIndexExpression(static_cast<const mcf::AST::Expression::Index*>(expression), scope);
		break;

	case AST::Expression::Type::INITIALIZER: __COUNTER__;
		object = EvalInitializerExpression(static_cast<const mcf::AST::Expression::Initializer*>(expression), scope);
		break;

	case AST::Expression::Type::MAP_INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->ConvertToString().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::AST::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");
	return std::move(object);
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	const std::string name = expression->GetTokenLiteral();
	if (scope->IsIdentifierRegistered(name) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Expression::Invalid::Make();
	}

	const mcf::Object::TypeInfo typeInfo = scope->FindTypeInfo(name);
	if (typeInfo.IsValid() == true)
	{
		return mcf::IR::Expression::TypeIdentifier::Make(typeInfo);
	}

	const mcf::Object::VariableInfo variableInfo = scope->FindVariableInfo(name);
	if (variableInfo.IsValid() == true)
	{
		MCF_EXECUTE_AND_DEBUG_ASSERT(scope->UseVariableInfo(variableInfo.Variable.Name), u8"변수가 존재해야 하는데 존재하지 않는 경우입니다. 존재하지 않는 원인을 찾아 해결해야 합니다.");
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

	MCF_DEBUG_TODO(u8"구현 필요");
	return mcf::IR::Expression::Invalid::Make();
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIntegerExpression(_Notnull_ const mcf::AST::Expression::Integer* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept
{
	MCF_UNUSED(scope);

	std::string stringValue = expression->GetTokenLiteral();

	if (Internal::IsStringConvertibleToInt64(stringValue) == true)
	{
		return mcf::IR::Expression::Integer::Make(std::stoll(stringValue));
	}

	if (Internal::IsStringConvertibleToUInt64(stringValue) == true)
	{
		return mcf::IR::Expression::Integer::Make(std::stoull(stringValue));
	}

	MCF_DEBUG_TODO(u8"구현 필요");
	return mcf::IR::Expression::Invalid::Make();
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalStringExpression(_Notnull_ const mcf::AST::Expression::String* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	// This is hard-coded because I assumed String TokenLiteral enclosing value by double quotations('"').
	const std::string stringLiteral = expression->GetTokenLiteral().substr(1, expression->GetTokenLiteral().size() - 2);
	mcf::Object::Data literalData = Internal::ConvertStringToData(stringLiteral);
	mcf::Object::ScopeTree* const scopeTree = scope->GetUnsafeScopeTreePointer();
	const auto emplacePairIter = scopeTree->LiteralIndexMap.try_emplace(expression->GetTokenLiteral(), std::make_pair(scopeTree->LiteralIndexMap.size(), literalData));
	return mcf::IR::Expression::String::Make(emplacePairIter.first->second.first, literalData.second.size());
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalCallExpression(_Notnull_ const mcf::AST::Expression::Call* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	const mcf::AST::Expression::Interface* leftExpression = expression->GetUnsafeLeftExpressionPointer();
	mcf::IR::Expression::Pointer leftObject = EvalExpression(leftExpression, scope);

	mcf::Object::FunctionInfo functionInfo;

	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (leftObject->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		functionInfo = static_cast<mcf::IR::Expression::FunctionIdentifier*>(leftObject.get())->GetInfo();
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(leftObject->GetExpressionType()), mcf::ENUM_INDEX(leftObject->GetExpressionType()), leftObject->Inspect().c_str());
		break;
	}
	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT = __COUNTER__ - LEFT_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == LEFT_EXPRESSION_TYPE_COUNT, "Find and return function identifier from expression object");

	if (functionInfo.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"함수 정보를 가져올수 없습니다.");
		return mcf::IR::Expression::Invalid::Make();
	}

	const size_t functionDefineParamCount = functionInfo.Params.HasVariadic() ? (functionInfo.Params.Variables.size() - 1) : functionInfo.Params.Variables.size();
	const size_t paramCount = expression->GetParamExpressionsCount();
	if (paramCount < functionDefineParamCount)
	{
		MCF_DEBUG_TODO(u8"함수 정보와 실제 함수 호출이 일치 하지 않습니다.");
		return mcf::IR::Expression::Invalid::Make();
	}

	mcf::IR::Expression::PointerVector paramObjects;
	if (functionInfo.Params.IsVoid() == false)
	{
		for (size_t i = 0; i < paramCount; i++)
		{
			paramObjects.emplace_back(EvalExpression(expression->GetUnsafeParamExpressionPointerAt(i), scope));
			if (paramObjects.back().get() == nullptr || paramObjects.back()->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
			{
				MCF_DEBUG_TODO(u8"함수 인자 정보가 잘못 되었습니다.");
				return mcf::IR::Expression::Invalid::Make();
			}

			if (i < functionDefineParamCount && ValidateVariableTypeAndValue(functionInfo.Params.Variables[i], paramObjects.back().get()) == false)
			{
				MCF_DEBUG_TODO(u8"함수에 정의된 인자 타입과 호출 인자의 타입이 일치하지 않습니다.");
				return mcf::IR::Expression::Invalid::Make();
			}
		}
	}

	return mcf::IR::Expression::Call::Make(functionInfo, std::move(paramObjects));
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalAsExpression(_Notnull_ const mcf::AST::Expression::As* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	const mcf::Object::TypeInfo targetType = EvalTypeSignatureIntermediate(expression->GetUnsafeTypeSignatureIntermediatePointer(), scope);
	if (targetType.IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"as 표현식 평가에 실패하였습니다.");
		return mcf::IR::Expression::Invalid::Make();
	}

	mcf::IR::Expression::Pointer leftObject = EvalExpression(expression->GetUnsafeLeftExpressionPointer(), scope);
	const mcf::Object::TypeInfo originalDataType = mcf::IR::Expression::Interface::GetDataTypeFromExpression(leftObject.get());
	if (originalDataType.IsStaticCastable(targetType) == false)
	{
		MCF_DEBUG_TODO(u8"정적 캐스팅이 불가능합니다. 현재 타입[%s] 캐스팅 타입[%s]", originalDataType.Inspect().c_str(), targetType.Inspect().c_str());
		return mcf::IR::Expression::Invalid::Make();
	}
	return mcf::IR::Expression::StaticCast::Make(std::move(leftObject), targetType);
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalIndexExpression(_Notnull_ const mcf::AST::Expression::Index* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	const mcf::AST::Expression::Interface* leftExpression = expression->GetUnsafeLeftExpressionPointer();
	const mcf::AST::Expression::Interface* indexExpression = expression->GetUnsafeIndexExpressionPointer();

	mcf::IR::Expression::Pointer leftObject = EvalExpression(leftExpression, scope);

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
		MCF_DEBUG_ASSERT(indexObject->GetType() == mcf::IR::Type::EXPRESSION, u8"구현 필요");

		mcf::Object::TypeInfo arrayTypeInfo = MakeArrayTypeInfo(info, indexObject.get());
		if (arrayTypeInfo.IsValid() == false)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return mcf::IR::Expression::Invalid::Make();
		}
		return mcf::IR::Expression::TypeIdentifier::Make(arrayTypeInfo);
	}

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
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

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(leftObject->GetExpressionType()), mcf::ENUM_INDEX(leftObject->GetExpressionType()), leftObject->Inspect().c_str());
		break;
	}
	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT = __COUNTER__ - LEFT_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == LEFT_EXPRESSION_TYPE_COUNT, "make index IR expression.");

	return mcf::IR::Expression::Invalid::Make();
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalInitializerExpression(_Notnull_ const mcf::AST::Expression::Initializer* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::IR::Expression::PointerVector keyVector;
	const size_t expressionCount = expression->GetKeyExpressionCount();
	for (size_t i = 0; i < expressionCount; i++)
	{
		keyVector.emplace_back(EvalExpression(expression->GetUnsafeKeyExpressionPointerAt(i), scope));
		if (keyVector.back().get() == nullptr || keyVector.back()->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return mcf::IR::Expression::Invalid::Make();
		}
	}
	return mcf::IR::Expression::Initializer::Make(std::move(keyVector));
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalInfixExpression(_Notnull_ const mcf::AST::Expression::Infix* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::IR::Expression::Pointer leftObject = EvalExpression(expression->GetUnsafeLeftExpressionPointer(), scope);
	if (leftObject.get() == nullptr || leftObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Expression::Invalid::Make();
	}

	mcf::IR::Expression::Pointer rightObject = EvalExpression(expression->GetUnsafeRightExpressionPointer(), scope);
	if (rightObject.get() == nullptr || rightObject->GetExpressionType() == mcf::IR::Expression::Type::INVALID)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Expression::Invalid::Make();
	}

	if (ValidateExpressionTypes(leftObject.get(), rightObject.get()) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Expression::Invalid::Make();
	}

	const mcf::Token::Data& token = expression->GetInfixOperator();
	if (mcf::IR::Expression::Conditional::IsValidTokenType(token.Type))
	{
		return mcf::IR::Expression::Conditional::Make(std::move(leftObject), token, std::move(rightObject));
	}

	if (mcf::IR::Expression::Arithmetic::IsValidTokenType(token.Type))
	{
		return mcf::IR::Expression::Arithmetic::Make(std::move(leftObject), token, std::move(rightObject));
	}

	MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(token.Type), mcf::ENUM_INDEX(token.Type), token.Literal.c_str());
	return mcf::IR::Expression::Invalid::Make();
}

mcf::Object::FunctionInfo mcf::Evaluator::Object::BuildFunctionInfo(const std::string& name, 
	_Notnull_ const mcf::AST::Intermediate::FunctionParams* functionParams, 
	const mcf::AST::Intermediate::TypeSignature* returnType, 
	_Notnull_ mcf::Object::Scope* scope) const noexcept
{
	mcf::Object::FunctionInfo functionInfo;
	functionInfo.Name = name;
	if (functionInfo.Name.empty())
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	const bool isParamsValid = EvalFunctionParamsIntermediate(functionInfo.Params, functionParams, scope);
	if (isParamsValid == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	if (returnType == nullptr)
	{
		return functionInfo;
	}

	functionInfo.ReturnType = EvalTypeSignatureIntermediate(returnType, scope);
	if (functionInfo.ReturnType.IsValid() == false || scope->FindTypeInfo(functionInfo.ReturnType.Name).IsValid() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::Object::FunctionInfo();
	}

	if (functionInfo.ReturnType.HasUnknownArrayIndex())
	{
		MCF_DEBUG_TODO(u8"함수의 반환 값으로 unknown 배열이 들어오면 안됩니다.");
		return mcf::Object::FunctionInfo();
	}

	return functionInfo;
}

mcf::Object::TypeInfo mcf::Evaluator::Object::MakeArrayTypeInfo(_In_ mcf::Object::TypeInfo info, _Notnull_ const mcf::IR::Expression::Interface* index) const noexcept
{
	MCF_DEBUG_ASSERT(info.IsValid(), u8"구현 필요");

	constexpr const size_t INDEX_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (index->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__; 
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; 
	{
		const mcf::IR::Expression::Integer* integerObject = static_cast<const mcf::IR::Expression::Integer*>(index);
		if (integerObject->IsNaturalInteger() == false)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return mcf::Object::TypeInfo();
		}
		info.ArraySizeList.emplace_back(integerObject->GetUInt64());
		break;
	}

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		info = mcf::Object::TypeInfo();
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(index->GetExpressionType()), mcf::ENUM_INDEX(index->GetExpressionType()), index->Inspect().c_str());
		break;
	}
	constexpr const size_t INDEX_EXPRESSION_TYPE_COUNT = __COUNTER__ - INDEX_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == INDEX_EXPRESSION_TYPE_COUNT, "make array type info.");

	return info;
}

void mcf::Evaluator::Object::DetermineUnknownArrayIndex(_Inout_ mcf::Object::Variable& variable, _Notnull_ const mcf::IR::Expression::Interface* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	MCF_DEBUG_ASSERT(variable.DataType.HasUnknownArrayIndex(), u8"unknown 배열 값을 가지고 있지 않으면 이곳에 들어오면 안됩니다.");
	scope->DetermineUnknownVariableTypeSize(variable.Name, CalculateMaximumArrayIndex(expression));
	variable = scope->FindVariableInfo(variable.Name).Variable;
}

const std::vector<size_t> mcf::Evaluator::Object::CalculateMaximumArrayIndex(_Notnull_ const mcf::IR::Expression::Interface* expression) const noexcept
{
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::GlobalVariableIdentifier* globalVariableIdentifier = static_cast<const mcf::IR::Expression::GlobalVariableIdentifier*>(expression);
		MCF_DEBUG_ASSERT(globalVariableIdentifier->GetVariable().DataType.HasUnknownArrayIndex(), u8"unknown 배열이 있으면 안됩니다.");
		return globalVariableIdentifier->GetVariable().DataType.ArraySizeList;
	}

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(expression);
		MCF_DEBUG_ASSERT(localVariableIdentifier->GetVariable().DataType.HasUnknownArrayIndex(), u8"unknown 배열이 있으면 안됩니다.");
		return localVariableIdentifier->GetVariable().DataType.ArraySizeList;
	}

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		return std::vector<size_t>();

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
	{
		const mcf::IR::Expression::String* stringExpression = static_cast<const mcf::IR::Expression::String*>(expression);
		return std::vector<size_t>({ stringExpression->GetSize() });
	}

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
	{
		std::vector<size_t> result;
		const mcf::IR::Expression::Initializer* initializer = static_cast<const mcf::IR::Expression::Initializer*>(expression);
		const size_t initExpressionCount = initializer->GetKeyExpressionCount();
		for (size_t i = 0; i < initExpressionCount; i++)
		{
			std::vector<size_t> tempResult = CalculateMaximumArrayIndex(initializer->GetUnsafeKeyExpressionPointerAt(i));
			if (tempResult.empty())
			{
				break;
			}

			if (result.empty())
			{
				result = tempResult;
				continue;
			}

			if (result.back() < tempResult.back())
			{
				result = tempResult;
				continue;
			}
		}
		result.emplace_back(initExpressionCount);
		return result;
	}

	case mcf::IR::Expression::Type::MAP_INITIALIZER: __COUNTER__;
	{
		MCF_DEBUG_TODO(u8"다시 확인 필요");
		std::vector<size_t> result;
		const mcf::IR::Expression::MapInitializer* mapInitializer = static_cast<const mcf::IR::Expression::MapInitializer*>(expression);
		const size_t initExpressionCount = mapInitializer->GetKeyExpressionCount();
		MCF_DEBUG_ASSERT(initExpressionCount == mapInitializer->GetValueExpressionCount(), u8"key와 value의 갯수가 일치하지 않습니다.");
		for (size_t i = 0; i < initExpressionCount; i++)
		{
			std::vector<size_t> tempResult = CalculateMaximumArrayIndex(mapInitializer->GetUnsafeKeyExpressionPointerAt(i));
			if (tempResult.empty())
			{
				break;
			}

			if (result.empty())
			{
				result = tempResult;
				continue;
			}

			if (result.back() < tempResult.back())
			{
				result = tempResult;
				continue;
			}
		}
		result.emplace_back(initExpressionCount);
		return result;
	}

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
	{
		const mcf::IR::Expression::Call* callExpression = static_cast<const mcf::IR::Expression::Call*>(expression);
		MCF_DEBUG_ASSERT(callExpression->GetInfo().ReturnType.HasUnknownArrayIndex(), u8"unknown 배열이 있으면 안됩니다.");
		return callExpression->GetInfo().ReturnType.ArraySizeList;
	}

	case mcf::IR::Expression::Type::STATIC_CAST: __COUNTER__;
	{
		const mcf::IR::Expression::StaticCast* staticCast = static_cast<const mcf::IR::Expression::StaticCast*>(expression);
		MCF_DEBUG_ASSERT(staticCast->GetCastedDatType().HasUnknownArrayIndex(), u8"unknown 배열이 있으면 안됩니다.");
		return staticCast->GetCastedDatType().ArraySizeList;
	}

	case mcf::IR::Expression::Type::CONDITIONAL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::ARITHMETIC: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()), expression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "calculate maximum array index.");
	return std::vector<size_t>();
}
