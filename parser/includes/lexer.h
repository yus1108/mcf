#pragma once
#include <stack>
#include <vector>
#include <unordered_map>
#include <string>

namespace mcf
{
	class evaluator;

	enum class token_type : unsigned char
	{
		invalid = 0,
		eof,		// \0

		// 식별자 + 리터럴
		identifier,				// [_a-zA-Z]+[_a-zA-Z0-9]*
		integer,				// [0-9]+
		string_utf8,			// "[^"\n\r]*"

		// 연산자
		assign,		// =
		plus,		// +
		minus,		// -
		asterisk,	// *
		slash,		// /
		bang,		// !
		equal,		// ==
		not_equal,	// !=
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
		colon,			// :
		double_colon,	// ::
		semicolon,		// ;
		comma,			// ,

		// 식별자 키워드
		keyword_identifier_start,	// 실제 값으로 사용되어선 안됩니다!!!
		keyword_const,				// const
		keyword_void,				// void
		keyword_int8,				// int8
		keyword_int16,				// int16
		keyword_int32,				// int32
		keyword_int64,				// int64
		keyword_uint8,				// uint8
		keyword_uint16,				// uint16
		keyword_uint32,				// uint32
		keyword_uint64,				// uint64
		keyword_utf8,				// utf8
		keyword_enum,				// enum
		keyword_unused,				// unused
		keyword_in,					// in
		keyword_out,				// out
		keyword_bool,				// bool
		keyword_true,				// true
		keyword_false,				// false
		keyword_identifier_end,		// 실제 값으로 사용되어선 안됩니다!!!

		custom_keyword_start,	// 실제 값으로 사용되어선 안됩니다!!!
		custom_enum_type,		// 커스텀 열거형 타입
		custom_keyword_end,		// 실제 값으로 사용되어선 안됩니다!!!

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

	const mcf::token_type find_keyword_token_type(const std::string& tokenLiteral) noexcept;

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
			registering_duplicated_symbol_name,

			count,
		};

	public:
		explicit lexer(void) noexcept = delete;
		explicit lexer(const evaluator* const evaluator, const std::string& input, const bool isFIle) noexcept;

		const mcf::lexer::error_token	get_last_error_token(void) noexcept;
		const std::string				get_name(void) const noexcept { return _name; }

		const mcf::token		read_next_token(void) noexcept;

	private:
		const char get_next_byte(void) const noexcept;

		void				read_next_byte(void) noexcept;
		const bool			read_line_if_start_with(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* startWith) noexcept;
		const bool			read_and_validate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* stringToCompare) noexcept;
		const bool			read_and_validate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* startWith, _In_opt_ const char* endWith, _In_opt_ const char* invalidCharList) noexcept;
		const std::string	read_keyword_or_identifier(void) noexcept;
		const std::string	read_number(void) noexcept;
		const mcf::token	read_string_utf8(void) noexcept;
		const mcf::token	read_slash_starting_token(void) noexcept;
		const mcf::token	read_colon_starting_token(void) noexcept;
		const mcf::token	read_dot_starting_token(void) noexcept;
		const mcf::token	read_macro_token(void) noexcept; 
		const mcf::token	read_numeric_literal(void) noexcept; 

		const mcf::token_type determine_keyword_or_identifier(const std::string& tokenLiteral) noexcept;

	private:
		const evaluator* _evaluator;

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