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
			virtual const mcf::ASM::Type GetType(void) const noexcept = 0;
			virtual const std::string ConvertToString(void) const noexcept = 0;
		};

		using Pointer = std::unique_ptr<Interface>;
		using PointerVector = std::vector<Pointer>;

		class Invalid : public Interface
		{
		public:
			inline static Pointer Make(void) noexcept { return std::make_unique<Invalid>(); }
			inline virtual const mcf::ASM::Type GetType(void) const noexcept override final { return Type::INVALID; }
			inline virtual const std::string ConvertToString(void) const noexcept override final { return "Invalid Code"; }
		};

		namespace MASM64
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				SECTION_DATA,
				TYPEDEF,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT
			};

			constexpr const char* TYPE_STRING_ARRAY[] =
			{
				"INVALID",

				"SECTION_DATA",
				"TYPEDEF",
			};
			constexpr const size_t MASM64_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
			static_assert(static_cast<size_t>(Type::COUNT) == MASM64_TYPE_SIZE, "MASM64 type count not matching!");

			constexpr const char* CONVERT_TYPE_TO_STRING(const Type value)
			{
				return TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
			}

			class Interface : public mcf::ASM::Interface
			{
			public:
				virtual const mcf::ASM::MASM64::Type GetMASM64Type(void) const noexcept = 0;
				inline virtual const mcf::ASM::Type GetType(void) const noexcept override final { return mcf::ASM::Type::MASM64; }
				inline virtual const std::string ConvertToString(void) const noexcept override = 0;
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Invalid : public Interface
			{
			public:
				inline static Pointer Make(void) noexcept { return std::make_unique<Invalid>(); }
				inline virtual const mcf::ASM::MASM64::Type GetMASM64Type(void) const noexcept override final { return Type::INVALID; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "Invalid Code"; }
			};

			class SectionData : public Interface
			{
			public:
				inline static Pointer Make(void) noexcept { return std::make_unique<SectionData>(); }
				inline virtual const mcf::ASM::MASM64::Type GetMASM64Type(void) const noexcept override final { return Type::SECTION_DATA; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return ".data"; }
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

				inline virtual const mcf::ASM::MASM64::Type GetMASM64Type(void) const noexcept override final { return Type::TYPEDEF; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::Object::TypeInfo _definedType;
				mcf::Object::TypeInfo _sourceType;
			};

			namespace Compiler
			{
				enum class Section : unsigned char
				{
					INVALID = 0,

					DATA,
					CODE,

					// 이 밑으로는 수정하면 안됩니다.
					COUNT
				};

				class Object final
				{
				public:
					explicit Object(void) noexcept = default;

					mcf::ASM::PointerVector GenerateCodes(_In_ const mcf::IR::Program* program, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept;

				private:
					const bool CompileTypedef(_Out_ mcf::ASM::PointerVector& outCodes, _In_ const mcf::IR::Typedef* irCode, _In_ const mcf::Object::ScopeTree* scopeTree) noexcept;

				private:
					Section _currentSection = Section::INVALID;
				};
			}
		}
	}
}