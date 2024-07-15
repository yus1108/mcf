#pragma once

// ENUM
namespace mcf
{
	template<typename T>
	constexpr const size_t enum_count(void)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum type is required for this function");
		return static_cast<size_t>(T::count);
	}

	template<typename T>
	constexpr const size_t enum_count(const T value)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return static_cast<size_t>(T::count);
	}

	template<typename T>
	constexpr const size_t enum_index(const T value)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return value < T::count ? static_cast<size_t>(value) : static_cast<size_t>(T::invalid);
	}

	template<typename T>
	const T enum_at(const size_t index)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return index < mcf::enum_count<T>() ? static_cast<T>(index) : T::invalid;
	}
}


namespace mcf
{
	enum class token_type : unsigned char
	{
		invalid = 0,
		eof,			// \0

		// 식별자 + 리터럴
		identifier,		// [_a-zA-Z]+[_a-zA-Z0-9]*
		integer_32bit,	// [0-9]+

		// 연산자
		assign,
		plus,			// +
		minus,			// -
		asterisk,		// *
		slash,			// /

		// 구분자
		semicolon,		// ;

		// 예약어
		keyword_int32,	// int32

		// 이 밑으로는 수정하면 안됩니다.
		count
	};

	struct token final
	{
		mcf::token_type	Type = mcf::token_type::invalid;
		std::string		Literal;
	};
	inline bool operator==(const mcf::token& lhs, const mcf::token& rhs) { return (lhs.Type == rhs.Type) && (lhs.Literal == rhs.Literal); }

	// 주의: thread-safe 하지 않은 클래스입니다.
	class lexer final {
	public:
		enum class error_token : unsigned char 
		{
			no_error = 0,
			invalid_input_length,
		};

	public:
		explicit lexer(void) noexcept = delete;
		explicit lexer(const std::string& input) noexcept;

		static const mcf::lexer::error_token get_last_error_token(void) noexcept;

		const mcf::token read_next_token(void) noexcept;

	private:
		void				read_next_byte(void) noexcept;
		const std::string	read_keyword_or_identifier(void) noexcept;
		const std::string	read_number(void) noexcept;

	private:
		const std::string	_input				= nullptr;
		size_t				_currentPosition	= 0;
		size_t				_nextPosition		= 0;
		char				_currentByte		= 0;
	};
}