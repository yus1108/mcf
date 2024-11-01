﻿#include "pch.h"
#include "object.h"

namespace mcf
{
	namespace IR
	{
		namespace ASM
		{
			namespace Internal
			{
				static const bool IsSizeMatching(const mcf::IR::ASM::Register lhs, const mcf::IR::ASM::Register rhs)
				{
					return GET_REGISTER_SIZE_VALUE(lhs) == GET_REGISTER_SIZE_VALUE(rhs);
				}

				static const bool IsSizeMatching(const mcf::IR::ASM::Register reg, const size_t size)
				{
					return GET_REGISTER_SIZE_VALUE(reg) == size;
				}

				const std::string GetAddressOf(const std::string& identifier) noexcept
				{
					return std::string( "[" ) + identifier + "]";
				}

				const std::string GetAddressOf(const std::string& identifier, const size_t offset) noexcept
				{
					return std::string( "[" ) + identifier + std::string( offset < 0 ? " - " : " + " ) + std::to_string( offset ) + "]";
				}

				const std::string GetAddressOf(const Register value, const size_t offset) noexcept
				{
					return std::string("[") + CONVERT_REGISTER_TO_STRING(value) + std::string(offset < 0 ? " - " : " + ") + std::to_string(offset) + "]";
				}
			}
		}
	}
}

const bool mcf::Object::TypeInfo::IsStaticCastable(const TypeInfo& typeToCast) const noexcept
{
	MCF_DEBUG_ASSERT(IsValid() && typeToCast.IsValid(), u8"두 타입 모드 유효해야 합니다.");
	// 배열(문자열 포함) 타입은 주소값을 담을수 있는 정수 타입으로 캐스팅 가능합니다.
	if (IsArrayType() && typeToCast.IsCompatibleAddressType())
	{
		return true;
	}

	if (IntrinsicSize != typeToCast.IntrinsicSize)
	{
		return false;
	}

	if (GetSize() != typeToCast.GetSize())
	{
		return false;
	}

	if (IsArrayType() && typeToCast.IsArrayType() && 
		ArraySizeList.size() == typeToCast.ArraySizeList.size() &&
		HasUnknownArrayIndex() == false && typeToCast.HasUnknownArrayIndex())
	{
		return true;
	}

	if (IsIntegerType() && typeToCast.IsIntegerType())
	{
		return true;
	}

	return false;
}

const bool mcf::Object::TypeInfo::HasUnknownArrayIndex(void) const noexcept
{
	const size_t size = ArraySizeList.size();
	for (size_t i = 0; i < size; i++)
	{
		if (ArraySizeList[i] == 0)
		{
			return true;
		}
	}
	return false;
}

const size_t mcf::Object::TypeInfo::GetSize(void) const noexcept
{
	MCF_DEBUG_ASSERT(IsVariadic == false, u8"variadic 은 사이즈를 알 수 없습니다.");
	size_t totalSize = IntrinsicSize;
	const size_t vectorCount = ArraySizeList.size();
	for (size_t i = 0; i < vectorCount; ++i)
	{
		totalSize *= ArraySizeList[i];
	}
	return totalSize;
}

const std::string mcf::Object::TypeInfo::Inspect(void) const noexcept
{
	MCF_DEBUG_ASSERT(IsValid(), u8"TypeInfo가 유효하지 않습니다.");
	return IsVariadic ? "VARARG" : Name;
}

const std::string mcf::Object::Variable::Inspect(void) const noexcept
{
	MCF_DEBUG_ASSERT(IsValid(), u8"Variable이 유효하지 않습니다.");
	return Name + " " + DataType.Inspect();
}

const size_t mcf::Object::FunctionParams::GetVariadicIndex(void) const noexcept
{
	MCF_DEBUG_ASSERT(HasVariadic(), u8"variadic 인자를 가지고 있어야 합니다.");
	return Variables.size() - 1;
}

const bool mcf::Object::Scope::IsGlobalScope(void) const noexcept
{
	return &_tree->Global == this;
}

const bool mcf::Object::Scope::IsIdentifierRegistered(const std::string& name) const noexcept
{
	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		return true;
	}
	if (_parent == nullptr)
	{
		return false;
	}
	return _parent->IsIdentifierRegistered(name);
}

