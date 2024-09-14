#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <common.h>

namespace mcf
{
	namespace Object
	{
		using Data = std::vector<unsigned __int8>;

		// if any item in ArraySizeList has the value as 0, it means it's unknown
		struct TypeInfo final
		{
			std::vector<size_t> ArraySizeList;
			std::string Name;
			size_t IntrinsicSize = 0;
			bool IsStruct = false;
			bool IsUnsigned = false;
			bool IsVariadic = false;

			static const mcf::Object::TypeInfo GetVoidTypeInfo(void) { return mcf::Object::TypeInfo(); }
			static const mcf::Object::TypeInfo MakePrimitive(const bool isUnsigned, const std::string& name, const size_t size) 
			{ 
				return 
				{ 
					std::vector<size_t>(),	// ArraySizeList
					name,					// Name
					size, 					// IntrinsicSize
					false, 					// IsStruct
					isUnsigned, 			// IsUnsigned
					false 					// IsVariadic
				};
			}

			inline const bool IsValid(void) const noexcept { return (Name.empty() == false && (IsUnsigned == false || IsStruct == false)) || IsVariadic; }
			inline const bool IsIntegerType(void) const noexcept { return IsArrayType() == false && IsStruct == false && IsVariadic == false; }
			inline const bool IsCompatibleAddressType(void) const noexcept { return IsUnsigned && IsIntegerType() && IntrinsicSize == sizeof(size_t); }

			inline const bool IsArrayType(void) const noexcept { return ArraySizeList.empty() == false; }
			inline const bool IsArraySizeUnknown(const size_t arrayIndex) const noexcept { return IsArrayType() && ArraySizeList[arrayIndex] == 0; }
			inline const bool IsLastArrayDimensionSizeUnknown() const noexcept { return IsArrayType() && ArraySizeList[ArraySizeList.size() - 1] == 0; }
			inline const bool IsStringCompatibleType(void) const noexcept { return ArraySizeList.size() == 1 && IntrinsicSize == 1; }
			const bool HasUnknownArrayIndex(void) const noexcept;

			const bool IsStaticCastable(const TypeInfo& typeToCast) const noexcept;

			const size_t GetSize(void) const noexcept;
			const std::string Inspect(void) const noexcept;
		};
		inline bool operator==(const TypeInfo& lhs, const TypeInfo& rhs) 
		{ 
			return (lhs.ArraySizeList == rhs.ArraySizeList) &&
				(lhs.Name == rhs.Name) &&
				(lhs.IntrinsicSize == rhs.IntrinsicSize) &&
				(lhs.IsStruct == rhs.IsStruct) &&
				(lhs.IsUnsigned == rhs.IsUnsigned) &&
				(lhs.IsVariadic == rhs.IsVariadic);
		}
		inline bool operator!=(const TypeInfo& lhs, const TypeInfo& rhs) 
		{ 
			return (lhs == rhs) == false;
		}

		struct Variable final
		{
			std::string Name;
			TypeInfo DataType;
			bool IsUsed = false;

			inline const bool IsValid(void) const noexcept { return Name.empty() == false && DataType.IsValid(); }
			inline const bool IsVariadic(void) const noexcept { return DataType.IsVariadic; }
			const std::string Inspect(void) const noexcept;
			const size_t GetTypeSize(void) const noexcept { return DataType.GetSize(); }
		};

		struct VariableInfo final
		{
			Variable Variable;
			bool IsGlobal = false;

			inline const bool IsValid(void) const noexcept { return Variable.IsValid(); }
		};

		struct FunctionParams final
		{
			std::vector<Variable> Variables;

			inline const bool IsVoid(void) const noexcept { return Variables.empty(); }
			inline const bool HasVariadic(void) const noexcept { return IsVoid() == false && Variables.back().IsVariadic(); }

			const size_t GetVariadicIndex(void) const noexcept;
		};

		class Scope;
		struct FunctionInfo final
		{
			std::string Name;
			FunctionParams Params;
			TypeInfo ReturnType;
			union
			{
				Scope* LocalScope = nullptr;
				bool IsExternal;
			} Definition;

			inline const bool IsValid(void) const noexcept { return Name.empty() == false && Definition.LocalScope != nullptr; }
			inline const bool IsReturnTypeVoid(void) const noexcept { return ReturnType.IsValid() == false; }
			
		};

		enum class InternalFunctionType : unsigned char
		{
			INVALID = 0,

			COPY_MEMORY,

			// 이 밑으로는 수정하면 안됩니다.
			COUNT,
		};

