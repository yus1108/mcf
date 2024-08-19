#pragma once
#include <memory>
#include <string>
#include <vector>
#include <common.h>

namespace mcf
{
	namespace Object
	{
		enum Type : unsigned char
		{
			INVALID = 0,

			PROGRAM,
			INCLUDELIB,
			INTEGER,

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		constexpr const char* TYPE_STRING_ARRAY[] =
		{
			"INVALID",

			"PROGRAM",
			"INCLUDELIB",
			"INTEGER",
		};
		constexpr const size_t INTERMEDIATE_OBJECT_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
		static_assert(static_cast<size_t>(Type::COUNT) == INTERMEDIATE_OBJECT_TYPE_SIZE, "intermediate object type count not matching!");

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
	}
}