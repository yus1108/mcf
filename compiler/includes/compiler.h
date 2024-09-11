#pragma once
#include <object.h>

namespace mcf
{
	namespace ASM
	{
		enum class Type : unsigned char
		{
			INVALID = 0,

			MASM64,

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		constexpr const char* TYPE_STRING_ARRAY[] =
		{
			"INVALID",

			"MASM64",
		};
		constexpr const size_t ASM_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
		static_assert(static_cast<size_t>(Type::COUNT) == ASM_TYPE_SIZE, "ASM type count not matching!");

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

		namespace MASM64
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				MASM64,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT
			};

			constexpr const char* TYPE_STRING_ARRAY[] =
			{
				"INVALID",

				"MASM64",
			};
			constexpr const size_t ASM_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
			static_assert(static_cast<size_t>(Type::COUNT) == ASM_TYPE_SIZE, "MASM64 type count not matching!");

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
		}
	}

	namespace Compiler
	{
		class Object final
		{
		public:
			explicit Object(void) noexcept = default;
		};
	}
}