		constexpr const char* INTERNAL_FUNCTION_TYPE_STRING_ARRAY[] =
		{
			"INVALID",

			"?CopyMemory",
		};
		constexpr const size_t INTERNAL_FUNCTION_TYPES_SIZE = MCF_ARRAY_SIZE(INTERNAL_FUNCTION_TYPE_STRING_ARRAY);
		static_assert(mcf::ENUM_COUNT<InternalFunctionType>() == INTERNAL_FUNCTION_TYPES_SIZE, "internal function type count not matching!");

		struct ScopeTree;
		class Scope final
		{
		public:
			explicit Scope(void) noexcept = delete;
			explicit Scope(ScopeTree* tree, Scope* parent, bool isFunctionScope) noexcept : _tree(tree), _parent(parent), _isFunctionScope(isFunctionScope) {}

			const bool IsGlobalScope(void) const noexcept;
			inline const bool IsFunctionScope(void) const noexcept { return _isFunctionScope; }
			const bool IsIdentifierRegistered(const std::string& name) const noexcept;

			inline ScopeTree* GetUnsafeScopeTreePointer(void) noexcept { return _tree;}
			inline const ScopeTree* GetUnsafeScopeTreePointer(void) const noexcept { return _tree;}

			const bool DefineType(const std::string& name, const mcf::Object::TypeInfo& info) noexcept;
			const bool DefineTypeValue(const std::string& typeName, const std::string , const Data& data) noexcept;
			const mcf::Object::TypeInfo FindTypeInfo(const std::string& name) const noexcept;

			const bool IsAllVariablesUsed(void) const noexcept;
			const mcf::Object::VariableInfo DefineVariable(const std::string& name, const mcf::Object::Variable& variable) noexcept;
			const mcf::Object::VariableInfo FindVariableInfo(const std::string& name) const noexcept;
			const bool UseVariableInfo(const std::string& name) noexcept;
			void DetermineUnknownVariableTypeSize(const std::string& name, std::vector<size_t> arraySizeList) noexcept;

			const bool MakeLocalScopeToFunctionInfo(_Inout_ mcf::Object::FunctionInfo& info) noexcept;
			const bool DefineFunction(const std::string& name, const mcf::Object::FunctionInfo& info) noexcept;
			const mcf::Object::FunctionInfo FindFunction(const std::string& name) const noexcept;
			const mcf::Object::FunctionInfo FindInternalFunction(const InternalFunctionType functionType) const noexcept;

		private:
			friend ScopeTree;
			explicit Scope(ScopeTree* tree) noexcept : _tree(tree) {}

		private:
			std::unordered_set<std::string> _allIdentifierSet;
			std::unordered_map<std::string, mcf::Object::TypeInfo> _typeInfoMap;
			std::unordered_map<std::string, mcf::Object::Variable> _variables;
			std::unordered_map<std::string, mcf::Object::FunctionInfo> _functionInfoMap;
			Scope* _parent = nullptr;
			ScopeTree* _tree = nullptr;
			bool _isFunctionScope = false;
		};

		struct ScopeTree final
		{
			Scope Global = Scope(this);
			std::vector<std::unique_ptr<Scope>> Locals;
			std::unordered_map<std::string, std::pair<size_t, Data>> LiteralIndexMap;
			mcf::Object::FunctionInfo InternalFunctionInfosByTypes[mcf::ENUM_COUNT<InternalFunctionType>()] =
			{
				mcf::Object::FunctionInfo(),
				mcf::Object::FunctionInfo
				{
					INTERNAL_FUNCTION_TYPE_STRING_ARRAY[mcf::ENUM_INDEX(InternalFunctionType::COPY_MEMORY)], 
					std::vector<Variable>
					({
						Variable{"src", mcf::Object::TypeInfo::MakePrimitive(true, "qword", 8), false},
						Variable{"dest", mcf::Object::TypeInfo::MakePrimitive(true, "qword", 8), false},
						Variable{"size", mcf::Object::TypeInfo::MakePrimitive(true, "qword", 8), false},
					}),
					mcf::Object::TypeInfo::GetVoidTypeInfo(),
					reinterpret_cast<Scope*>(true), // Definition.IsExternal = true
				}
			};
		};
	}

	namespace IR
	{
		enum class Type : unsigned char
		{
			INVALID = 0,

			EXPRESSION,
			ASM,

			INCLUDELIB,
			TYPEDEF,
			EXTERN,
			LET,
			FUNC,
			UNUSEDIR,
			RETURN,

			PROGRAM,

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		constexpr const char* TYPE_STRING_ARRAY[] =
		{
			"INVALID",

			"EXPRESSION",
			"ASM",

			"INCLUDELIB",
			"TYPEDEF",
			"EXTERN",
			"LET",
			"FUNC",
			"UNUSEDIR",
			"RETURN",

			"PROGRAM",
		};
		constexpr const size_t IR_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
		static_assert(static_cast<size_t>(Type::COUNT) == IR_TYPE_SIZE, "IR type count not matching!");

