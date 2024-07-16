#pragma once
#include <stack>

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
		assign,			// =
		plus,			// +
		minus,			// -
		asterisk,		// *
		slash,			// /
		lt,				// <
		gt,				// >

		lparen,			// (
		rparen,			// )
		lbrace,			// {
		rbrace,			// }
		lbracket,		// [
		rbracket,		// ]

		// 구분자
		semicolon,		// ;
		comma,			// ,

		// 예약어
		keyword_int32,	// int32

		// 이 밑으로는 수정하면 안됩니다.
		count
	};

	struct token final
	{
		mcf::token_type	Type = mcf::token_type::invalid;
		std::string		Literal;
		size_t			Line	= 0;
		size_t			Index	= 0;
	};
	inline bool operator==(const mcf::token& lhs, const mcf::token& rhs) { return (lhs.Type == rhs.Type) && (lhs.Literal == rhs.Literal); }

	// 주의: thread-safe 하지 않은 클래스입니다.
	class lexer final {
	public:
		enum class error_token : unsigned char 
		{
			invalid = 0,

			no_error,
			invalid_input_length,
			fail_read_file,

			count,
		};

	public:
		explicit lexer(void) noexcept = delete;
		explicit lexer(const std::string& input, const bool isFIle) noexcept;

		const mcf::lexer::error_token	get_last_error_token(void) noexcept;
		const std::string				get_name(void) const noexcept { return _name; }

		const mcf::token read_next_token(void) noexcept;

	private:
		void				read_next_byte(void) noexcept;
		const std::string	read_keyword_or_identifier(void) noexcept;
		const std::string	read_number(void) noexcept;

	private:
		std::stack<lexer::error_token>	_tokens;
		const std::string				_input;
		const std::string				_name;
		size_t							_currentPosition	= 0;
		size_t							_nextPosition		= 0;
		size_t							_currentLine		= 1; // 코드 명령줄은 항상 1부터 시작합니다.
		size_t							_currentIndex		= 0;
		char							_currentByte		= 0;
	};
}