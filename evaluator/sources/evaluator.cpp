#include "pch.h"
#include "evaluator.h"

namespace mcf
{
	namespace Evaluator
	{
		namespace Internal
		{
			constexpr mcf::IR::ASM::Register FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[] =
			{
				mcf::IR::ASM::Register::RCX,
				mcf::IR::ASM::Register::RDX,
				mcf::IR::ASM::Register::R8,
				mcf::IR::ASM::Register::R9,
			};

			inline static const bool IsStringConvertibleToInt64(const std::string& stringValue) noexcept
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

			inline static const bool IsStringConvertibleToUInt64(const std::string& stringValue) noexcept
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
				MCF_DEBUG_ASSERT(size > 0, u8"사이즈는 0일 수 없습니다.");
				MCF_DEBUG_ASSERT(size < alignment, u8"사이즈는 alignment 보다 크면 안됩니다.");
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
				outPaddings.emplace_back(alignment - ((estimatedOffset + size) % alignment));
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
		_localMemory.AddSize(_info.Params.Variables[i].GetTypeSize());
	}
	_reservedMemory = _localMemory.GetTotalSize() < MemoryAllocator::DEFAULT_ALIGNMENT ? MemoryAllocator::DEFAULT_ALIGNMENT : _localMemory.GetTotalSize();
}

void mcf::Evaluator::FunctionCallIRGenerator::AddParameter(_Notnull_ const mcf::IR::Expression::String* stringExpression) noexcept
{
	AddParameterInternal(mcf::IR::ASM::UnsafePointerAddress(stringExpression));
}

void mcf::Evaluator::FunctionCallIRGenerator::AddParameter(const mcf::IR::ASM::UnsafePointerAddress& source) noexcept
{
	AddParameterInternal(mcf::IR::ASM::UnsafePointerAddress(source, _reservedMemory));
}

void mcf::Evaluator::FunctionCallIRGenerator::AddParameter(const mcf::IR::ASM::SizeOf& source) noexcept
{
	if (_currParamIndex < MCF_ARRAY_SIZE(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS))
	{
		_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[_currParamIndex++], source));
	}
	else
	{
		MCF_DEBUG_TODO(u8"구현 필요");
	}
}

void mcf::Evaluator::FunctionCallIRGenerator::AddGeneratedIRCode(_Out_ mcf::IR::ASM::PointerVector& outVector) noexcept
{
	outVector.emplace_back(mcf::IR::ASM::Sub::Make(mcf::IR::ASM::Register::RSP, static_cast<unsigned __int64>(_reservedMemory)));
	
	const size_t localCodeCount = _localCodes.size();
	for (size_t i = 0; i < localCodeCount; ++i)
	{
		outVector.emplace_back(std::move(_localCodes[localCodeCount - 1 - i]));
	}
	outVector.emplace_back(mcf::IR::ASM::Call::Make(_info.Name));

	outVector.emplace_back(mcf::IR::ASM::Add::Make(mcf::IR::ASM::Register::RSP, static_cast<unsigned __int64>(_reservedMemory)));

	_localCodes.clear();
	_localMemory.Clear();
	_currParamIndex = 0;
}

