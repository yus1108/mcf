#pragma once
#include <memory>
#include <string>
#include <vector>
#include <common.h>

namespace mcf
{
	namespace Object
	{
		enum class Type : unsigned char
		{
			INVALID = 0,

			INDEXDATA,
			TYPEINFO,
			INTEGER,

			INCLUDELIB,
			EXTERN,

			PROGRAM,

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		constexpr const char* TYPE_STRING_ARRAY[] =
		{
			"INVALID",

			"INDEXDATA",
			"TYPEINFO",
			"INTEGER",

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
			inline virtual const Type GetType( void ) const noexcept override final { return Type::INVALID; }
			inline virtual const std::string Inspect( void ) const noexcept override final { return "Invalid Object"; }
		};

		namespace IndexData
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				UNKNOWN,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT
			};

			constexpr const char* TYPE_STRING_ARRAY[] =
			{
				"INVALID",

				"UNKNOWN",
			};
			constexpr const size_t INDEXDATA_TYPE_SIZE = MCF_ARRAY_SIZE( TYPE_STRING_ARRAY );
			static_assert(static_cast<size_t>(Type::COUNT) == INDEXDATA_TYPE_SIZE, "index data type count not matching!");

			constexpr const char* CONVERT_TYPE_TO_STRING( const Type value )
			{
				return TYPE_STRING_ARRAY[mcf::ENUM_INDEX( value )];
			}

			class Interface : public mcf::Object::Interface
			{
			public:

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Interface>(std::move(args)...); }

			public:
				explicit Interface(void) noexcept = default;

				virtual const Type GetIndexDataType(void) const noexcept = 0;

				inline virtual const mcf::Object::Type GetType(void) const noexcept override final { return mcf::Object::Type::INDEXDATA; }
				inline virtual const std::string Inspect(void) const noexcept override = 0;
			};
			using Pointer = std::unique_ptr<Interface>;

			class Unknown final : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Unknown>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Unknown>(std::move(args)...); }

			public:
				explicit Unknown(void) noexcept = default;

				inline virtual const Type GetIndexDataType(void) const noexcept override final { return Type::UNKNOWN; }
				virtual const std::string Inspect(void) const noexcept override final;
			};
		}

		class TypeInfo final : public Interface
		{
		public:
			using Pointer = std::unique_ptr<TypeInfo>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) { return std::make_unique<TypeInfo>(std::move(args)...); }

		public:
			explicit TypeInfo(void) noexcept = default;
			explicit TypeInfo(const bool isUnsigned, const std::string& typeName, std::vector<mcf::Object::IndexData::Pointer>&& indexDataList) noexcept;

			inline const std::string GetTypeName(void) const noexcept { return _typeName; }
			inline const bool IsArrayType(void) const noexcept { return _indexDataList.empty() == false; }

			inline virtual const Type GetType(void) const noexcept override final { return Type::TYPEINFO; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			bool _isUnsigned;
			std::string _typeName;
			std::vector<mcf::Object::IndexData::Pointer> _indexDataList;
		};

		class Integer : public Interface
		{
		public:
			using Pointer = std::unique_ptr<Integer>;

			template <class... Variadic>
			inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Integer>(std::move(args)...); }

		public:
			explicit Integer(void) noexcept = default;
			explicit Integer(const bool isSigned, const unsigned __int64& value) noexcept : _isSigned(isSigned), _value(value) {}

			inline const bool IsByte(void) const noexcept { return INT8_MIN <= static_cast<__int64>(_value) && static_cast<__int64>(_value) <= INT8_MAX; }
			inline const bool IsUnsignedByte(void) const noexcept { return _value <= UINT8_MAX; }
			inline const bool IsWord(void) const noexcept { return INT16_MIN <= static_cast<__int64>(_value) && static_cast<__int64>(_value) <= INT16_MAX; }
			inline const bool IsUnsignedWord(void) const noexcept { return _value <= UINT16_MAX; }
			inline const bool IsDoubleWord(void) const noexcept { return INT32_MIN <= static_cast<__int64>(_value) && static_cast<__int64>(_value) <= INT32_MAX; }
			inline const bool IsUnsignedDoubleWord(void) const noexcept { return _value <= UINT32_MAX; }
			inline const bool IsQuadWord(void) const noexcept { return INT64_MIN <= static_cast<__int64>(_value) && static_cast<__int64>(_value) <= INT64_MAX; }
			inline const bool IsUnsignedQuadWord(void) const noexcept { return _value <= UINT64_MAX; }

			const int8_t GetByte(void) const noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::INTEGER; }
			inline virtual const std::string Inspect(void) const noexcept override final { return std::to_string(_value); }

		private:
			bool _isSigned;
			unsigned __int64 _value;
		};

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
			explicit Extern(const std::string& functionName, const std::vector<std::string>& paramTypes) noexcept;

			inline virtual const Type GetType(void) const noexcept override final { return Type::EXTERN; }
			virtual const std::string Inspect(void) const noexcept override final;

		private:
			std::string _functionName;
			std::vector<std::string> _paramTypes;
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