const bool mcf::Object::Scope::DefineType(const std::string& name, const mcf::Object::TypeInfo& info) noexcept
{
	MCF_DEBUG_ASSERT(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	MCF_DEBUG_ASSERT(info.IsValid(), u8"함수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_typeInfoMap.emplace(name, info);

	return true;
}

const mcf::Object::TypeInfo mcf::Object::Scope::FindTypeInfo(const std::string& name) const noexcept
{
	MCF_DEBUG_ASSERT(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _typeInfoMap.find(name);
	if (infoFound == _typeInfoMap.end())
	{
		if (_parent == nullptr)
		{
			return mcf::Object::TypeInfo();
		}
		return _parent->FindTypeInfo(name);
	}
	return infoFound->second;
}

const bool mcf::Object::Scope::IsAllVariablesUsed(void) const noexcept
{
	for (auto pair : _variables)
	{
		if (pair.second.IsUsed == false)
		{
			return false;
		}
	}
	return true;
}

const mcf::Object::VariableInfo mcf::Object::Scope::DefineVariable(const std::string& name, const mcf::Object::Variable& variable) noexcept
{
	MCF_DEBUG_ASSERT(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	MCF_DEBUG_ASSERT(variable.IsValid(), u8"변수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return mcf::Object::VariableInfo();
	}

	_allIdentifierSet.emplace(name);
	_variables.emplace(name, variable);

	return { variable, _parent == nullptr };
}

const mcf::Object::VariableInfo mcf::Object::Scope::FindVariableInfo(const std::string& name) const noexcept
{
	MCF_DEBUG_ASSERT(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _variables.find(name);
	if (infoFound == _variables.end())
	{
		if (_parent == nullptr)
		{
			return mcf::Object::VariableInfo();
		}
		return _parent->FindVariableInfo(name);
	}
	return { infoFound->second, _parent == nullptr };
}

const bool mcf::Object::Scope::UseVariableInfo(const std::string& name) noexcept
{
	Scope* currentScope = this;
	while (currentScope != nullptr)
	{
		auto infoFound = currentScope->_variables.find(name);
		if (infoFound == currentScope->_variables.end())
		{
			if (currentScope->_allIdentifierSet.find(name) != currentScope->_allIdentifierSet.end())
			{
				MCF_DEBUG_MESSAGE(u8"해당 식별자는 변수가 아닙니다. 함수[UseVariableInfo] 식별자[%s]", name.c_str());
				return false;
			}
			currentScope = currentScope->_parent;
			continue;
		}

		infoFound->second.IsUsed = true;
		return true;
	}
	
	MCF_DEBUG_MESSAGE(u8"해당 식별자를 가지고 있는 변수를 찾을 수 없습니다. 함수[UseVariableInfo] 식별자[%s]", name.c_str());
	return false;
}

void mcf::Object::Scope::DetermineUnknownVariableTypeSize(const std::string& name, std::vector<size_t> arraySizeList) noexcept
{
	MCF_DEBUG_ASSERT(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _variables.find(name);
	if (infoFound == _variables.end())
	{
		MCF_DEBUG_ASSERT(_parent != nullptr, u8"현재 스코프에서 해당 이름의 변수를 찾을 수 없습니다.");
		return;
	}
	MCF_DEBUG_ASSERT(arraySizeList.size() == infoFound->second.DataType.ArraySizeList.size(), u8"주어진 배열 차원 수가 기존 배열 차원수와 다릅니다.");
	infoFound->second.DataType.ArraySizeList = arraySizeList;
	MCF_DEBUG_ASSERT(infoFound->second.DataType.HasUnknownArrayIndex() == false, u8"주어진 배열 크기 값에 unknown이 있으면 안됩니다.");
}

const bool mcf::Object::Scope::MakeLocalScopeToFunctionInfo(_Inout_ mcf::Object::FunctionInfo& info) noexcept
{
	MCF_DEBUG_ASSERT(info.Name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	MCF_DEBUG_ASSERT(info.LocalScope == nullptr, u8"로컬 스코프가 이미 할당 되어 있습니다.");

	if (MakeLocalScope(&info.LocalScope, true) == false)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}

	if (info.Params.IsVoid() == false)
	{
		const size_t paramCount = info.Params.Variables.size();
		for (size_t i = 0; i < paramCount; ++i)
		{
			info.LocalScope->DefineVariable(info.Params.Variables[i].Name, info.Params.Variables[i]);
		}
	}
	return true;
}

const bool mcf::Object::Scope::DefineFunction(const std::string& name, const mcf::Object::FunctionInfo& info) noexcept
{
	MCF_DEBUG_ASSERT(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	MCF_DEBUG_ASSERT(info.IsValid(), u8"함수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_functionInfoMap.emplace(name, info);

	return true;
}

const mcf::Object::FunctionInfo mcf::Object::Scope::FindFunction(const std::string& name) const noexcept
{
	MCF_DEBUG_ASSERT(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _functionInfoMap.find(name);
	if (infoFound == _functionInfoMap.end())
	{
		if (_parent == nullptr)
		{
			return mcf::Object::FunctionInfo();
		}
		return _parent->FindFunction(name);
	}
	return infoFound->second;
}

const mcf::Object::FunctionInfo mcf::Object::Scope::FindInternalFunction(const InternalFunctionType functionType) const noexcept
{
	return _tree->InternalFunctionInfosByTypes[mcf::ENUM_INDEX(functionType)];
}

const bool mcf::Object::Scope::MakeLocalScope(_Outptr_ mcf::Object::Scope** outScopePtr, const bool isFunctionScope) noexcept
{
	MCF_DEBUG_ASSERT(*outScopePtr == nullptr, u8"outScope는 null로 초기화 되어 있어야합니다.");
	_tree->Locals.emplace_back(std::make_unique<Scope>(_tree, this, isFunctionScope));
	if (_tree->Locals.back() == nullptr)
	{
		MCF_DEBUG_TODO(u8"구현 필요");
		return false;
	}

	*outScopePtr = _tree->Locals.back().get();
	return true;
}

const mcf::Object::TypeInfo mcf::IR::Expression::Interface::GetDataTypeFromExpression(const mcf::IR::Expression::Interface* expression) noexcept
{
	constexpr const size_t EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (expression->GetExpressionType())
	{
	case mcf::IR::Expression::Type::TYPE_IDENTIFIER: __COUNTER__;
		return static_cast<const mcf::IR::Expression::TypeIdentifier*>(expression)->GetInfo();

	case mcf::IR::Expression::Type::GLOBAL_VARIABLE_IDENTIFIER: __COUNTER__;
		return static_cast<const mcf::IR::Expression::GlobalVariableIdentifier*>(expression)->GetVariable().DataType;

	case mcf::IR::Expression::Type::LOCAL_VARIABLE_IDENTIFIER: __COUNTER__;
		return static_cast<const mcf::IR::Expression::LocalVariableIdentifier*>(expression)->GetVariable().DataType;

	case mcf::IR::Expression::Type::FUNCTION_IDENTIFIER: __COUNTER__;
		return static_cast<const mcf::IR::Expression::FunctionIdentifier*>(expression)->GetInfo().ReturnType;

	case mcf::IR::Expression::Type::INTEGER: __COUNTER__;
		MCF_DEBUG_TODO(u8"구현 필요");
		break;

	case mcf::IR::Expression::Type::STRING: __COUNTER__;
		return mcf::Object::TypeInfo::MakePrimitive(true, "byte", 1, { static_cast<const mcf::IR::Expression::String*>(expression)->GetSize() });

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
		return static_cast<const mcf::IR::Expression::StaticCast*>(expression)->GetCastedDatType();

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
	static_assert(static_cast<size_t>(mcf::IR::Expression::Type::COUNT) == EXPRESSION_TYPE_COUNT, "get data type from expression.");
	return mcf::Object::TypeInfo();
}

mcf::IR::Expression::TypeIdentifier::TypeIdentifier(const mcf::Object::TypeInfo& info) noexcept
	: _info(info)
{
	MCF_DEBUG_ASSERT(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

mcf::IR::Expression::GlobalVariableIdentifier::GlobalVariableIdentifier(const mcf::Object::Variable& variable) noexcept
	: _variable(variable)
{
	MCF_DEBUG_ASSERT(_variable.IsValid(), u8"_variable가 유효하지 않습니다.");
}

mcf::IR::Expression::LocalVariableIdentifier::LocalVariableIdentifier(const mcf::Object::Variable& variable) noexcept
	: _variable(variable)
{
	MCF_DEBUG_ASSERT(_variable.IsValid(), u8"_info가 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::LocalVariableIdentifier::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

mcf::IR::Expression::FunctionIdentifier::FunctionIdentifier(const mcf::Object::FunctionInfo& info) noexcept
	: _info(info)
{
	MCF_DEBUG_ASSERT(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::FunctionIdentifier::Inspect(void) const noexcept
{
	MCF_DEBUG_TODO(u8"구현 필요");
	return std::string();
}

const bool mcf::IR::Expression::Integer::IsCompatible(const mcf::Object::TypeInfo& dataType) const noexcept
{
	if (dataType.IsIntegerType() == false)
	{
		MCF_DEBUG_MESSAGE(u8"정수 타입이 아닌 타입은 정수로 호환되지 않습니다.");
		return false;
	}

	switch (dataType.GetSize())
		{
		case sizeof(__int8):
			MCF_DEBUG_MESSAGE_RETURN_BOOL(dataType.IsUnsigned ? IsUInt8() : IsInt8(), u8"데이터 타입과 호환되지 않는 정수 입니다. 데이터 타입[%s] 정수 값[%s]", dataType.Inspect().c_str(), std::to_string(_isUnsigned ? _unsignedValue : _signedValue).c_str());
		case sizeof(__int16):
			MCF_DEBUG_MESSAGE_RETURN_BOOL(dataType.IsUnsigned ? IsUInt16() : IsInt16(), u8"데이터 타입과 호환되지 않는 정수 입니다. 데이터 타입[%s] 정수 값[%s]", dataType.Inspect().c_str(), std::to_string(_isUnsigned ? _unsignedValue : _signedValue).c_str());
		case sizeof(__int32):
			MCF_DEBUG_MESSAGE_RETURN_BOOL(dataType.IsUnsigned ? IsUInt32() : IsInt32(), u8"데이터 타입과 호환되지 않는 정수 입니다. 데이터 타입[%s] 정수 값[%s]", dataType.Inspect().c_str(), std::to_string(_isUnsigned ? _unsignedValue : _signedValue).c_str());
		case sizeof(__int64):
			MCF_DEBUG_MESSAGE_RETURN_BOOL(dataType.IsUnsigned ? IsUInt64() : IsInt64(), u8"데이터 타입과 호환되지 않는 정수 입니다. 데이터 타입[%s] 정수 값[%s]", dataType.Inspect().c_str(), std::to_string(_isUnsigned ? _unsignedValue : _signedValue).c_str());
		default:
			MCF_DEBUG_MESSAGE(u8"타입의 크기가 호환 가능한 정수 타입의 크기와 맞지 않습니다. 타입 크기[%zu]", dataType.GetSize());
			return false;
	}
}

const __int64 mcf::IR::Expression::Integer::GetInt64(void) const noexcept
{
	MCF_DEBUG_ASSERT(_isUnsigned == false, u8"signed가 아닌 값은 int64 타입의 값이 될 수 없습니다.");
	return _signedValue;
}

const __int32 mcf::IR::Expression::Integer::GetInt32(void) const noexcept
{
	MCF_DEBUG_ASSERT((_isUnsigned == false && INT32_MIN <= _signedValue && _signedValue <= INT32_MAX) || (_isUnsigned == true && _unsignedValue <= INT32_MAX), u8"가지고 있는 값이 int32 타입의 범위 안에 있지 않습니다.");
	return static_cast<__int32>(_signedValue);
}

const __int16 mcf::IR::Expression::Integer::GetInt16(void) const noexcept
{
	MCF_DEBUG_ASSERT((_isUnsigned == false && INT16_MIN <= _signedValue && _signedValue <= INT16_MAX) || (_isUnsigned == true && _unsignedValue <= INT16_MAX), u8"가지고 있는 값이 int16 타입의 범위 안에 있지 않습니다.");
	return static_cast<__int16>(_signedValue);
}

const __int8 mcf::IR::Expression::Integer::GetInt8(void) const noexcept
{
	MCF_DEBUG_ASSERT((_isUnsigned == false && INT8_MIN <= _signedValue && _signedValue <= INT8_MAX) || (_isUnsigned == true && _unsignedValue <= INT8_MAX), u8"가지고 있는 값이 int8 타입의 범위 안에 있지 않습니다.");
	return static_cast<__int8>(_signedValue);
}

const unsigned __int64 mcf::IR::Expression::Integer::GetUInt64(void) const noexcept
{
	MCF_DEBUG_ASSERT(IsNaturalInteger(), u8"자연수가 아닌 값은 uint64 타입의 값이 될 수 없습니다.");
	return _unsignedValue;
}

const unsigned __int32 mcf::IR::Expression::Integer::GetUInt32(void) const noexcept
{
	MCF_DEBUG_ASSERT((_isUnsigned == false && 0 <= _signedValue && _signedValue <= UINT32_MAX) || (_isUnsigned == true && _unsignedValue <= UINT32_MAX), u8"가지고 있는 값이 uint32 타입의 범위 안에 있지 않습니다.");
	return static_cast<unsigned __int32>(_unsignedValue);
}

const unsigned __int16 mcf::IR::Expression::Integer::GetUInt16(void) const noexcept
{
	MCF_DEBUG_ASSERT((_isUnsigned == false && 0 <= _signedValue && _signedValue <= UINT16_MAX) || (_isUnsigned == true && _unsignedValue <= UINT16_MAX), u8"가지고 있는 값이 uint16 타입의 범위 안에 있지 않습니다.");
	return static_cast<unsigned __int16>(_unsignedValue);
}

const unsigned __int8 mcf::IR::Expression::Integer::GetUInt8( void ) const noexcept
{
	MCF_DEBUG_ASSERT((_isUnsigned == false && 0 <= _signedValue && _signedValue <= UINT8_MAX) || (_isUnsigned == true && _unsignedValue <= UINT8_MAX), u8"가지고 있는 값이 uint8 타입의 범위 안에 있지 않습니다.");
	return static_cast<unsigned __int8>(_unsignedValue);
}

const std::string mcf::IR::Expression::Integer::Inspect(void) const noexcept
{
	return std::to_string(_isUnsigned ? _unsignedValue : _signedValue);
}

const std::string mcf::IR::Expression::String::Inspect(void) const noexcept
{
	return "?" + std::to_string(_index);
}

mcf::IR::Expression::Initializer::Initializer(PointerVector&& keyList) noexcept
	: _keyList(std::move(keyList))
{
#if defined(_DEBUG)
	const size_t size = _keyList.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_keyList[i].get() != nullptr, u8"_keyList[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Expression::Initializer::Inspect(void) const noexcept
{
	const size_t keyListCount = _keyList.size();
	MCF_DEBUG_ASSERT(keyListCount != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");

	std::string buffer;
	buffer = "{ ";
	for (size_t i = 0; i < keyListCount; i++)
	{
		MCF_DEBUG_ASSERT(_keyList[i].get() != nullptr, u8"_keyList[%zu].key는 nullptr 여선 안됩니다.", i);
		buffer += _keyList[i]->Inspect() + ", ";
	}
	return buffer + "}";
}

mcf::IR::Expression::MapInitializer::MapInitializer(PointerVector&& keyList, PointerVector&& valueList) noexcept
	: Initializer(std::move(keyList))
	, _valueList(std::move(valueList))
{
#if defined(_DEBUG)
	const size_t size = _valueList.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_valueList에 값이 최소 한개 이상 있어야 합니다.");
	MCF_DEBUG_ASSERT(GetKeyExpressionCount() == size, u8"key와 value의 갯수가 같아야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_valueList[i].get() != nullptr, u8"_valueList[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Expression::MapInitializer::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

mcf::IR::Expression::Call::Call(const mcf::Object::FunctionInfo& info, mcf::IR::Expression::PointerVector&& paramObjects) noexcept
	: _info(info)
	, _paramObjects(std::move(paramObjects))
{
	MCF_DEBUG_ASSERT(_info.IsValid(), u8"함수 정의가 유효하지 않습니다.");
#if defined(_DEBUG)
	const size_t size = _paramObjects.size();
	if (_info.Params.HasVariadic())
	{
		MCF_DEBUG_ASSERT(size >= _info.Params.Variables.size() - 1, u8"_paramObjects에 값은 함수 정보안에 있는 인자 갯수와 동일 해야합니다.");
	}
	else
	{
		MCF_DEBUG_ASSERT(size == _info.Params.Variables.size(), u8"_paramObjects에 값은 함수 정보안에 있는 인자 갯수와 동일 해야합니다.");
	}

	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_paramObjects[i].get() != nullptr, u8"_paramObjects[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Expression::Call::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

mcf::IR::Expression::StaticCast::StaticCast(mcf::IR::Expression::Pointer&& castingValue, const mcf::Object::TypeInfo& castedType) noexcept
	: _castingValue(std::move(castingValue))
	, _castedType(castedType)
{
	MCF_DEBUG_ASSERT(_castingValue.get() && _castingValue->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"캐스팅 하려는 변수가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(_castedType.IsValid(), u8"변수에 캐스팅 되려하는 타입이 유효하지 않습니다.");

#if defined(_DEBUG)
	const mcf::Object::TypeInfo typeToCast = GetDataTypeFromExpression(_castingValue.get());
	MCF_DEBUG_ASSERT(typeToCast.IsStaticCastable(_castedType), u8"정적 캐스팅이 불가능합니다. 현재 타입[%s] 캐스팅 타입[%s]", typeToCast.Inspect().c_str(), _castedType.Inspect().c_str());
#endif
}

const std::string mcf::IR::Expression::StaticCast::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

mcf::IR::Expression::Assign::Assign(mcf::IR::Expression::Pointer&& left, mcf::IR::Expression::Pointer&& right) noexcept
	: _left(std::move(left))
	, _right(std::move(right))
{
	MCF_DEBUG_ASSERT(_left.get() && _left->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"left 표현식이 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(_right.get() && _right->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"right 표현식이 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::Assign::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

const bool mcf::IR::Expression::Conditional::IsValidTokenType(const mcf::Token::Type type) noexcept
{
	constexpr const size_t TOKENTYPE_COUNT_BEGIN = __COUNTER__;
	switch (type)
	{
	case Token::Type::EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::NOT_EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::LT: __COUNTER__; [[fallthrough]];
	case Token::Type::GT: __COUNTER__;
		return true;

	case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
	case Token::Type::MINUS: __COUNTER__; [[fallthrough]];
	case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
	case Token::Type::SLASH: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_INCLUDE: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_TYPEDEF: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_EXTERN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_LET: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_RETURN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_FUNC: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_MAIN: __COUNTER__; [[fallthrough]];
	case Token::Type::IDENTIFIER: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_WHILE: __COUNTER__; [[fallthrough]];
	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::SEMICOLON: __COUNTER__; [[fallthrough]];
	case Token::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case Token::Type::STRING: __COUNTER__; [[fallthrough]];
	case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	case Token::Type::BANG: __COUNTER__; [[fallthrough]];
	case Token::Type::AMPERSAND: __COUNTER__; [[fallthrough]];
	case Token::Type::LPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMA: __COUNTER__; [[fallthrough]];
	case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_VOID: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNSIGNED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_AS: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_BREAK: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
	case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		break;
	}
	constexpr const size_t TOKENTYPE_COUNT = __COUNTER__ - TOKENTYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == TOKENTYPE_COUNT, "evaluate whether token type is conditional for infix expression.");
	return false;
}

mcf::IR::ASM::Pointer mcf::IR::Expression::Conditional::GenerateJumpIf(const mcf::Token::Type type, const std::string& label) noexcept
{
	constexpr const size_t TOKENTYPE_COUNT_BEGIN = __COUNTER__;
	switch (type)
	{
	case Token::Type::EQUAL: __COUNTER__;
		return mcf::IR::ASM::Je::Make(label);

	case Token::Type::NOT_EQUAL: __COUNTER__;
		return mcf::IR::ASM::Invalid::Make();

	case Token::Type::LT: __COUNTER__;
		return mcf::IR::ASM::Jl::Make(label);

	case Token::Type::GT: __COUNTER__;
		return mcf::IR::ASM::Invalid::Make();

	case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
	case Token::Type::MINUS: __COUNTER__; [[fallthrough]];
	case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
	case Token::Type::SLASH: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_INCLUDE: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_TYPEDEF: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_EXTERN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_LET: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_RETURN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_FUNC: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_MAIN: __COUNTER__; [[fallthrough]];
	case Token::Type::IDENTIFIER: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_WHILE: __COUNTER__; [[fallthrough]];
	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::SEMICOLON: __COUNTER__; [[fallthrough]];
	case Token::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case Token::Type::STRING: __COUNTER__; [[fallthrough]];
	case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	case Token::Type::BANG: __COUNTER__; [[fallthrough]];
	case Token::Type::AMPERSAND: __COUNTER__; [[fallthrough]];
	case Token::Type::LPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMA: __COUNTER__; [[fallthrough]];
	case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_VOID: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNSIGNED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_AS: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_BREAK: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
	case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		break;
	}
	constexpr const size_t TOKENTYPE_COUNT = __COUNTER__ - TOKENTYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == TOKENTYPE_COUNT, "generate jump asm code with the given token condition.");
	return mcf::IR::ASM::Invalid::Make();
}

mcf::IR::Expression::Conditional::Conditional(mcf::IR::Expression::Pointer&& left, const mcf::Token::Data& infixOperator, mcf::IR::Expression::Pointer&& right) noexcept
	: _left(std::move(left))
	, _infixOperator(infixOperator)
	, _right(std::move(right))
{
	MCF_DEBUG_ASSERT(_infixOperator.Type != mcf::Token::Type::INVALID, u8"_infixOperator가 유효하지 않습니다..");
	MCF_DEBUG_ASSERT(_left.get() && _left->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"left 표현식이 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(_right.get() && _right->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"right 표현식이 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::Conditional::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

const bool mcf::IR::Expression::Arithmetic::IsValidTokenType(const mcf::Token::Type type) noexcept
{
	constexpr const size_t TOKENTYPE_COUNT_BEGIN = __COUNTER__;
	switch (type)
	{
	case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
	case Token::Type::MINUS: __COUNTER__; [[fallthrough]];
	case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
	case Token::Type::SLASH: __COUNTER__;
		return true;

	case Token::Type::EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::NOT_EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::LT: __COUNTER__; [[fallthrough]];
	case Token::Type::GT: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_INCLUDE: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_TYPEDEF: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_EXTERN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_LET: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_RETURN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_FUNC: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_MAIN: __COUNTER__; [[fallthrough]];
	case Token::Type::IDENTIFIER: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_WHILE: __COUNTER__; [[fallthrough]];
	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::SEMICOLON: __COUNTER__; [[fallthrough]];
	case Token::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case Token::Type::STRING: __COUNTER__; [[fallthrough]];
	case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	case Token::Type::BANG: __COUNTER__; [[fallthrough]];
	case Token::Type::AMPERSAND: __COUNTER__; [[fallthrough]];
	case Token::Type::LPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMA: __COUNTER__; [[fallthrough]];
	case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_VOID: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNSIGNED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_AS: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_BREAK: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
	case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		break;
	}
	constexpr const size_t TOKENTYPE_COUNT = __COUNTER__ - TOKENTYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == TOKENTYPE_COUNT, "evaluate whether token type is arithemetic for infix expression.");
	return false;
}

mcf::IR::Expression::Arithmetic::Arithmetic(mcf::IR::Expression::Pointer&& left, const mcf::Token::Data& infixOperator, mcf::IR::Expression::Pointer&& right) noexcept
	: _left(std::move(left))
	, _infixOperator(infixOperator)
	, _right(std::move(right))
{
	MCF_DEBUG_ASSERT(_infixOperator.Type != mcf::Token::Type::INVALID, u8"_infixOperator가 유효하지 않습니다..");
	MCF_DEBUG_ASSERT(_left.get() && _left->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"left 표현식이 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(_right.get() && _right->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"right 표현식이 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::Arithmetic::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

mcf::IR::ASM::Address::Address(const mcf::Object::TypeInfo& targetType, mcf::IR::ASM::Register targetRegister, const size_t offset)
	: _targetType(targetType)
	, _targetAddress(Internal::GetAddressOf(targetRegister, offset))
{
	MCF_DEBUG_ASSERT(targetType.IsValid(), u8"유효하지 않은 타입입니다.");
	MCF_DEBUG_ASSERT(targetType.IsArrayType() == false, u8"배열 타입은 허용되지 않습니다.");
	MCF_DEBUG_ASSERT(targetType.IsVariadic == false, u8"variadic은 허용되지 않습니다.");
	MCF_DEBUG_ASSERT(targetType.GetSize() == targetType.IntrinsicSize, u8"타입의 고유 사이즈와 실제사이즈가 같아야 합니다.");
	MCF_DEBUG_ASSERT(targetRegister != Register::INVALID && targetRegister < Register::COUNT, u8"유효하지 않은 레지스터 값입니다.");
}

const std::string mcf::IR::ASM::Address::Inspect(void) const noexcept
{
	return _targetType.Inspect() + " ptr " + _targetAddress;
}

mcf::IR::ASM::UnsafePointerAddress::UnsafePointerAddress(_Notnull_ const mcf::IR::Expression::String* stringExpression)
	: _identifier(stringExpression->Inspect())
	, _hasOffset(false)
{}

mcf::IR::ASM::UnsafePointerAddress::UnsafePointerAddress(mcf::IR::ASM::Register targetRegister, const size_t offset)
	: _identifier(CONVERT_REGISTER_TO_STRING(targetRegister))
	, _offset(offset)
	, _hasOffset(true)
{
	MCF_DEBUG_ASSERT(targetRegister != Register::INVALID && targetRegister < Register::COUNT, u8"유효하지 않은 레지스터 값입니다.");
}

mcf::IR::ASM::UnsafePointerAddress::UnsafePointerAddress(const UnsafePointerAddress& targetAddress, const size_t offset)
	: _identifier(targetAddress._identifier)
	, _offset(targetAddress._offset + offset)
	, _hasOffset(true)
{}

const std::string mcf::IR::ASM::UnsafePointerAddress::Inspect(void) const noexcept
{
	return _hasOffset ? Internal::GetAddressOf(_identifier, _offset) : Internal::GetAddressOf(_identifier);
}

mcf::IR::ASM::SizeOf::SizeOf(const mcf::IR::Expression::String* stringExpression)
	: _targetSize("sizeof " + stringExpression->Inspect())
{
}

mcf::IR::ASM::ProcBegin::ProcBegin(const std::string& name) noexcept
	: _name(name)
{
	MCF_DEBUG_ASSERT(_name.empty() == false, u8"프로시져의 이름이 반드시 존재해야 합니다.");
}

const std::string mcf::IR::ASM::ProcBegin::Inspect(void) const noexcept
{
	return _name + " proc\n";
}

mcf::IR::ASM::ProcEnd::ProcEnd(const std::string& name) noexcept
	: _name( name )
{
	MCF_DEBUG_ASSERT(_name.empty() == false, u8"프로시져의 이름이 반드시 존재해야 합니다.");
}

const std::string mcf::IR::ASM::ProcEnd::Inspect(void) const noexcept
{
	return _name + " endp";
}

mcf::IR::ASM::Push::Push(const Register address) noexcept
	: _value(CONVERT_REGISTER_TO_STRING(address))
{
	MCF_DEBUG_ASSERT(address != mcf::IR::ASM::Register::INVALID, u8"address가 유효하지 않습니다.");
}

const std::string mcf::IR::ASM::Push::Inspect(void) const noexcept
{
	return "\tpush " + _value + "\n";
}

mcf::IR::ASM::Pop::Pop(const Register target) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
}

const std::string mcf::IR::ASM::Pop::Inspect(void) const noexcept
{
	return "\tpop " + _target + "\n";
}

mcf::IR::ASM::Mov::Mov(const Address& target, const Register source) noexcept
	: _target(target.Inspect())
	, _source(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(source))
{
	MCF_DEBUG_ASSERT(source != mcf::IR::ASM::Register::INVALID, u8"source가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(Internal::IsSizeMatching(source, target.GetTypeInfo().GetSize()), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
}

mcf::IR::ASM::Mov::Mov(const Address& target, const __int64 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(__int64), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == false, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const __int32 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(__int32), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == false, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const __int16 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(__int16), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == false, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const __int8 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(__int8), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == false, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const unsigned __int64 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(unsigned __int64), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == true, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const unsigned __int32 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(unsigned __int32), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == true, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const unsigned __int16 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(unsigned __int16), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == true, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const unsigned __int8 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target.GetTypeInfo().GetSize() >= sizeof(unsigned __int8), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(target.GetTypeInfo().IsUnsigned == true, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, _In_ const mcf::IR::Expression::GlobalVariableIdentifier* globalExpression) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(globalExpression->GetVariable().Name)
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(Internal::IsSizeMatching(target, globalExpression->GetVariable().GetTypeSize()), u8"레지스터가 source의 데이터 크기와 맞지 않는 레지스터 입니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const Address& source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(source.Inspect())
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(Internal::IsSizeMatching(target, source.GetTypeInfo().GetSize()), u8"레지스터가 source의 데이터 크기와 맞지 않는 레지스터 입니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const SizeOf& source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(source.Inspect())
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(target) == sizeof(__int64), u8"64비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const __int64 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int64), u8"레지스터의 크기가 인자로 받은 64비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const __int32 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int32), u8"레지스터의 크기가 인자로 받은 32비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const __int16 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int16), u8"레지스터의 크기가 인자로 받은 16비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const __int8 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int8), u8"레지스터의 크기가 인자로 받은 8비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const unsigned __int64 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int64), u8"레지스터의 크기가 인자로 받은 64비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const unsigned __int32 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int32), u8"레지스터의 크기가 인자로 받은 32비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const unsigned __int16 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int16), u8"레지스터의 크기가 인자로 받은 16비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Mov::Mov(const Register target, const unsigned __int8 source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(std::to_string(source))
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(target) >= sizeof(__int8), u8"레지스터의 크기가 인자로 받은 8비트 정수의 값을 수용할 수 없습니다.");
}

const std::string mcf::IR::ASM::Mov::Inspect(void) const noexcept
{
	return "\tmov " + _target + ", " + _source + "\n";
}

mcf::IR::ASM::Lea::Lea(const Register target, const mcf::IR::ASM::UnsafePointerAddress& source) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
	, _source(source.Inspect())
{
	MCF_DEBUG_ASSERT(target != mcf::IR::ASM::Register::INVALID, u8"target가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(target) == sizeof(__int64), u8"64비트 레지스터가 아닙니다.");
}

const std::string mcf::IR::ASM::Lea::Inspect(void) const noexcept
{
	return "\tlea " + _target + ", " + _source + "\n";
}

mcf::IR::ASM::Add::Add(const Register lhs, const __int64 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int64), u8"레지스터의 크기가 인자로 받은 64비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const __int32 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int32), u8"레지스터의 크기가 인자로 받은 32비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const __int16 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int16), u8"레지스터의 크기가 인자로 받은 16비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const __int8 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int8), u8"레지스터의 크기가 인자로 받은 8비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const unsigned __int64 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int64), u8"레지스터의 크기가 인자로 받은 64비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const unsigned __int32 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int32), u8"레지스터의 크기가 인자로 받은 32비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const unsigned __int16 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int16), u8"레지스터의 크기가 인자로 받은 16비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const unsigned __int8 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(mcf::IR::ASM::GET_REGISTER_SIZE_VALUE(lhs) >= sizeof(__int8), u8"레지스터의 크기가 인자로 받은 8비트 정수의 값을 수용할 수 없습니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const Address& rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(rhs.Inspect())
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(Internal::IsSizeMatching(lhs, rhs.GetTypeInfo().GetSize()), u8"레지스터가 rhs의 데이터 크기와 맞지 않는 레지스터 입니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const Register rhs) noexcept
	: _rhs(CONVERT_REGISTER_TO_STRING(rhs))
	, _lhs(lhs.Inspect())
{
	MCF_DEBUG_ASSERT(rhs != mcf::IR::ASM::Register::INVALID, u8"rhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(Internal::IsSizeMatching(rhs, lhs.GetTypeInfo().GetSize()), u8"레지스터가 lhs의 데이터 크기와 맞지 않는 레지스터 입니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const __int64 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(__int64), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == false, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const __int32 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(__int32), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == false, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const __int16 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(__int16), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == false, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const __int8 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(__int8), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == false, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const unsigned __int64 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(unsigned __int64), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == true, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const unsigned __int32 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(unsigned __int32), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == true, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const unsigned __int16 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(unsigned __int16), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == true, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Add::Add(const Address& lhs, const unsigned __int8 rhs) noexcept
	: _lhs(lhs.Inspect())
	, _rhs(std::to_string(rhs))
{
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().GetSize() >= sizeof(unsigned __int8), u8"rhs와 lhs의 사이즈가 일치 하지 않습니다. Size=%zu", lhs.GetTypeInfo().GetSize());
	MCF_DEBUG_ASSERT(lhs.GetTypeInfo().IsUnsigned == true, u8"rhs와 lhs의 사인 타입이 일치 하지 않습니다.");
}

const std::string mcf::IR::ASM::Add::Inspect(void) const noexcept
{
	return "\tadd " + _lhs + ", " + _rhs + "\n";
}

mcf::IR::ASM::Sub::Sub(const Register minuend, const __int64 subtrahend) noexcept
	: _minuend(CONVERT_REGISTER_TO_STRING(minuend))
	, _subtrahend(std::to_string(subtrahend))
{
	MCF_DEBUG_ASSERT(minuend != mcf::IR::ASM::Register::INVALID, u8"minuend가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(minuend) == sizeof(__int64), u8"64비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Sub::Sub(const Register minuend, const unsigned __int64 subtrahend) noexcept
	: _minuend(CONVERT_REGISTER_TO_STRING(minuend))
	, _subtrahend(std::to_string(subtrahend))
{
	MCF_DEBUG_ASSERT(minuend != mcf::IR::ASM::Register::INVALID, u8"minuend가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(minuend) == sizeof(__int64), u8"64비트 레지스터가 아닙니다.");
}

const std::string mcf::IR::ASM::Sub::Inspect(void) const noexcept
{
	return "\tsub " + _minuend + ", " + _subtrahend + "\n";
}

mcf::IR::ASM::Xor::Xor(const Register& lhs, const Register rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(CONVERT_REGISTER_TO_STRING(rhs))
{
	MCF_DEBUG_ASSERT(lhs != mcf::IR::ASM::Register::INVALID, u8"lhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(rhs != mcf::IR::ASM::Register::INVALID, u8"rhs가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(Internal::IsSizeMatching(lhs, rhs), u8"64비트 레지스터가 아닙니다.");
}

const std::string mcf::IR::ASM::Xor::Inspect(void) const noexcept
{
	return "\txor " + _lhs + ", " + _rhs + "\n";
}

mcf::IR::ASM::Call::Call(const std::string& procName) noexcept
	: _procName(procName)
{}

const std::string mcf::IR::ASM::Call::Inspect(void) const noexcept
{
	return "\tcall " + _procName + "\n";
}

mcf::IR::ASM::Label::Label(const std::string& labelName) noexcept
	: _labelName(labelName)
{}

const std::string mcf::IR::ASM::Label::Inspect(void) const noexcept
{
	return _labelName + ":\n";
}

mcf::IR::ASM::Cmp::Cmp(const mcf::IR::ASM::Register leftRegister, const mcf::IR::ASM::Register rightRegister) noexcept
	: _lhs(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(leftRegister))
	, _rhs(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(rightRegister))
{
	MCF_DEBUG_ASSERT(leftRegister != mcf::IR::ASM::Register::INVALID, u8"leftRegister가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(rightRegister != mcf::IR::ASM::Register::INVALID, u8"rightRegister가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(Internal::IsSizeMatching(leftRegister, rightRegister), u8"64비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Cmp::Cmp(const mcf::IR::ASM::Register leftRegister, const __int8 rightValue) noexcept
	: _lhs(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(leftRegister))
	, _rhs(std::to_string(static_cast<int>(rightValue)))
{
	MCF_DEBUG_ASSERT(leftRegister != mcf::IR::ASM::Register::INVALID, u8"leftRegister가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(leftRegister) == sizeof(__int8), u8"8비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Cmp::Cmp(const mcf::IR::ASM::Register leftRegister, const __int16 rightValue) noexcept
	: _lhs(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(leftRegister))
	, _rhs(std::to_string(static_cast<int>(rightValue)))
{
	MCF_DEBUG_ASSERT(leftRegister != mcf::IR::ASM::Register::INVALID, u8"leftRegister가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(leftRegister) == sizeof(__int16), u8"16비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Cmp::Cmp(const mcf::IR::ASM::Register leftRegister, const __int32 rightValue) noexcept
	: _lhs(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(leftRegister))
	, _rhs(std::to_string(static_cast<int>(rightValue)))
{
	MCF_DEBUG_ASSERT(leftRegister != mcf::IR::ASM::Register::INVALID, u8"leftRegister가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(leftRegister) == sizeof(__int32), u8"32비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Cmp::Cmp(const mcf::IR::ASM::Register leftRegister, const __int64 rightValue) noexcept
	: _lhs(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(leftRegister))
	, _rhs(std::to_string(static_cast<int>(rightValue)))
{
	MCF_DEBUG_ASSERT(leftRegister != mcf::IR::ASM::Register::INVALID, u8"leftRegister가 유효하지 않습니다.");
	MCF_DEBUG_ASSERT(GET_REGISTER_SIZE_VALUE(leftRegister) == sizeof(__int64), u8"64비트 레지스터가 아닙니다.");
}

const std::string mcf::IR::ASM::Cmp::Inspect(void) const noexcept
{
	return "\tcmp " + _lhs + ", " + _rhs + "\n";
}

mcf::IR::ASM::Jmp::Jmp(const std::string& label) noexcept
	: _label(label)
{}

const std::string mcf::IR::ASM::Jmp::Inspect(void) const noexcept
{
	return "\tjmp " + _label + "\n";
}

mcf::IR::ASM::Jl::Jl(const std::string& label) noexcept
	: _label(label)
{}

const std::string mcf::IR::ASM::Jl::Inspect(void) const noexcept
{
	return "\tjl " + _label + "\n";
}

mcf::IR::ASM::Je::Je(const std::string& label) noexcept
	: _label(label)
{}

const std::string mcf::IR::ASM::Je::Inspect(void) const noexcept
{
	return "\tje " + _label + "\n";
}

mcf::IR::IncludeLib::IncludeLib(const std::string& libPath) noexcept
	: _libPath(libPath)
{
	MCF_DEBUG_ASSERT(_libPath.empty() == false, u8"유효하지 않은 _libPath입니다.");
}

const std::string mcf::IR::IncludeLib::Inspect(void) const noexcept
{
	MCF_DEBUG_ASSERT(_libPath.empty() == false, u8"유효하지 않은 _libPath입니다.");
	return "includelib " + _libPath;
}

mcf::IR::Typedef::Typedef(const mcf::Object::TypeInfo& definedType, const mcf::Object::TypeInfo& sourceType) noexcept
	: _definedType(definedType)
	, _sourceType(sourceType)
{
	MCF_DEBUG_ASSERT(_definedType.IsValid(), u8"유효하지 않은 _definedType입니다.");
	MCF_DEBUG_ASSERT(_sourceType.IsValid(), u8"유효하지 않은 _sourceType입니다.");
}

const std::string mcf::IR::Typedef::Inspect(void) const noexcept
{
	return _definedType.Name + " typedef " + _sourceType.Inspect();
}

mcf::IR::Extern::Extern(const std::string& name, const std::vector<mcf::Object::Variable>& params) noexcept
	: _name(name)
	, _params(params)
{
#if defined(_DEBUG)
	const size_t size = _params.size();
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_params[i].IsValid(), u8"_params[%zu]가 유효하지 않습니다.", i);
		MCF_DEBUG_ASSERT(_params[i].DataType.IsValid(), u8"_params[%zu].DataType가 유효하지 않습니다.", i);
		MCF_DEBUG_ASSERT(_params[i].DataType.HasUnknownArrayIndex() == false, u8"unknown 배열이 있으면 안됩니다. _params[%zu].DataType", i);
	}
#endif
}

const std::string mcf::IR::Extern::Inspect(void) const noexcept
{
	std::string buffer = _name + " PROTO";
	const size_t size = _params.size();
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_params[i].IsValid(), u8"_params[%zu]가 valid 하지 않습니다.", i);
		MCF_DEBUG_ASSERT(_params[i].DataType.IsValid(), u8"_params[%zu].DataType가 유효하지 않습니다.", i);
		MCF_DEBUG_ASSERT(_params[i].DataType.HasUnknownArrayIndex() == false, u8"unknown 배열이 있으면 안됩니다. _params[%zu].DataType", i);
		buffer += " " + _params[i].Name + ":" + _params[i].DataType.Inspect() + (i == size - 1 ? "" : ",");
	}
	return buffer;
}

mcf::IR::Let::Let(const mcf::Object::VariableInfo&& info, mcf::IR::Expression::Pointer&& assignedExpression) noexcept
	: _info(info)
	, _assignExpression(std::move(assignedExpression))
{
	MCF_DEBUG_ASSERT(_info.IsValid(), u8"변수 정보가 유효하지 않습니다.");
}

const std::string mcf::IR::Let::Inspect(void) const noexcept
{
	MCF_DEBUG_ASSERT(_info.IsGlobal, u8"지역 변수는 중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");

	std::string buffer;
	buffer = _info.Variable.Inspect() + " ";
	return buffer + (_assignExpression.get() == nullptr ? "?" : _assignExpression->Inspect());
}

mcf::IR::Func::Func(mcf::IR::ASM::PointerVector&& defines) noexcept
	: _defines(std::move(defines))
{
#if defined(_DEBUG)
	const size_t size = _defines.size();
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_defines[i].get() != nullptr, u8"_defines[%zu]가 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Func::Inspect(void) const noexcept
{
	std::string buffer;
	const size_t size = _defines.size();
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_defines[i].get() != nullptr, u8"_defines[%zu]가 nullptr 여선 안됩니다.", i);
		MCF_DEBUG_ASSERT(_defines[i]->GetType() == Type::ASM, u8"_defines[%zu]가 유효하지 않습니다.", i);
		buffer += _defines[i]->Inspect();
	}
	return buffer;
}

mcf::IR::Return::Return(mcf::IR::Expression::Pointer&& returnExpression) noexcept
	: _returnExpression(std::move(returnExpression))
{
	MCF_DEBUG_ASSERT(_returnExpression.get() && _returnExpression->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"_returnExpression 표현식이 유효하지 않습니다.");
}

const std::string mcf::IR::Return::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

mcf::IR::While::While(mcf::IR::Expression::Pointer&& condition, mcf::IR::PointerVector&& block, _Notnull_ const mcf::Object::Scope* const blockScope) noexcept
	: _condition(std::move(condition))
	, _block(std::move(block))
	, _blockScope(blockScope)
{
	MCF_DEBUG_ASSERT(_condition.get() && _condition->GetExpressionType() != mcf::IR::Expression::Type::INVALID, u8"_condition 표현식이 유효하지 않습니다.");
#if defined(_DEBUG)
	const size_t size = _block.size();
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_block[i].get() && _block[i]->GetType() != mcf::IR::Type::INVALID, u8"_block[%zu] 표현식이 유효하지 않습니다.", i);
	}
#endif
}

const std::string mcf::IR::While::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

const std::string mcf::IR::Break::Inspect(void) const noexcept
{
	MCF_DEBUG_BREAK(u8"중간 단계 오브젝트입니다. FunctionIRGenerator 제너레이터에 의해 변환되어야 합니다.");
	return std::string();
}

mcf::IR::Program::Program(PointerVector&& objects) noexcept
	: _objects(std::move(objects))
{
#if defined(_DEBUG)
	const size_t size = _objects.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_objects에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_objects[i].get() != nullptr, u8"_objects[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Program::Inspect(void) const noexcept
{
	std::string buffer;
	const size_t size = _objects.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_objects에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_objects[i].get() != nullptr, u8"_objects[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += (i == 0 ? "" : "\n") + _objects[i]->Inspect();
	}
	return buffer;
}