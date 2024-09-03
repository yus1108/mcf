#include "pch.h"
#include "evaluator.h"
#include "object.h"

namespace mcf
{
	namespace IR
	{
		namespace ASM
		{
			namespace Internal
			{
				static const bool IsRegister64Bit(const mcf::IR::ASM::Register reg)
				{
					// 8바이트 레지스터인지 검증
					constexpr const size_t REGISTER_COUNT_BEGIN = __COUNTER__;
					switch (reg)
					{
					case Register::RAX: __COUNTER__; [[fallthrough]];
					case Register::RBX: __COUNTER__; [[fallthrough]];
					case Register::RCX: __COUNTER__; [[fallthrough]];
					case Register::RDX: __COUNTER__; [[fallthrough]];
					case Register::R8: __COUNTER__; [[fallthrough]];
					case Register::R9: __COUNTER__; [[fallthrough]];
					case Register::RSP: __COUNTER__; [[fallthrough]];
					case Register::RBP: __COUNTER__;
						return true;

					default:
						break;
					}
					constexpr const size_t REGISTER_COUNT = __COUNTER__ - REGISTER_COUNT_BEGIN;
					static_assert(static_cast<size_t>(mcf::IR::ASM::Register::COUNT) == REGISTER_COUNT, "register count is changed. this SWITCH need to be changed as well.");
					return false;
				}
			}
		}
	}
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

const size_t mcf::Object::TypeInfo::GetSize( void ) const noexcept
{
	DebugAssert(IsVariadic == false, u8"variadic 은 사이즈를 알 수 없습니다.");
	size_t totalSize = IntrinsicSize;
	const size_t vectorCount = ArraySizeList.size();
	for (size_t i = 0; i < vectorCount; ++i)
	{
		totalSize *= ArraySizeList[i];
	}
	return totalSize;
}

const std::string mcf::Object::TypeInfo::Inspect( void ) const noexcept
{
	DebugAssert(IsValid(), u8"TypeInfo가 유효하지 않습니다.");
	std::string buffer = std::string(IsUnsigned ? "unsigned " : "") + Name;
	const size_t arraySize = ArraySizeList.size();
	for (size_t i = 0; i < arraySize; ++i)
	{
		DebugAssert(ArraySizeList[i] >= 0, u8"ArraySizeList[%zu]는 0이상 이어야 합니다. 값=%zu", i, ArraySizeList[i]);
		buffer += "[" + std::to_string(ArraySizeList[i]) + "]";
	}
	return buffer;
}

const std::string mcf::Object::Variable::Inspect(void) const noexcept
{
	DebugAssert(IsValid(), u8"Variable이 유효하지 않습니다.");
	return Name + " " + DataType.Inspect();
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

const mcf::Object::TypeInfo mcf::Object::Scope::FindTypeInfo(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

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
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(variable.IsValid(), u8"변수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		DebugMessage(u8"구현 필요");
		return mcf::Object::VariableInfo();
	}

	_allIdentifierSet.emplace(name);
	_variables.emplace(name, variable);

	return { variable, _parent == nullptr };
}

const mcf::Object::VariableInfo mcf::Object::Scope::FindVariableInfo(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

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
		auto infoFound = _variables.find( name );
		if (infoFound == _variables.end())
		{
			if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
			{
				DebugMessage(u8"해당 식별자는 변수가 아닙니다.");
				return false;
			}
			currentScope = currentScope->_parent;
			continue;
		}

		infoFound->second.IsUsed = true;
		return true;
	}
	
	DebugMessage(u8"해당 식별자를 가지고 있는 변수를 찾을 수 없습니다.");
	return false;
}

const bool mcf::Object::Scope::MakeLocalScopeToFunctionInfo(_Inout_ mcf::Object::FunctionInfo& info) noexcept
{
	DebugAssert(info.Name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(info.LocalScope == nullptr, u8"로컬 스코프가 이미 할당 되어 있습니다.");

	_tree->Locals.emplace_back(std::make_unique<Scope>(_tree, this));
	if (_tree->Locals.back() == nullptr)
	{
		DebugMessage(u8"구현 필요");
		return false;
	}

	if (info.Params.IsVoid() == false)
	{
		const size_t paramCount = info.Params.Variables.size();
		for (size_t i = 0; i < paramCount; ++i)
		{
			_tree->Locals.back()->DefineVariable(info.Params.Variables[i].Name, info.Params.Variables[i]);
		}
	}

	info.LocalScope = _tree->Locals.back().get();
	return true;
}

const bool mcf::Object::Scope::DefineFunction(const std::string& name, mcf::Object::FunctionInfo info) noexcept
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

const mcf::Object::FunctionInfo mcf::Object::Scope::FindFunction(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

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

mcf::IR::Expression::TypeIdentifier::TypeIdentifier(const mcf::Object::TypeInfo& info) noexcept
	: _info(info)
{
	DebugAssert(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

mcf::IR::Expression::GlobalVariableIdentifier::GlobalVariableIdentifier(const mcf::Object::Variable& variable) noexcept
	: _variable(variable)
{
	DebugAssert(_variable.IsValid(), u8"_variable가 유효하지 않습니다.");
}

mcf::IR::Expression::LocalVariableIdentifier::LocalVariableIdentifier(const mcf::Object::Variable& info) noexcept
	: _info(info)
{
	DebugAssert(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::LocalVariableIdentifier::Inspect(void) const noexcept
{
	DebugMessage(u8"구현 필요");
	return std::string();
}

mcf::IR::Expression::FunctionIdentifier::FunctionIdentifier(const mcf::Object::FunctionInfo& info) noexcept
	: _info(info)
{
	DebugAssert(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::FunctionIdentifier::Inspect(void) const noexcept
{
	DebugMessage(u8"구현 필요");
	return std::string();
}

const size_t mcf::IR::Expression::Integer::GetSize(void) const noexcept
{
	constexpr unsigned __int8 MAX_BYTES = 8;
	unsigned __int8 bitsPerByte = 8;
	for (unsigned __int8 sizeCounter = 1; sizeCounter < MAX_BYTES; sizeCounter++)
	{
		if (_unsignedValue <= mcf::Internal::UInt64::Pow(2, bitsPerByte * sizeCounter) - 1)
		{
			return sizeCounter;
		}
	}
	return MAX_BYTES;
}

const unsigned __int64 mcf::IR::Expression::Integer::GetUInt64(void) const noexcept
{
	DebugAssert(IsUnsignedValue(), u8"unsigned가 아닌 값은 UInt64 타입의 값이 될 수 없습니다.");
	return _unsignedValue;
}

const __int32 mcf::IR::Expression::Integer::GetInt32(void) const noexcept
{
	DebugAssert(IsUnsignedValue(), u8"signed가 아닌 값은 int32 타입의 값이 될 수 없습니다.");
	DebugAssert(GetSize() < sizeof(__int32), u8"4바이트 값만 int32 값으로 변환 가능합니다.");
	return static_cast<__int32>(_signedValue);
}

const std::string mcf::IR::Expression::Integer::Inspect(void) const noexcept
{
	return std::to_string(_isUnsigned ? _unsignedValue : _signedValue);
}

mcf::IR::Expression::Initializer::Initializer(PointerVector&& keyList) noexcept
	: _keyList(std::move(keyList))
{
#if defined(_DEBUG)
	const size_t size = _keyList.size();
	DebugAssert(size != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_keyList[i].get() != nullptr, u8"_keyList[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Expression::Initializer::Inspect(void) const noexcept
{
	const size_t keyListCount = _keyList.size();
	DebugAssert(keyListCount != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");

	std::string buffer;
	buffer = "{ ";
	for (size_t i = 0; i < keyListCount; i++)
	{
		DebugAssert(_keyList[i].get() != nullptr, u8"_keyList[%zu].key는 nullptr 여선 안됩니다.", i);
		buffer += _keyList[i]->Inspect() + ", ";
	}
	return buffer + "}";
}

mcf::IR::ASM::Address::Address(const mcf::Object::TypeInfo& targetType, mcf::IR::ASM::Register targetRegister, const size_t offset)
	: _targetType(targetType)
	, _targetAddress(GetAddressOf(targetRegister, offset))
{
	DebugAssert(targetType.IsValid(), u8"유효하지 않은 타입입니다.");
	DebugAssert(targetType.IsArrayType() == false, u8"배열 타입은 허용되지 않습니다.");
	DebugAssert(targetType.IsVariadic == false, u8"variadic은 허용되지 않습니다.");
	DebugAssert(targetType.GetSize() == targetType.IntrinsicSize, u8"타입의 고유 사이즈와 실제사이즈가 같아야 합니다.");
	DebugAssert(targetRegister != Register::INVALID && targetRegister < Register::COUNT, u8"유효하지 않은 레지스터 값입니다.");
}

const std::string mcf::IR::ASM::Address::GetAddressOf(const Register value, const size_t offset) noexcept
{
	return std::string("[") + CONVERT_REGISTER_TO_STRING(value) + std::string(offset < 0 ? " - " : " + ") + std::to_string(offset) + "]";
}

const std::string mcf::IR::ASM::Address::Inspect(void) const noexcept
{
	return _targetType.Inspect() + " ptr " + _targetAddress;
}

mcf::IR::ASM::ProcBegin::ProcBegin(const std::string& name) noexcept
	: _name(name)
{
	DebugAssert(_name.empty() == false, u8"프로시져의 이름이 반드시 존재해야 합니다.");
}

const std::string mcf::IR::ASM::ProcBegin::Inspect(void) const noexcept
{
	return _name + " proc\n";
}

mcf::IR::ASM::ProcEnd::ProcEnd(const std::string& name) noexcept
	: _name( name )
{
	DebugAssert(_name.empty() == false, u8"프로시져의 이름이 반드시 존재해야 합니다.");
}

const std::string mcf::IR::ASM::ProcEnd::Inspect(void) const noexcept
{
	return _name + " endp\n";
}

mcf::IR::ASM::Push::Push(const Register address) noexcept
	: _value(CONVERT_REGISTER_TO_STRING(address))
{
}

const std::string mcf::IR::ASM::Push::Inspect(void) const noexcept
{
	return "\tpush " + _value + "\n";
}

mcf::IR::ASM::Pop::Pop(const Register target) noexcept
	: _target(CONVERT_REGISTER_TO_STRING(target))
{
}

const std::string mcf::IR::ASM::Pop::Inspect(void) const noexcept
{
	return "\tpop " + _target + "\n";
}

mcf::IR::ASM::Mov::Mov(const Address& target, const Register source) noexcept
	: _target(target.Inspect())
	, _source(mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(source))
{
#if defined(_DEBUG)
	// 8바이트 레지스터인지 검증
	constexpr const size_t REGISTER_COUNT_BEGIN = __COUNTER__;
	switch (source)
	{
	case Register::RAX: __COUNTER__; [[fallthrough]];
	case Register::RBX: __COUNTER__; [[fallthrough]];
	case Register::RCX: __COUNTER__; [[fallthrough]];
	case Register::RDX: __COUNTER__; [[fallthrough]];
	case Register::R8: __COUNTER__; [[fallthrough]];
	case Register::R9: __COUNTER__; [[fallthrough]];
	case Register::RSP: __COUNTER__; [[fallthrough]];
	case Register::RBP: __COUNTER__;
		if (target.GetTypeInfo().GetSize() != sizeof(__int64))
		{
			DebugMessage(u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
		}
		break;

	default:
		DebugBreak(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. Register=%s(%zu)", mcf::IR::ASM::CONVERT_REGISTER_TO_STRING(source), mcf::ENUM_INDEX(source));
	}
	constexpr const size_t REGISTER_COUNT = __COUNTER__ - REGISTER_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::IR::ASM::Register::COUNT) == REGISTER_COUNT, "register count is changed. this SWITCH need to be changed as well.");
#endif
}

mcf::IR::ASM::Mov::Mov(const Address& target, const unsigned __int64 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	DebugAssert(target.GetTypeInfo().GetSize() >= sizeof(unsigned __int64), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	DebugAssert(target.GetTypeInfo().IsUnsigned == true, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

mcf::IR::ASM::Mov::Mov(const Address& target, const __int32 source) noexcept
	: _target(target.Inspect())
	, _source(std::to_string(source))
{
	DebugAssert(target.GetTypeInfo().GetSize() >= sizeof(__int32), u8"source와 target의 사이즈가 일치 하지 않습니다. Size=%zu", target.GetTypeInfo().GetSize());
	DebugAssert(target.GetTypeInfo().IsUnsigned == false, u8"source와 target의 사인 타입이 일치 하지 않습니다.");
}

const std::string mcf::IR::ASM::Mov::Inspect(void) const noexcept
{
	return "\tmov " + _target + ", " + _source + "\n";
}

mcf::IR::ASM::Sub::Sub(const Register minuend, const __int64 subtrahend) noexcept
	: _minuend(CONVERT_REGISTER_TO_STRING(minuend))
	, _subtrahend(std::to_string(subtrahend))
{
	DebugAssert(Internal::IsRegister64Bit(minuend), u8"64비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Sub::Sub(const Register minuend, const unsigned __int64 subtrahend) noexcept
	: _minuend(CONVERT_REGISTER_TO_STRING(minuend))
	, _subtrahend(std::to_string(subtrahend))
{
	DebugAssert(Internal::IsRegister64Bit(minuend), u8"64비트 레지스터가 아닙니다.");
}

const std::string mcf::IR::ASM::Sub::Inspect(void) const noexcept
{
	return "\tsub " + _minuend + ", " + _subtrahend + "\n";
}

mcf::IR::ASM::Add::Add(const Register lhs, const __int64 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	DebugAssert(Internal::IsRegister64Bit(lhs), u8"64비트 레지스터가 아닙니다.");
}

mcf::IR::ASM::Add::Add(const Register lhs, const unsigned __int64 rhs) noexcept
	: _lhs(CONVERT_REGISTER_TO_STRING(lhs))
	, _rhs(std::to_string(rhs))
{
	DebugAssert(Internal::IsRegister64Bit(lhs), u8"64비트 레지스터가 아닙니다.");
}

const std::string mcf::IR::ASM::Add::Inspect(void) const noexcept
{
	return "\tadd " + _lhs + ", " + _rhs + "\n";
}

mcf::IR::IncludeLib::IncludeLib(const std::string& libPath) noexcept
	: _libPath(libPath)
{}

const std::string mcf::IR::IncludeLib::Inspect(void) const noexcept
{
	return "includelib " + _libPath;
}

mcf::IR::Extern::Extern(const std::string& name, const std::vector<mcf::Object::TypeInfo>& params, const bool hasVariadic) noexcept
	: _name(name)
	, _params(params)
	, _hasVariadic(hasVariadic)
{
#if defined(_DEBUG)
	const size_t size = _params.size();
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_params[i].IsValid(), u8"_params[%zu]가 유효하지 않습니다.", i);
		const size_t arraySize = _params[i].ArraySizeList.size();
		for (size_t j = 0; j < arraySize; ++j)
		{
			DebugAssert(_params[i].ArraySizeList[j] > 0, u8"_params[%zu].ArraySizeList[%zu]는 0이상 이어야 합니다. 값=%zu", i, j, _params[i].ArraySizeList[j]);
		}
	}
#endif
}

const std::string mcf::IR::Extern::Inspect(void) const noexcept
{
	std::string buffer = _name + " PROTO";
	if (_params.empty())
	{
		return _hasVariadic ? (buffer + " : VARARG") : buffer;
	}

	const size_t size = _params.size();
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_params[i].IsValid(), u8"_params[%zu]가 valid 하지 않습니다.", i);
		buffer += std::string(i == 0 ? " : " : ", ") + _params[i].Inspect();
	}
	return _hasVariadic ? (buffer + ", VARARG") : buffer;
}

mcf::IR::Let::Let(const mcf::Object::VariableInfo&& info, mcf::IR::Expression::Pointer&& assignedExpression) noexcept
	: _info(info)
	, _assignExpression(std::move(assignedExpression))
{
	DebugAssert(_info.IsValid(), u8"변수 정보가 유효하지 않습니다.");
}

const std::string mcf::IR::Let::Inspect(void) const noexcept
{
	std::string buffer;
	if (_info.IsGlobal)
	{
		buffer = _info.Variable.Inspect() + " ";
		return buffer + (_assignExpression.get() == nullptr ? "?" : _assignExpression->Inspect());
	}
	else
	{
		DebugMessage(u8"구현 필요");
	}
	return std::string();
}

mcf::IR::Func::Func(PointerVector&& defines) noexcept
	: _defines(std::move(defines))
{
#if defined(_DEBUG)
	const size_t size = _defines.size();
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_defines[i].get() != nullptr, u8"_body[%zu]가 nullptr 여선 안됩니다.", i);
		DebugAssert(_defines[i]->GetType() == Type::ASM, u8"_body[%zu]가 유효하지 않습니다.", i);
	}
#endif
}

const std::string mcf::IR::Func::Inspect(void) const noexcept
{
	std::string buffer;
	const size_t size = _defines.size();
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_defines[i].get() != nullptr, u8"_body[%zu]가 nullptr 여선 안됩니다.", i);
		DebugAssert(_defines[i]->GetType() == Type::ASM, u8"_body[%zu]가 유효하지 않습니다.", i);
		buffer += _defines[i]->Inspect();
	}
	return buffer;
}

mcf::IR::Program::Program(PointerVector&& objects) noexcept
	: _objects(std::move(objects))
{
#if defined(_DEBUG)
	const size_t size = _objects.size();
	DebugAssert(size != 0, u8"_objects에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_objects[i].get() != nullptr, u8"_objects[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Program::Inspect(void) const noexcept
{
	std::string buffer;
	const size_t size = _objects.size();
	DebugAssert(size != 0, u8"_objects에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_objects[i].get() != nullptr, u8"_objects[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += (i == 0 ? "" : "\n") + _objects[i]->Inspect();
	}
	return buffer;
}