		constexpr const char* CONVERT_TYPE_TO_STRING(const Type value)
		{
			return TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
		}

		class Interface
		{
		public:
			virtual ~Interface(void) noexcept = default;
			virtual const Type GetType(void) const noexcept = 0;
			virtual const std::string Inspect(void) const noexcept = 0;
		};

		using Pointer = std::unique_ptr<Interface>;
		using PointerVector = std::vector<Pointer>;

		class Invalid : public Interface
		{
		public:
			inline static Pointer Make(void) noexcept { return std::make_unique<Invalid>(); }
			inline virtual const Type GetType(void) const noexcept override final { return Type::INVALID; }
			inline virtual const std::string Inspect(void) const noexcept override final { return "Invalid Object"; }
		};

		namespace Expression
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				TYPE_IDENTIFIER,
				GLOBAL_VARIABLE_IDENTIFIER,
				LOCAL_VARIABLE_IDENTIFIER,
				FUNCTION_IDENTIFIER,
				INTEGER,
				STRING,
				INITIALIZER,
				MAP_INITIALIZER,
				CALL,
				STATIC_CAST,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT
			};

			constexpr const char* TYPE_STRING_ARRAY[] =
			{
				"INVALID",

				"TYPE_IDENTIFIER",
				"GLOBAL_VARIABLE_IDENTIFIER",
				"LOCAL_VARIABLE_IDENTIFIER",
				"FUNCTION_IDENTIFIER",
				"INTEGER",
				"STRING",
				"INITIALIZER",
				"MAP_INITIALIZER",
				"CALL",
				"STATIC_CAST",
			};
			constexpr const size_t EXPRESSION_IR_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
			static_assert(static_cast<size_t>(Type::COUNT) == EXPRESSION_IR_TYPE_SIZE, "expression ir type count not matching!");

			constexpr const char* CONVERT_TYPE_TO_STRING(const Type value)
			{
				return TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
			}

			class Interface : public mcf::IR::Interface
			{
			public:
				virtual const Type GetExpressionType(void) const noexcept = 0;

				static const mcf::Object::TypeInfo GetDatTypeFromExpression(const mcf::IR::Expression::Interface* expression) noexcept;

				inline virtual const mcf::IR::Type GetType(void) const noexcept override final { return mcf::IR::Type::EXPRESSION; }
				virtual const std::string Inspect(void) const noexcept override = 0;
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Invalid : public Interface
			{
			public:
				inline static Pointer Make(void) noexcept { return std::make_unique<Invalid>(); }
				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::INVALID; }
				inline virtual const std::string Inspect(void) const noexcept override final { return "Invalid Expression Object"; }
			};
		}