void mcf::Evaluator::FunctionCallIRGenerator::AddParameterInternal(const mcf::IR::ASM::UnsafePointerAddress& source) noexcept
{
	if (_currParamIndex < MCF_ARRAY_SIZE(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS))
	{
		_localCodes.emplace_back(mcf::IR::ASM::Lea::Make(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[_currParamIndex++], source));
	}
	else
	{
		MCF_DEBUG_TODO(u8"구현 필요");
	}
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

mcf::Evaluator::FunctionIRGenerator::FunctionIRGenerator(const mcf::Object::FunctionInfo& info) noexcept
	: _returnType(info.ReturnType)
{
	_beginCodes.emplace_back(mcf::IR::ASM::ProcBegin::Make(info.Name));
	_beginCodes.emplace_back(mcf::IR::ASM::Push::Make(mcf::IR::ASM::Register::RBP));
	_endCodes.emplace_back(mcf::IR::ASM::ProcEnd::Make(info.Name));
	_endCodes.emplace_back(mcf::IR::ASM::Ret::Make());
	_endCodes.emplace_back(mcf::IR::ASM::Pop::Make(mcf::IR::ASM::Register::RBP));

	const size_t paramCount = info.Params.Variables.size();
	const mcf::Object::TypeInfo paramType = mcf::Object::TypeInfo::MakePrimitive(true, "qword", sizeof(unsigned __int64));
	MCF_DEBUG_ASSERT(paramType.IsValid(), u8"");
	for (size_t i = 0; i < paramCount && i < MCF_ARRAY_SIZE(Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS); ++i)
	{
		const size_t offset = PARAM_OFFSET + i * 0x8;
		mcf::IR::ASM::Address target(paramType, mcf::IR::ASM::Register::RSP, offset);
		_paramOffsetMap[info.Params.Variables[i].Name] = offset;
		_beginCodes.emplace_back(mcf::IR::ASM::Mov::Make(target, Internal::FIRST_FOUR_FUNCTION_PARAM_TARGET_REGISTERS[i]));
	}
}

void mcf::Evaluator::FunctionIRGenerator::AddLetStatement(_Notnull_ const mcf::IR::Let* object, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	mcf::Object::VariableInfo targetVariableInfo = object->GetInfo();
	MCF_DEBUG_ASSERT(targetVariableInfo.IsValid(), u8"variableInfo가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(targetVariableInfo.IsGlobal == false, u8"함수안의 변수는 로컬이어야 하는데 글로벌로 등록이 되어 있습니다.");
	mcf::Object::Variable targetVariable = targetVariableInfo.Variable;
	MCF_DEBUG_ASSERT(targetVariable.IsVariadic() == false, u8"변수 선언시 variadic은 불가능 합니다.");
	const size_t targetVariableIndex = _localMemory.GetCount();
	_localVariableIndicesMap[targetVariable.Name] = _localMemory.GetCount();

	const mcf::IR::Expression::Interface* assignExpression = object->GetUnsafeAssignExpressionPointer();
	MCF_DEBUG_ASSERT(assignExpression != nullptr, u8"assignExpression가 nullptr이면 안됩니다.");
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (assignExpression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(assignExpression);
		if (targetVariable.DataType != localVariableIdentifier->GetInfo().DataType)
		{
			MCF_DEBUG_TODO(u8"타입이 일치 하지 않습니다.");
			return;
		}

		if (targetVariable.DataType.IsStringCompatibleType() || targetVariable.DataType.IsArrayType() == true)
		{
			MCF_DEBUG_TODO(u8"배열 타입 구현 필요");
			return;
		}

		if (targetVariable.DataType.IsStruct == true)
		{
			MCF_DEBUG_TODO(u8"구조체 타입 구현 필요");
			return;
		}

		const size_t targetVariableSize = targetVariable.GetTypeSize();
		const size_t lastTargetVariableArrayIndex = targetVariable.DataType.ArraySizeList.size() - 1;
		const bool isTargetVariableSizeUnknown = targetVariableSize == 0 ? targetVariable.DataType.IsArraySizeUnknown(lastTargetVariableArrayIndex) : false;
		if (isTargetVariableSizeUnknown)
		{
			_localMemory.AddSize(localVariableIdentifier->GetInfo().GetTypeSize());
		}
		else
		{
			_localMemory.AddSize(targetVariable.GetTypeSize());
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

		const size_t targetTypeSize = targetVariable.DataType.GetSize();
		const size_t targetTypeBytes = targetTypeSize / sizeof(__int8);
		MCF_DEBUG_ASSERT(targetTypeSize % sizeof(__int8) == 0 && sizeof(__int8) <= targetTypeSize && targetTypeBytes <= sizeof(__int64), u8"유효하지 않은 데이터 크기가 들어왔습니다.");
		if (registers[targetTypeBytes] != mcf::IR::ASM::Register::INVALID)
		{
			const std::string sourceVariableName = localVariableIdentifier->GetInfo().Name;
			size_t sourceOffset, targetOffset;

			auto sourceParamOffsetPairIter = _paramOffsetMap.find(sourceVariableName);
			if (sourceParamOffsetPairIter != _paramOffsetMap.end())
			{
				sourceOffset = _localMemory.GetTotalSize() + sourceParamOffsetPairIter->second;
			}
			else
			{
				auto localVariableIndexPairIter = _localVariableIndicesMap.find(sourceVariableName);
				MCF_DEBUG_ASSERT(localVariableIndexPairIter != _localVariableIndicesMap.end(), u8"해당 이름의 지역변수를 찾을 수 없습니다. Name=`%s`", sourceVariableName.c_str());
				sourceOffset = _localMemory.GetOffset(localVariableIndexPairIter->second);
			}
			const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetInfo().DataType, mcf::IR::ASM::Register::RSP, sourceOffset);
			_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(registers[targetTypeBytes], sourceAddress));

			auto targetParamOffsetPairIter = _paramOffsetMap.find(targetVariable.Name);
			if (targetParamOffsetPairIter != _paramOffsetMap.end())
			{
				targetOffset = _localMemory.GetTotalSize() + targetParamOffsetPairIter->second;
			}
			else
			{
				auto localVariableIndexPairIter = _localVariableIndicesMap.find(targetVariable.Name);
				MCF_DEBUG_ASSERT(localVariableIndexPairIter != _localVariableIndicesMap.end(), u8"해당 이름의 지역변수를 찾을 수 없습니다. Name=`%s`", targetVariable.Name.c_str());
				targetOffset = _localMemory.GetOffset(localVariableIndexPairIter->second);
			}
			const mcf::IR::ASM::Address targetAddress(targetVariable.DataType, mcf::IR::ASM::Register::RSP, targetOffset);
			_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(targetAddress, registers[targetTypeBytes]));
			break;
		}

		MCF_DEBUG_TODO(u8"기본 타입중에 처리할 수 있는 크기가 아닙니다.");
		break;
	}
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
	{
		_localMemory.AddSize(targetVariable.GetTypeSize());

		if (targetVariable.DataType.IsIntegerType() == false)
		{
			MCF_DEBUG_TODO(u8"정수 타입이 아닌 값에 정수를 대입하려 하고 있습니다. 타입[%s]", targetVariable.DataType.Inspect().c_str());
			return;
		}

		const mcf::IR::Expression::Integer* integerExpression = static_cast<const mcf::IR::Expression::Integer*>(assignExpression);
		if (integerExpression->IsCompatible(targetVariable.DataType) == false)
		{
			MCF_DEBUG_TODO(u8"정수 값과 호환 불가능한 타입입니다. 타입[%s]", targetVariable.DataType.Inspect().c_str());
			return;
		}

		const mcf::IR::ASM::Address target(targetVariable.DataType, mcf::IR::ASM::Register::RSP, _localMemory.GetOffset(targetVariableIndex));
		switch (targetVariable.DataType.GetSize())
		{
		case sizeof(__int8):
			_localCodes.emplace_back(targetVariable.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt8()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt8()));
			break;

		case sizeof(__int16):
			_localCodes.emplace_back(targetVariable.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt16()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt16()));
			break;

		case sizeof(__int32):
			_localCodes.emplace_back(targetVariable.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt32()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt32()));
			break;

		case sizeof(__int64) :
			_localCodes.emplace_back(targetVariable.DataType.IsUnsigned ? mcf::IR::ASM::Mov::Make(target, integerExpression->GetUInt64()) : mcf::IR::ASM::Mov::Make(target, integerExpression->GetInt64()));
			break;

		default:
			MCF_DEBUG_BREAK(u8"integerExpression->IsCompatible(variable.DataType)에서 처리되어야 합니다.");
			break;
		}
		break;
	}

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
	{
		if (targetVariable.DataType.IsStringCompatibleType() == false)
		{
			MCF_DEBUG_TODO(u8"문자열 호환 가능한 타입(바이트 배열 타입)이 아닌 변수에 문자열을 대입하려 하고 있습니다. 타입[%s]", targetVariable.DataType.Inspect().c_str());
			return;
		}

		const mcf::IR::Expression::String* stringExpression = static_cast<const mcf::IR::Expression::String*>(assignExpression);
		const size_t targetVariableSize = targetVariable.GetTypeSize();
		const size_t lastTargetVariableArrayIndex = targetVariable.DataType.ArraySizeList.size() - 1;
		const bool isTargetVariableSizeUnknown = targetVariableSize == 0 ? targetVariable.DataType.IsArraySizeUnknown(lastTargetVariableArrayIndex) : false;
		if (isTargetVariableSizeUnknown)
		{
			_localMemory.AddSize(stringExpression->GetSize());
			scope->DetermineUnknownVariableTypeSize(targetVariable.Name, lastTargetVariableArrayIndex, stringExpression->GetSize());
		}
		else
		{
			_localMemory.AddSize(targetVariable.GetTypeSize());
		}

		const mcf::IR::ASM::UnsafePointerAddress targetAddress(mcf::IR::ASM::Register::RSP, _localMemory.GetOffset(targetVariableIndex));
		FunctionCallIRGenerator generator(scope->FindInternalFunction(mcf::Object::InternalFunctionType::COPY_MEMORY));
		generator.AddParameter(stringExpression);
		generator.AddParameter(targetAddress);
		generator.AddParameter(mcf::IR::ASM::SizeOf(stringExpression));
		generator.AddGeneratedIRCode(_localCodes);
		break;
	}

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요.");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(assignExpression->GetExpressionType()), mcf::ENUM_INDEX(assignExpression->GetExpressionType()), assignExpression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

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
		if (_returnType != localVariableIdentifier->GetInfo().DataType)
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
			const std::string varName = localVariableIdentifier->GetInfo().Name;
			auto paramOffsetPairIter = _paramOffsetMap.find(varName);
			if (paramOffsetPairIter != _paramOffsetMap.end())
			{
				const size_t offset = _localMemory.GetTotalSize() + paramOffsetPairIter->second;
				const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetInfo().DataType, mcf::IR::ASM::Register::RSP, offset);
				_localCodes.emplace_back(mcf::IR::ASM::Mov::Make(returnRegisters[returnTypeBytes], sourceAddress ));
			}
			else
			{
				auto localVariableIndexPairIter = _localVariableIndicesMap.find(varName);
				MCF_DEBUG_ASSERT(localVariableIndexPairIter != _localVariableIndicesMap.end(), u8"해당 이름의 지역변수를 찾을 수 없습니다. Name=`%s`", varName.c_str());
				const size_t offset = _localMemory.GetOffset(localVariableIndexPairIter->second);
				const mcf::IR::ASM::Address sourceAddress(localVariableIdentifier->GetInfo().DataType, mcf::IR::ASM::Register::RSP, offset);
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
			_localCodes.emplace_back(_returnType.IsUnsigned ? mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AL, integerExpression->GetUInt8()) : mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AL, integerExpression->GetInt8()));
			break;

		case sizeof(__int16):
			_localCodes.emplace_back(_returnType.IsUnsigned ? mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AX, integerExpression->GetUInt16()) : mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::AX, integerExpression->GetInt16()));
			break;

		case sizeof(__int32):
			_localCodes.emplace_back(_returnType.IsUnsigned ? mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::EAX, integerExpression->GetUInt32()) : mcf::IR::ASM::Mov::Make(mcf::IR::ASM::Register::EAX, integerExpression->GetInt32()));
			break;

		case sizeof(__int64) :
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

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO( u8"구현 필요." );
		break;

	

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING( retrunExpression->GetExpressionType()), mcf::ENUM_INDEX( retrunExpression->GetExpressionType()), retrunExpression->Inspect().c_str());
		break;
	}
	constexpr const size_t EXPRESSION_TYPE_COUNT = __COUNTER__ - EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");
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

