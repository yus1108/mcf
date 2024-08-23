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
		// if any item in ArraySizeList has the value as 0, it means it's unknown
		struct TypeInfo final
		{
			std::vector<size_t> ArraySizeList;
			std::string Name;
			bool IsUnsigned = false;

			static const mcf::Object::TypeInfo MakePrimitive(std::string name) { return { std::vector<size_t>(), name, false }; }

			inline const bool IsValid(void) const noexcept { return Name.empty() == false; }
			const std::string Inspect(void) const noexcept;
		};

		struct Variable final
		{
			std::string Name;
			TypeInfo DataType;

			inline const bool IsValid(void) const noexcept { return Name.empty() == false; }
		};

		struct VariableInfo final
		{
			Variable Variable;
			bool IsGlobal;

			inline const bool IsValid(void) const noexcept { return Variable.IsValid() == false; }
		};

		struct FunctionParams final
		{
			std::vector<Variable> Variables;
			std::string VariadicIdentifier;

			inline const bool IsVoid(void) const noexcept { return Variables.empty() && VariadicIdentifier.empty(); }
		};

		struct FunctionInfo final
		{
			std::string Name;
			FunctionParams Params;
			TypeInfo ReturnType;

			inline const bool IsValid( void ) const noexcept { return Name.empty() == false; }
		};

		class Scope final
		{
		public:
			explicit Scope(void) noexcept {}
			explicit Scope(const Scope* parent) noexcept : _parent(parent) {}

			const bool IsIdentifierRegistered(const std::string& name) const noexcept;

			const bool DefineType(const std::string& name, const mcf::Object::TypeInfo& info) noexcept;
			const mcf::Object::TypeInfo FindTypeInfo(const std::string& name) const noexcept;

			const bool DefineVariable(const std::string& name, const mcf::Object::Variable& info) noexcept;
			const mcf::Object::VariableInfo FindVariableInfo(const std::string& name) const noexcept;

			const bool DefineFunction(const std::string& name, const mcf::Object::FunctionInfo& info) noexcept;
			const mcf::Object::FunctionInfo FindFunction(const std::string& name) const noexcept;

		private:
			std::unordered_set<std::string> _allIdentifierSet;
			std::unordered_map<std::string, mcf::Object::TypeInfo> _typeInfoMap;
			std::unordered_map<std::string, mcf::Object::Variable> _variables;
			std::unordered_map<std::string, mcf::Object::FunctionInfo> _functionInfoMap;
			const Scope* _parent = nullptr;
		};
	}

	namespace IR
	{
		enum class Type : unsigned char
		{
			INVALID = 0,

			EXPRESSION,

			INCLUDELIB,
			EXTERN,

			PROGRAM,

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		constexpr const char* TYPE_STRING_ARRAY[] =
		{
			"INVALID",

			"EXPRESSION",

			"INCLUDELIB",
			"EXTERN",

			"PROGRAM",
		};
		constexpr const size_t OBJECT_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
		static_assert(static_cast<size_t>(Type::COUNT) == OBJECT_TYPE_SIZE, "object type count not matching!");

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
			inline static Pointer Make() { return std::make_unique<Invalid>(); }
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
			};
			constexpr const size_t OBJECT_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
			static_assert(static_cast<size_t>(Type::COUNT) == OBJECT_TYPE_SIZE, "object type count not matching!");

			constexpr const char* CONVERT_TYPE_TO_STRING(const Type value)
			{
				return TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
			}

			class Interface : public mcf::IR::Interface
			{
			public:
				virtual const Type GetExpressionType(void) const noexcept = 0;

				inline virtual const mcf::IR::Type GetType(void) const noexcept override final { return mcf::IR::Type::EXPRESSION; }
				virtual const std::string Inspect(void) const noexcept override = 0;
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Invalid : public Interface
			{
			public:
				inline static Pointer Make() { return std::make_unique<Invalid>(); }
				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::INVALID; }
				inline virtual const std::string Inspect(void) const noexcept override final { return "Invalid Object"; }
			};

			class TypeIdentifier final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<TypeIdentifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<TypeIdentifier>(std::move(args)...); }

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
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<GlobalVariableIdentifier>(std::move(args)...); }

			public:
				explicit GlobalVariableIdentifier(void) noexcept = default;
				explicit GlobalVariableIdentifier(const mcf::Object::Variable& variable) noexcept;

				const mcf::Object::Variable& GetInfo(void) const noexcept { return _variable;}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::GLOBAL_VARIABLE_IDENTIFIER; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				mcf::Object::Variable _variable;
			};

			class LocalVariableIdentifier final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<LocalVariableIdentifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<LocalVariableIdentifier>(std::move(args)...); }

			public:
				explicit LocalVariableIdentifier(void) noexcept = default;
				explicit LocalVariableIdentifier(const mcf::Object::Variable& info) noexcept;

				const mcf::Object::Variable& GetInfo(void) const noexcept { return _info;}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::LOCAL_VARIABLE_IDENTIFIER; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				mcf::Object::Variable _info;
			};

			class FunctionIdentifier final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<FunctionIdentifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<FunctionIdentifier>(std::move(args)...); }

			public:
				explicit FunctionIdentifier(void) noexcept = default;
				explicit FunctionIdentifier(const mcf::Object::FunctionInfo& info) noexcept;

				const mcf::Object::FunctionInfo& GetInfo(void) const noexcept { return _info;}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::FUNCTION_IDENTIFIER; }
				virtual const std::string Inspect(void) const noexcept override final;

			private:
				mcf::Object::FunctionInfo _info;
			};
		}

		class IncludeLib final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<IncludeLib>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) { return std::make_unique<IncludeLib>(std::move(args)...); }

		public:
			explicit IncludeLib(void) noexcept = default;
			explicit IncludeLib(const std::string& libPath) noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::INCLUDELIB; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			std::string _libPath;
		};

		class Extern final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Extern>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Extern>(std::move(args)...); }

		public:
			explicit Extern(void) noexcept = default;
			explicit Extern(const std::string& name, const std::vector<mcf::Object::TypeInfo>& params, const bool hasVariadic) noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::EXTERN; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			std::string _name;
			std::vector<mcf::Object::TypeInfo> _params;
			bool _hasVariadic;
		};

		class Program final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Program>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Program>(std::move(args)...); }

		public:
			explicit Program(void) noexcept = default;
			explicit Program(PointerVector&& objects) noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::PROGRAM; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			PointerVector _objects;
		};
	}
}