		namespace ASM
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				RET,
				PROC_BEGIN,
				PROC_END,
				PUSH,
				POP,
				MOV,
				LEA,
				ADD,
				SUB,
				XOR,
				CALL,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT
			};

			constexpr const char* TYPE_STRING_ARRAY[] =
			{
				"INVALID",

				"RET",
				"PROC_BEGIN",
				"PROC_END",
				"PUSH",
				"POP",
				"MOV",
				"LEA",
				"ADD",
				"SUB",
				"XOR",
				"CALL",
			};
			constexpr const size_t ASM_IR_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
			static_assert(static_cast<size_t>(Type::COUNT) == ASM_IR_TYPE_SIZE, "asm ir type count not matching!");
			constexpr const char* CONVERT_TYPE_TO_STRING(const Type value)
			{
				return TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
			}

			enum class Register : unsigned char
			{
				INVALID = 0,

				RAX,
				EAX,
				AX,
				AL,

				RBX,

				RCX,
				ECX,
				CX,
				CL,

				RDX,
				EDX,
				DX,
				DL,

				R8,
				R8D,
				R8W,
				R8B,

				R9,
				R9D,
				R9W,
				R9B,

				RSP,
				ESP,
				SP,
				SPL,

				RBP,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT
			};

			constexpr const char* REGISTER_STRING_ARRAY[] =
			{
				"INVALID",

				"rax",
				"eax",
				"ax",
				"al",

				"rbx",

				"rcx",
				"ecx",
				"cx",
				"cl",

				"rdx",
				"edx",
				"dx",
				"dl",

				"r8",
				"r8d",
				"r8w",
				"r8b",

				"r9",
				"r9d",
				"r9w",
				"r9b",

				"rsp",
				"esp",
				"sp",
				"spl",

				"rbp",
			};
			constexpr const size_t REGISTER_TYPE_SIZE = MCF_ARRAY_SIZE(REGISTER_STRING_ARRAY);
			static_assert(static_cast<size_t>(Register::COUNT) == REGISTER_TYPE_SIZE, "register count not matching!");
			constexpr const char* CONVERT_REGISTER_TO_STRING(const Register value) noexcept
			{
				return REGISTER_STRING_ARRAY[mcf::ENUM_INDEX(value)];
			}

			enum class RegisterSize : unsigned char
			{
				INVALID = 0,

				BYTE,
				WORD,
				DWORD,
				QWORD,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT,
			};

			constexpr const mcf::IR::ASM::RegisterSize GET_REGISTER_SIZE(Register reg) noexcept
			{
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
					return mcf::IR::ASM::RegisterSize::QWORD;

				case Register::EAX: __COUNTER__; [[fallthrough]];
				case Register::ECX: __COUNTER__; [[fallthrough]];
				case Register::EDX: __COUNTER__; [[fallthrough]];
				case Register::R8D: __COUNTER__; [[fallthrough]];
				case Register::R9D: __COUNTER__; [[fallthrough]];
				case Register::ESP: __COUNTER__;
					return mcf::IR::ASM::RegisterSize::DWORD;

				case Register::AX: __COUNTER__; [[fallthrough]];
				case Register::CX: __COUNTER__; [[fallthrough]];
				case Register::DX: __COUNTER__; [[fallthrough]];
				case Register::R8W: __COUNTER__; [[fallthrough]];
				case Register::R9W: __COUNTER__; [[fallthrough]];
				case Register::SP: __COUNTER__;
					return mcf::IR::ASM::RegisterSize::WORD;

				case Register::AL: __COUNTER__; [[fallthrough]];
				case Register::CL: __COUNTER__; [[fallthrough]];
				case Register::DL: __COUNTER__; [[fallthrough]];
				case Register::R8B: __COUNTER__; [[fallthrough]];
				case Register::R9B: __COUNTER__; [[fallthrough]];
				case Register::SPL: __COUNTER__;
					return mcf::IR::ASM::RegisterSize::BYTE;

				default:
					break;
				}
				constexpr const size_t REGISTER_COUNT = __COUNTER__ - REGISTER_COUNT_BEGIN;
				static_assert(static_cast<size_t>(mcf::IR::ASM::Register::COUNT) == REGISTER_COUNT, "register count is changed. this SWITCH need to be changed as well.");
				return mcf::IR::ASM::RegisterSize::INVALID;
			}

			constexpr const size_t REGISTER_SIZE_VALUE_ARRAY[mcf::ENUM_COUNT<RegisterSize>()] =
			{
				0,
				sizeof(__int8),
				sizeof(__int16),
				sizeof(__int32),
				sizeof(__int64),
			};
			constexpr const size_t GET_REGISTER_SIZE_VALUE(const mcf::IR::ASM::Register reg) { return REGISTER_SIZE_VALUE_ARRAY[mcf::ENUM_INDEX(GET_REGISTER_SIZE(reg))]; }
			constexpr const mcf::IR::ASM::RegisterSize GET_REGISTER_SIZE_BY_VALUE(const size_t value)
			{ 
				switch (value)
				{
				case sizeof(__int8) :
					return mcf::IR::ASM::RegisterSize::BYTE;
				case sizeof(__int16) :
					return mcf::IR::ASM::RegisterSize::WORD;
				case sizeof(__int32) :
					return mcf::IR::ASM::RegisterSize::DWORD;
				case sizeof(__int64) :
					return mcf::IR::ASM::RegisterSize::QWORD;
				default:
					return mcf::IR::ASM::RegisterSize::INVALID;
				}
			}

			class Interface : public mcf::IR::Interface
			{
			public:
				virtual const Type GetASMType( void ) const noexcept = 0;

				inline virtual const mcf::IR::Type GetType( void ) const noexcept override final { return mcf::IR::Type::ASM; }
				virtual const std::string Inspect( void ) const noexcept override = 0;
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Invalid : public Interface
			{
			public:
				inline static Pointer Make( void ) noexcept { return std::make_unique<Invalid>(); }
				inline virtual const Type GetASMType( void ) const noexcept override final { return Type::INVALID; }
				inline virtual const std::string Inspect( void ) const noexcept override final { return "Invalid Expression Object"; }
			};
		}

		namespace Expression
		{
			class TypeIdentifier final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<TypeIdentifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<TypeIdentifier>(std::move(args)...); }

			public:
				explicit TypeIdentifier(void) noexcept = default;
				explicit TypeIdentifier(const mcf::Object::TypeInfo& info) noexcept;

				const mcf::Object::TypeInfo& GetInfo(void) const noexcept { return _info; }

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::TYPE_IDENTIFIER; }
				inline virtual const std::string Inspect(void) const noexcept override final { return _info.Inspect(); }

			private:
				mcf::Object::TypeInfo _info;
			};

			class GlobalVariableIdentifier final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<GlobalVariableIdentifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<GlobalVariableIdentifier>(std::move(args)...); }

			public:
				explicit GlobalVariableIdentifier(void) noexcept = default;
				explicit GlobalVariableIdentifier(const mcf::Object::Variable& variable) noexcept;

				mcf::Object::Variable& GetVariable(void) noexcept { return _variable; }
				const mcf::Object::Variable& GetVariable(void) const noexcept { return _variable;}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::GLOBAL_VARIABLE_IDENTIFIER; }
				inline virtual const std::string Inspect(void) const noexcept override final { return _variable.Inspect(); }

			private:
				mcf::Object::Variable _variable;
			};

			class LocalVariableIdentifier final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<LocalVariableIdentifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<LocalVariableIdentifier>(std::move(args)...); }

			public:
				explicit LocalVariableIdentifier(void) noexcept = default;
				explicit LocalVariableIdentifier(const mcf::Object::Variable& variable) noexcept;

				mcf::Object::Variable& GetVariable(void) noexcept { return _variable;}
				const mcf::Object::Variable& GetVariable(void) const noexcept { return _variable;}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::LOCAL_VARIABLE_IDENTIFIER; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				mcf::Object::Variable _variable;
			};

			class FunctionIdentifier final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<FunctionIdentifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<FunctionIdentifier>(std::move(args)...); }

			public:
				explicit FunctionIdentifier(void) noexcept = default;
				explicit FunctionIdentifier(const mcf::Object::FunctionInfo& info) noexcept;

				inline const mcf::Object::FunctionInfo& GetInfo(void) const noexcept { return _info;}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::FUNCTION_IDENTIFIER; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				mcf::Object::FunctionInfo _info;
			};

			class Integer final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Integer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Integer>(std::move(args)...); }

			private:
				inline const bool IsUnsigned(void) const noexcept { return _isUnsigned; }

			public:
				explicit Integer(void) noexcept = default;
				explicit Integer(const __int64 value) noexcept : _signedValue(value), _isUnsigned(false){}
				explicit Integer(const unsigned __int64 value) noexcept : _unsignedValue(value), _isUnsigned(true){}

				inline const bool IsNaturalInteger(void) const noexcept { return _isUnsigned ? true : (_signedValue >= 0); }
				const bool IsCompatible(const mcf::Object::TypeInfo& dataType) const noexcept;
				const bool IsZero(void) const noexcept { return _unsignedValue == 0; }

				inline const bool IsInt64(void) const noexcept { return _isUnsigned == false; }
				const __int64 GetInt64(void) const noexcept;
				inline const bool IsInt32(void) const noexcept { return (_isUnsigned == false && INT32_MIN <= _signedValue && _signedValue <= INT32_MAX) || (_isUnsigned == true && _unsignedValue <= INT32_MAX); }
				const __int32 GetInt32(void) const noexcept;
				inline const bool IsInt16(void) const noexcept { return (_isUnsigned == false && INT16_MIN <= _signedValue && _signedValue <= INT16_MAX) || (_isUnsigned == true && _unsignedValue <= INT16_MAX); }
				const __int16 GetInt16(void) const noexcept;
				inline const bool IsInt8(void) const noexcept { return (_isUnsigned == false && INT8_MIN <= _signedValue && _signedValue <= INT8_MAX) || (_isUnsigned == true && _unsignedValue <= INT8_MAX); }
				const __int8 GetInt8(void) const noexcept;

				inline const bool IsUInt64(void) const noexcept { return IsNaturalInteger(); }
				const unsigned __int64 GetUInt64(void) const noexcept;
				inline const bool IsUInt32(void) const noexcept { return (_isUnsigned == false && 0 <= _signedValue && _signedValue <= UINT32_MAX) || (_isUnsigned == true && _unsignedValue <= UINT32_MAX); }
				const unsigned __int32 GetUInt32(void) const noexcept;
				inline const bool IsUInt16(void) const noexcept { return (_isUnsigned == false && 0 <= _signedValue && _signedValue <= UINT16_MAX) || (_isUnsigned == true && _unsignedValue <= UINT16_MAX); }
				const unsigned __int16 GetUInt16(void) const noexcept;
				inline const bool IsUInt8(void) const noexcept { return (_isUnsigned == false && 0 <= _signedValue && _signedValue <= UINT8_MAX) || (_isUnsigned == true && _unsignedValue <= UINT8_MAX); }
				const unsigned __int8 GetUInt8(void) const noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::INTEGER; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				union
				{
					unsigned __int64 _unsignedValue;
					__int64 _signedValue;
				};
				bool _isUnsigned = false;
			};

			class String final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<String>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<String>(std::move(args)...); }

			public:
				explicit String(void) noexcept = default;
				explicit String(const size_t index, const size_t size) noexcept : _index(index), _size(size) {}

				const size_t GetIndex(void) const noexcept { return _index;}
				const size_t GetSize(void) const noexcept { return _size;}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::STRING; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				const size_t _index;
				const size_t _size;
			};

			class Initializer : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Initializer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Initializer>(std::move(args)...); }

			public:
				explicit Initializer(void) noexcept = default;
				explicit Initializer(PointerVector&& keyList) noexcept;

				inline virtual const size_t GetKeyExpressionCount(void) const noexcept final { return _keyList.size(); }
				inline virtual mcf::IR::Expression::Interface* GetUnsafeKeyExpressionPointerAt(const size_t index) noexcept final
				{
					return _keyList[index].get();
				}
				inline virtual const mcf::IR::Expression::Interface* GetUnsafeKeyExpressionPointerAt(const size_t index) const noexcept final
				{
					return _keyList[index].get();
				}

				inline virtual const Type GetExpressionType(void) const noexcept override { return Type::INITIALIZER; }
				virtual const std::string Inspect(void) const noexcept override;

			private:
				PointerVector _keyList;
			};

			class MapInitializer : public Initializer
			{
			public:
				using Pointer = std::unique_ptr<MapInitializer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<MapInitializer>(std::move(args)...); }

			public:
				explicit MapInitializer(void) noexcept = default;
				explicit MapInitializer(PointerVector&& keyList, PointerVector&& valueList) noexcept;

				inline const size_t GetValueExpressionCount(void) const noexcept { return _valueList.size(); }
				inline mcf::IR::Expression::Interface* GetUnsafeValueExpressionPointerAt(const size_t index) noexcept
				{
					return _valueList[index].get();
				}
				inline const mcf::IR::Expression::Interface* GetUnsafeValueExpressionPointerAt(const size_t index) const noexcept
				{
					return _valueList[index].get();
				}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::MAP_INITIALIZER; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				PointerVector _valueList;
			};

			class Call final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Call>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Call>(std::move(args)...); }

			public:
				explicit Call(void) noexcept = default;
				explicit Call(const mcf::Object::FunctionInfo& info, mcf::IR::Expression::PointerVector&& paramObjects) noexcept;

				inline const mcf::Object::FunctionInfo& GetInfo(void) const noexcept { return _info; }
				inline const size_t GetParamCount(void) const noexcept { return _paramObjects.size(); }
				inline mcf::IR::Expression::Interface* GetUnsafeParamPointerAt(const size_t index) noexcept
				{
					return _paramObjects[index].get();
				}
				inline const mcf::IR::Expression::Interface* GetUnsafeParamPointerAt(const size_t index) const noexcept
				{
					return _paramObjects[index].get();
				}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::CALL; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				mcf::Object::FunctionInfo _info;
				mcf::IR::Expression::PointerVector _paramObjects;
			};

			class StaticCast final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<StaticCast>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<StaticCast>(std::move(args)...); }

			public:
				explicit StaticCast(void) noexcept = default;
				explicit StaticCast(mcf::IR::Expression::Pointer&& castingValue, const mcf::Object::TypeInfo& castedType) noexcept;

				inline const mcf::Object::TypeInfo GetOriginalDatType(void) const noexcept { return GetDatTypeFromExpression(_castingValue.get()); }
				inline const mcf::Object::TypeInfo GetCastedDatType(void) const noexcept { return _castedType; }
				inline mcf::IR::Expression::Interface* GetUnsafeOriginalExpressionPointer(void) const noexcept { return _castingValue.get(); }

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::STATIC_CAST; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				mcf::IR::Expression::Pointer _castingValue;
				mcf::Object::TypeInfo _castedType;
			};
		}

		namespace ASM
		{
			class Address final
			{
			public:
				explicit Address(void) noexcept = default;
				explicit Address(const mcf::Object::TypeInfo& targetType, mcf::IR::ASM::Register targetRegister, const size_t offset);

				const std::string Inspect(void) const noexcept;
				inline const mcf::Object::TypeInfo GetTypeInfo(void) const noexcept { return _targetType; }

			private:
				mcf::Object::TypeInfo _targetType;
				std::string _targetAddress;
			};

			class UnsafePointerAddress final
			{
			public:
				explicit UnsafePointerAddress(void) noexcept = default;
				explicit UnsafePointerAddress(_Notnull_ const mcf::IR::Expression::String* stringExpression);
				explicit UnsafePointerAddress(mcf::IR::ASM::Register targetRegister, const size_t offset);
				explicit UnsafePointerAddress(const UnsafePointerAddress& targetAddress, const size_t offset);

				const std::string Inspect(void) const noexcept;

			private:
				std::string _identifier;
				size_t _offset = 0;
				bool _hasOffset = false;
			};

			class SizeOf final
			{
			public:
				explicit SizeOf(void) noexcept = default;
				explicit SizeOf(const mcf::IR::Expression::String* stringExpression);

				inline const std::string Inspect(void) const noexcept { return _targetSize; }

			private:
				std::string _targetSize;
			};

			class Ret : public Interface
			{
			public:
				inline static Pointer Make(void) noexcept { return std::make_unique<Ret>(); }
				inline virtual const Type GetASMType(void) const noexcept override final { return Type::RET; }
				inline virtual const std::string Inspect(void) const noexcept override final { return "\tret\n"; }
			};

			class ProcBegin : public Interface
			{
			public:
				using Pointer = std::unique_ptr<ProcBegin>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<ProcBegin>(std::move(args)...); }

			public:
				explicit ProcBegin(void) noexcept = default;
				explicit ProcBegin(const std::string& name) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::PROC_BEGIN; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _name;
			};

			class ProcEnd : public Interface
			{
			public:
				using Pointer = std::unique_ptr<ProcEnd>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<ProcEnd>(std::move(args)...); }

			public:
				explicit ProcEnd(void) noexcept = default;
				explicit ProcEnd(const std::string& name) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::PROC_END; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _name;
			};

			class Push : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Push>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Push>(std::move(args)...); }

			public:
				explicit Push(void) noexcept = default;
				explicit Push(const Register address) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::PUSH; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _value;
			};

			class Pop : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Pop>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Pop>(std::move(args)...); }

			public:
				explicit Pop(void) noexcept = default;
				explicit Pop(const Register target) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::POP; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _target;
			};

			class Mov : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Mov>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Mov>(std::move(args)...); }

			public:
				explicit Mov(void) noexcept = default;
				explicit Mov(const Address& target, const Register source) noexcept;
				explicit Mov(const Address& target, const __int64 source) noexcept;
				explicit Mov(const Address& target, const __int32 source) noexcept;
				explicit Mov(const Address& target, const __int16 source) noexcept;
				explicit Mov(const Address& target, const __int8 source) noexcept;
				explicit Mov(const Address& target, const unsigned __int64 source) noexcept;
				explicit Mov(const Address& target, const unsigned __int32 source) noexcept;
				explicit Mov(const Address& target, const unsigned __int16 source) noexcept;
				explicit Mov(const Address& target, const unsigned __int8 source) noexcept;
				explicit Mov(const Register target, _In_ const mcf::IR::Expression::GlobalVariableIdentifier* globalExpression) noexcept;
				explicit Mov(const Register target, const Address& source) noexcept;
				explicit Mov(const Register target, const SizeOf& source) noexcept;
				explicit Mov(const Register target, const __int64 source) noexcept;
				explicit Mov(const Register target, const __int32 source) noexcept;
				explicit Mov(const Register target, const __int16 source) noexcept;
				explicit Mov(const Register target, const __int8 source) noexcept;
				explicit Mov(const Register target, const unsigned __int64 source) noexcept;
				explicit Mov(const Register target, const unsigned __int32 source) noexcept;
				explicit Mov(const Register target, const unsigned __int16 source) noexcept;
				explicit Mov(const Register target, const unsigned __int8 source) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::MOV; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _target;
				std::string _source;
			};

			class Lea : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Lea>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Lea>(std::move(args)...); }

			public:
				explicit Lea(void) noexcept = default;
				explicit Lea(const Register target, const mcf::IR::ASM::UnsafePointerAddress& source) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::LEA; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _target;
				std::string _source;
			};

			class Add : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Add>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Add>(std::move(args)...); }

			public:
				explicit Add(void) noexcept = default;
				explicit Add(const Register lhs, const __int64 rhs) noexcept;
				explicit Add(const Register lhs, const unsigned __int64 rhs) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::ADD; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _lhs;
				std::string _rhs;
			};

			class Sub : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Sub>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Sub>(std::move(args)...); }

			public:
				explicit Sub(void) noexcept = default;
				explicit Sub(const Register minuend, const __int64 subtrahend) noexcept;
				explicit Sub(const Register minuend, const unsigned __int64 subtrahend) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::SUB; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _minuend;
				std::string _subtrahend;
			};

			class Xor : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Xor>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Xor>(std::move(args)...); }

			public:
				explicit Xor(void) noexcept = default;
				explicit Xor(const Register& lhs, const Register rhs) noexcept;

				inline virtual const Type GetASMType(void) const noexcept override { return Type::XOR; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _lhs;
				std::string _rhs;
			};

			class Call : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Call>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Call>(std::move(args)...); }

			public:
				explicit Call(void) noexcept = default;
				explicit Call(const std::string& procName) noexcept;


				inline virtual const Type GetASMType(void) const noexcept override { return Type::CALL; }
				virtual const std::string Inspect(void) const noexcept override final;

			protected:
				std::string _procName;
			};
		}

		class IncludeLib final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<IncludeLib>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<IncludeLib>(std::move(args)...); }

		public:
			explicit IncludeLib(void) noexcept = default;
			explicit IncludeLib(const std::string& libPath) noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::INCLUDELIB; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			std::string _libPath;
		};

		class Typedef final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Typedef>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Typedef>(std::move(args)...); }

		public:
			explicit Typedef(void) noexcept = default;
			explicit Typedef(const mcf::Object::TypeInfo& definedType, const mcf::Object::TypeInfo& sourceType) noexcept;

			const mcf::Object::TypeInfo& GetDefinedType(void) const noexcept { return _definedType; }
			const mcf::Object::TypeInfo& GetSourceType(void) const noexcept { return _sourceType; }

			inline virtual const Type GetType(void) const noexcept override final { return Type::TYPEDEF; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			mcf::Object::TypeInfo _definedType;
			mcf::Object::TypeInfo _sourceType;
		};

		class Extern final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Extern>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Extern>(std::move(args)...); }

		public:
			explicit Extern(void) noexcept = default;
			explicit Extern(const std::string& name, const std::vector<mcf::Object::Variable>& params) noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::EXTERN; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			std::string _name;
			std::vector<mcf::Object::Variable> _params;
		};

		class Let final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Let>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Let>(std::move(args)...); }

		public:
			explicit Let(void) noexcept = default;
			explicit Let(const mcf::Object::VariableInfo&& info, mcf::IR::Expression::Pointer&& assignedExpression) noexcept;

			inline const mcf::Object::VariableInfo GetInfo(void) const noexcept { return _info; }
			inline const mcf::IR::Expression::Interface* GetUnsafeAssignExpressionPointer(void) const noexcept { return _assignExpression.get(); }

			inline virtual const Type GetType(void) const noexcept override final { return Type::LET; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			mcf::Object::VariableInfo _info;
			mcf::IR::Expression::Pointer _assignExpression;
		};

		class Func final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Func>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Func>(std::move(args)...); }

		public:
			explicit Func(void) noexcept = default;
			explicit Func(mcf::IR::ASM::PointerVector&& defines) noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::FUNC; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			mcf::IR::ASM::PointerVector _defines;
		};

		class Unused final : public Interface
		{
		public:
			inline static Pointer Make(void) noexcept { return std::make_unique<Unused>(); }
			inline virtual const Type GetType(void) const noexcept override final { return Type::UNUSEDIR; }
			inline virtual const std::string Inspect(void) const noexcept override final { return std::string(); }
		};

		class Return final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Return>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Return>(std::move(args)...); }

		public:
			explicit Return(void) noexcept = default;
			explicit Return(mcf::IR::Expression::Pointer&& returnExpression) noexcept;

			inline const mcf::IR::Expression::Interface* GetUnsafeReturnExpressionPointer(void) const noexcept { return _returnExpression.get(); }

			inline virtual const Type GetType(void) const noexcept override final { return Type::RETURN; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			mcf::IR::Expression::Pointer _returnExpression;
		};

		class Program final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Program>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) noexcept { return std::make_unique<Program>(std::move(args)...); }

		public:
			explicit Program(void) noexcept = default;
			explicit Program(PointerVector&& objects) noexcept;

			inline virtual const size_t GetObjectCount(void) const noexcept final { return _objects.size(); }
			inline virtual mcf::IR::Interface* GetUnsafeKeyObjectPointerAt(const size_t index) noexcept final
			{
				return _objects[index].get();
			}
			inline virtual const mcf::IR::Interface* GetUnsafeKeyObjectPointerAt(const size_t index) const noexcept final
			{
				return _objects[index].get();
			}

			inline virtual const Type GetType(void) const noexcept override final { return Type::PROGRAM; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			PointerVector _objects;
		};
	}
}