const std::string mcf::Evaluator::FunctionIRGenerator::INTERNAL_COPY_MEMORY_NAME = "?CopyMemory";

mcf::IR::Pointer mcf::Evaluator::Object::Eval(_Notnull_ const mcf::AST::Node::Interface* node, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	MCF_DEBUG_ASSERT(node != nullptr, u8"node가 nullptr이면 안됩니다.");

	mcf::IR::Pointer object;
	switch (node->GetNodeType())
	{
	case mcf::AST::Node::Type::EXPRESSION:
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::AST::Node::Type::INTERMEDIATE:
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::AST::Node::Type::STATEMENT:
		object = EvalStatement(static_cast<const mcf::AST::Statement::Interface*>(node), scope);
		break;

	case mcf::AST::Node::Type::PROGRAM:
		object = EvalProgram(static_cast<const mcf::AST::Program*>(node), scope);
		break;

	default:
		MCF_DEBUG_TODO("");
		break;
	}
	return std::move(object);
}
		
mcf::IR::Pointer mcf::Evaluator::Object::EvalProgram(_Notnull_ const mcf::AST::Program* program, _Notnull_ mcf::Object::Scope* scope) noexcept
{
	MCF_DEBUG_ASSERT(program != nullptr, u8"program가 nullptr이면 안됩니다.");

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
	MCF_DEBUG_ASSERT(statement != nullptr, u8"statement가 nullptr이면 안됩니다.");

	mcf::IR::Pointer object = mcf::IR::Invalid::Make();
	constexpr const size_t STATEMENT_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (statement->GetStatementType())
	{
	case AST::Statement::Type::INCLUDE_LIBRARY: __COUNTER__;
		object = mcf::IR::IncludeLib::Make(static_cast<const mcf::AST::Statement::IncludeLibrary*>(statement)->GetLibPath());
		break;

	case AST::Statement::Type::TYPEDEF: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
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
			MCF_DEBUG_TODO(u8"구현 필요");
			break;
		}
		object = std::move(expressionObject);
		break;
	}

	case AST::Statement::Type::UNUSED: __COUNTER__;
		object = EvalUnusedStatement(static_cast<const mcf::AST::Statement::Unused*>(statement), scope);
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

	const size_t paramCount = functionInfo.Params.Variables.size();
	std::vector<mcf::Object::TypeInfo> params;
	for (size_t i = 0; i < paramCount; ++i)
	{
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].IsValid(), u8"");
		MCF_DEBUG_ASSERT(functionInfo.Params.Variables[i].DataType.IsValid(), u8"");

		if (functionInfo.Params.Variables[i].DataType.IsVariadic)
		{
			continue;
		}

		MCF_DEBUG_ASSERT(scope->FindTypeInfo(functionInfo.Params.Variables[i].DataType.Name).IsValid(), u8"");
		params.emplace_back(functionInfo.Params.Variables[i].DataType);
	}

	MCF_DEBUG_ASSERT(functionInfo.Name.empty() == false, u8"");
	if (scope->DefineFunction(functionInfo.Name, functionInfo) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::Invalid::Make();
	}
	return mcf::IR::Extern::Make(functionInfo.Name, params, functionInfo.Params.HasVariadic());
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

	if (ValidateVariableTypeAndValue(info, expressionObject.get()) == false)
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

	// TODO : 메인 함수에서는 특정 인자들만 받을 수 있도록 개선
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

