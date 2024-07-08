#pragma once

#include <string>

namespace mcf
{
	enum class token_type : unsigned char
	{
		illegal = 0,
		eof,

		// 식별자 + 리터럴
		identifier,
		integer_32bit,

		// 연산자
		assign,
		plus,

		// 구분자
		semicolon,

		// 예약어
		keyword,

		// 이 밑으로는 수정하면 안됩니다.
		count
	};

	struct token final
	{
		mcf::token_type	Type	= mcf::token_type::illegal;
		std::string		Literal;
	};

	// 주의: thread-safe 하지 않은 클래스입니다.
	class lexer final {
	public:
		enum class error_token : unsigned char {
			no_error = 0,
			invalid_input_length,
		};

	public:
		explicit lexer(const std::string input) noexcept;
				~lexer(void) noexcept;

		static const mcf::lexer::error_token get_last_error_token(void) noexcept;

		const mcf::token read_next_token() noexcept;

	private:
		void		read_next_byte(void) noexcept;
		std::string read_keyword_or_identifier(void) noexcept;
		std::string read_number(void) noexcept;

	private:
		const std::string	_input				= nullptr;
		size_t				_currentPosition	= 0;
		size_t				_nextPosition		= 0;
		char				_currentByte		= 0;
	};
}