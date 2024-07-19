#pragma once
#include <stack>

#define test             invalid;          test             \
invalid;

namespace mcf
{
	enum class token_type : unsigned char
	{
		invalid = 0,
		eof,		// \0

		// 식별자 + 리터럴
		identifier,		// [_a-zA-Z]+[_a-zA-Z0-9]*
		integer_32bit,	// [0-9]+
		string_utf8,	// "[^"\n\r]*"

		// 연산자
		assign,		// =
		plus,		// +
		minus,		// -
		asterisk,	// *
		slash,		// /
		lt,			// <
		gt,			// >
		ampersand,	// &

		lparen,		// (
		rparen,		// )
		lbrace,		// {
		rbrace,		// }
		lbracket,	// [
		rbracket,	// ]

		// 구분자
		semicolon,	// ;
		comma,		// ,
		colon,		// :

		// 식별자 키워드
		keyword_identifier_start,	// 실제 값으로 사용되어선 안됩니다!!!
		keyword_const,				// const
		keyword_void,				// void
		keyword_int32,				// int32
		keyword_uint32,				// int32
		keyword_utf8,				// utf8
		keyword_enum,				// enum
		keyword_unused,				// unused
		keyword_identifier_end,		// 실제 값으로 사용되어선 안됩니다!!!

		// '.' 으로 시작하는 토큰
		keyword_variadic,	// ...

		// 매크로
		macro_start,				// 실제 값으로 사용되어선 안됩니다!!!
		macro_iibrary_file_include,	// #include <[^<>\n\r]+>
		macro_project_file_include,	// #include "[^<>\n\r]+"
		macro_end,					// 실제 값으로 사용되어선 안됩니다!!!

		// 주석
		comment,		// //[^\n\r]
		comment_block,	// /*[^"*/"]*/

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
			fail_memory_allocation,

			count,
		};

	public:
		explicit lexer(void) noexcept = delete;
		explicit lexer(const std::string& input, const bool isFIle) noexcept;

		const mcf::lexer::error_token	get_last_error_token(void) noexcept;
		const std::string				get_name(void) const noexcept { return _name; }

		const mcf::token read_next_token(void) noexcept;

	private:
		const char get_next_byte(void) const noexcept;

		void				read_next_byte(void) noexcept;
		const bool			read_line_if_start_with(std::string* optionalOut, const char* startWith) noexcept;
		const bool			read_and_validate(std::string* optionalOut, const char* stringToCompare) noexcept;
		const bool			read_and_validate(std::string* optionalOut, const char* startWith, const char* endWith, const char* invalidCharList) noexcept;
		const std::string	read_keyword_or_identifier(void) noexcept;
		const std::string	read_number(void) noexcept;
		const mcf::token	read_string_utf8(void) noexcept;
		const mcf::token	read_slash_starting_token(void) noexcept;
		const mcf::token	read_dot_starting_token(void) noexcept;
		const mcf::token	read_macro_token(void) noexcept;

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