mcf::IR::ASM::PointerVector mcf::Evaluator::Object::EvalFunctionBlockStatement(const mcf::Object::FunctionInfo& info, _Notnull_ const mcf::AST::Statement::Block* statement) noexcept
{
	mcf::Evaluator::FunctionIRGenerator generator(info);
	
	bool hasReturn = false;
	mcf::IR::ASM::PointerVector objects;
	const size_t statementCount = statement->GetStatementCount();
	for (size_t i = 0; i < statementCount; i++)
	{
		if (hasReturn == true)
		{
			MCF_DEBUG_ASSERT(info.IsReturnTypeVoid() == false, u8"리턴문이 들어왔을때 함수의 리턴타입이 있는지 확인을 했어야 합니다.");
			MCF_DEBUG_TODO(u8"리턴문 이후에 다른 명령문은 들어올 수 없습니다.");
			return mcf::IR::ASM::PointerVector();
		}

		mcf::IR::Pointer object = EvalStatement(statement->GetUnsafeStatementPointerAt(i), info.Definition.LocalScope);
		if (object.get() == nullptr)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return mcf::IR::ASM::PointerVector();
		}

		constexpr const size_t IR_TYPE_COUNT_BEGIN = __COUNTER__;
		switch (object->GetType())
		{
		case IR::Type::LET: __COUNTER__;
			generator.AddLetStatement(static_cast<mcf::IR::Let*>(object.get()), info.Definition.LocalScope);
			break;

		case IR::Type::EXPRESSION: __COUNTER__;
			MCF_DEBUG_TODO(u8"구현 필요");
			break;

		case IR::Type::UNUSEDIR: __COUNTER__;
			break;

		case IR::Type::RETURN: __COUNTER__;
			if (info.IsReturnTypeVoid() == true)
			{
				MCF_DEBUG_TODO(u8"리턴 타입이 없는 함수에는 리턴이 들어올 수 없습니다.");
				return mcf::IR::ASM::PointerVector();
			}
			generator.AddReturnStatement(static_cast<mcf::IR::Return*>(object.get()));
			hasReturn = true;
			break;

		case IR::Type::ASM: __COUNTER__; [[fallthrough]];
		case IR::Type::INCLUDELIB: __COUNTER__; [[fallthrough]];
		case IR::Type::EXTERN: __COUNTER__; [[fallthrough]];
		case IR::Type::FUNC: __COUNTER__; [[fallthrough]];
		case IR::Type::PROGRAM: __COUNTER__; [[fallthrough]];
		default:
			objects.clear();
			MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. IRType=%s(%zu) ConvertedString=`%s`",
				mcf::IR::CONVERT_TYPE_TO_STRING(object->GetType()), mcf::ENUM_INDEX(object->GetType()), object->Inspect().c_str());
			break;
		}
		constexpr const size_t IR_TYPE_COUNT = __COUNTER__ - IR_TYPE_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::IR::Type::COUNT) == IR_TYPE_COUNT, "IR type count is changed. this SWITCH need to be changed as well.");
	}

	if (info.IsReturnTypeVoid() == hasReturn)
	{
		MCF_DEBUG_ASSERT(info.IsReturnTypeVoid() == false, u8"리턴문이 들어왔을때 함수의 리턴타입이 있는지 확인을 했어야 합니다.");
		MCF_DEBUG_TODO(u8"리턴문이 들어오지 않았습니다.");
		return mcf::IR::ASM::PointerVector();
	}

	if (info.Definition.LocalScope->IsAllVariablesUsed() == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::IR::ASM::PointerVector();
	}

	objects = generator.GenerateIRCode();
	return std::move(objects);
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
	MCF_DEBUG_ASSERT(expression != nullptr, u8"expression가 nullptr이면 안됩니다.");
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
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case AST::Expression::Type::CALL: __COUNTER__;
		object = EvalCallExpression(static_cast<const mcf::AST::Expression::Call*>(expression), scope);
		MCF_DEBUG_TODO(u8"구현 필요");
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
	static_assert(static_cast<size_t>(mcf::AST::Statement::Type::COUNT) == EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");
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
	return mcf::IR::Expression::Pointer();
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
	mcf::Object::ScopeTree* const scopeTree = scope->GetUnsafeScopeTreePointer();
	const auto emplacePairIter = scopeTree->LiteralIndexMap.try_emplace(stringLiteral, scopeTree->LiteralIndexMap.size());
	return mcf::IR::Expression::String::Make(emplacePairIter.first->second, stringLiteral.size());
}

