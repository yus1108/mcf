#pragma once
#include <memory>
#include <string>
#include <vector>
#include <common.h>

namespace mcf
{
	namespace Object
	{
		namespace Intermediate
		{
			enum Type
			{
				INVALID = 0,

				INTEGER,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT
			};

			constexpr const char* TYPE_STRING_ARRAY[] =
			{
				"INVALID",

				"INTEGER",
			};
			constexpr const size_t INTERMEDIATE_OBJECT_TYPE_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
			static_assert(static_cast<size_t>(Type::COUNT) == INTERMEDIATE_OBJECT_TYPE_SIZE, "intermediate object type count not matching!");

			constexpr const char* CONVERT_TYPE_TO_STRING(const Type& value)
			{
				return TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
			}

			class Interface
			{
			public:
				virtual ~Interface(void) noexcept = default;
				virtual const mcf::Object::Intermediate::Type GetType(void) const noexcept = 0;
				virtual const std::string ConvertToString(void) const noexcept = 0;
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Integer : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Integer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Integer>(std::move(args)...); }

			public:
				explicit Integer(void) noexcept = default;
				explicit Integer(const __int64& value) noexcept : _value(value) {}

				inline virtual const Type GetType(void) const noexcept override final { return Type::INTEGER; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return std::to_string(_value); }

			private:
				__int64 _value;
			};

		}
	}
}