mcf::IR::Expression::Pointer mcf::Evaluator::Object::EvalCallExpression(_Notnull_ const mcf::AST::Expression::Call* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept
{
	const mcf::AST::Expression::Interface* leftExpression = expression->GetUnsafeLeftExpressionPointer();
	mcf::IR::Expression::Pointer leftObject = EvalExpression(leftExpression, scope);

	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (leftObject->GetExpressionType())
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

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Expression::CONVERT_TYPE_TO_STRING(leftExpression->GetExpressionType()), mcf::ENUM_INDEX(leftExpression->GetExpressionType()), leftExpression->ConvertToString().c_str());
		break;
	}
	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT = __COUNTER__ - LEFT_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == LEFT_EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

	MCF_DEBUG_TODO(u8"구현 필요");
	return mcf::IR::Expression::Invalid::Make();
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

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) ConvertedString=`%s`",
			mcf::AST::Expression::CONVERT_TYPE_TO_STRING(leftExpression->GetExpressionType()), mcf::ENUM_INDEX(leftExpression->GetExpressionType()), leftExpression->ConvertToString().c_str());
		break;
	}
	constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT = __COUNTER__ - LEFT_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == LEFT_EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");

	return mcf::IR::Expression::Pointer();
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

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];
	default:
		info = mcf::Object::TypeInfo();
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(index->GetExpressionType()), mcf::ENUM_INDEX(index->GetExpressionType()), index->Inspect().c_str());
		break;
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
	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::GlobalVariableIdentifier* globalVariableIdentifier = static_cast<const mcf::IR::Expression::GlobalVariableIdentifier*>(value);
		if (info.Variable.DataType != globalVariableIdentifier->GetInfo().DataType)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		break;
	}

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
	{
		const mcf::IR::Expression::LocalVariableIdentifier* localVariableIdentifier = static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(value);
		if (info.Variable.DataType != localVariableIdentifier->GetInfo().DataType)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
			return false;
		}
		break;
	}


	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		return static_cast<const mcf::IR::Expression::Integer*>(value)->IsCompatible(info.Variable.DataType);

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		return info.Variable.DataType.IsStringCompatibleType();

	case mcf::IR::Expression::Type::INITIALIZER: __COUNTER__;
	{
		if (info.Variable.DataType.IsArrayType() == false)
		{
			MCF_DEBUG_TODO(u8"구현 필요");
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
				MCF_DEBUG_TODO(u8"구현 필요");
				return false;
			}
		}
		break;
	}

	case mcf::IR::Expression::Type::CALL: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__; [[fallthrough]];
	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__; [[fallthrough]];
	default:
		MCF_DEBUG_TODO(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. ExpressionType=%s(%zu) Inspect=`%s`",
			mcf::IR::Expression::CONVERT_TYPE_TO_STRING(value->GetExpressionType()), mcf::ENUM_INDEX(value->GetExpressionType()), value->Inspect().c_str());
		break;
	}
	constexpr const size_t ASSIGNED_EXPRESSION_TYPE_COUNT = __COUNTER__ - ASSIGNED_EXPRESSION_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == ASSIGNED_EXPRESSION_TYPE_COUNT, "expression type count is changed. this SWITCH need to be changed as well.");
	return true;
}
