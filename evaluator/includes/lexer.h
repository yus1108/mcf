#pragma once
#include <stack>
#include <string>
#include <common.h>

namespace mcf
{
	namespace Token
	{
		enum class Type : unsigned char
		{
			INVALID = 0,
			END_OF_FILE, // \0

			// 식별자 + 리터럴
			IDENTIFIER,	// [_a-zA-Z]+[_a-zA-Z0-9]*
			INTEGER,	// [0-9]+
			STRING,		// "[^"\n\r]*"

			// 연산자
			ASSIGN,		// =
			PLUS,		// +
			MINUS,		// -
			ASTERISK,	// *
			SLASH,		// /
			BANG,		// !
			EQUAL,		// ==
			NOT_EQUAL,	// !=
			LT,			// <
			GT,			// >
			AMPERSAND,	// &

			LPAREN,		// (
			RPAREN,		// )
			LBRACE,		// {
			RBRACE,		// }
			LBRACKET,	// [
			RBRACKET,	// ]

			// 구분자
			COLON,			// :
			DOUBLE_COLON,	// ::
			SEMICOLON,		// ;
			COMMA,			// ,
			POINTING,		// ->

			// 식별자 키워드
			KEYWORD_IDENTIFIER_START,	// 실제 값으로 사용되어선 안됩니다!!!
			KEYWORD_ASM,				// asm
			KEYWORD_EXTERN,				// extern
			KEYWORD_TYPEDEF,			// typedef
			KEYWORD_LET,				// let
			KEYWORD_FUNC,				// func
			KEYWORD_MAIN,				// main
			KEYWORD_VOID,				// void
			KEYWORD_UNSIGNED,			// unsigned
			KEYWORD_RETURN,				// return
			KEYWORD_UNUSED,				// unused
			KEYWORD_AS,					// as
			KEYWORD_IDENTIFIER_END,		// 실제 값으로 사용되어선 안됩니다!!!

			// '.' 으로 시작하는 토큰
			VARIADIC,	// ...

			// 매크로: '#' 으로 시작하는 토큰
			MACRO_START,	// 실제 값으로 사용되어선 안됩니다!!!
			MACRO_INCLUDE,	// #include
			MACRO_END,		// 실제 값으로 사용되어선 안됩니다!!!

			// 주석
			COMMENT,		// //[^\n\r]
			COMMENT_BLOCK,	// /*[^"*/"]*/

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		constexpr const char* TYPE_STRING_ARRAY[] =
		{
			"INVALID",
			"END_OF_FILE",

			// 식별자 + 리터럴
			"IDENTIFIER",
			"INTEGER",
			"STRING",

			// 연산자
			"ASSIGN",
			"PLUS",
			"MINUS",
			"ASTERISK",
			"SLASH",
			"BANG",
			"EQUAL",
			"NOT_EQUAL",
			"LT",
			"GT",
			"AMPERSAND",

			"LPAREN",
			"RPAREN",
			"LBRACE",
			"RBRACE",
			"LBRACKET",
			"RBRACKET",


			// 구분자
			"COLON",
			"DOUBLE_COLON",
			"SEMICOLON",
			"COMMA",
			"POINTING",

			// 식별자 키워드
			"KEYWORD_IDENTIFIER_START",
			"KEYWORD_ASM",
			"KEYWORD_EXTERN",
			"KEYWORD_TYPEDEF",
			"KEYWORD_LET",
			"KEYWORD_FUNC",
			"KEYWORD_MAIN",
			"KEYWORD_VOID",
			"KEYWORD_UNSIGNED",
			"KEYWORD_RETURN",
			"KEYWORD_UNUSED",
			"KEYWORD_AS",
			"KEYWORD_IDENTIFIER_END",

			// '.' 으로 시작하는 토큰
			"VARIADIC",

			// 매크로
			"MACRO_START",
			"MACRO_INCLUDE",
			"MACRO_END",

			"COMMENT",
			"COMMENT_BLOCK",
		};
		constexpr const size_t TOKEN_TYPES_SIZE = MCF_ARRAY_SIZE(TYPE_STRING_ARRAY);
		static_assert(static_cast<size_t>(Type::COUNT) == TOKEN_TYPES_SIZE, "token count not matching!");

		constexpr const char* CONVERT_TYPE_TO_STRING(const Type value)
		{
			return TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
		}

		struct Data final
		{
			Type Type = Type::INVALID;
			std::string Literal;
			size_t Line = 0;
			size_t Index = 0;
		};
		inline bool operator==(const Data& lhs, const Data& rhs) { return (lhs.Type == rhs.Type) && (lhs.Literal == rhs.Literal); }
		static const Data FindPredefinedKeyword(const std::string& tokenLiteral) noexcept;
	}

	namespace Lexer
	{
		enum class Error : unsigned char
		{
			INVALID = 0,

			SUCCESS,
			INVALID_INPUT_LENGTH,
			FAIL_READ_FILE,

			COUNT,
		};

		// 주의: thread-safe 하지 않은 클래스입니다.
		class Object final 
		{
		public:
			explicit Object(void) noexcept = delete;
			explicit Object(const std::string& input, const bool isFIle) noexcept;

			const Error GetLastErrorToken(void) noexcept;
			const std::string GetName(void) const noexcept { return _name; }

			const mcf::Token::Data ReadNextToken(void) noexcept;

		private:
			inline const char GetNextByte(void) const noexcept;

			inline void ReadNextByte(void) noexcept;
			inline const bool ReadLineIfStartWith(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* startWith) noexcept;
			inline const bool ReadAndValidate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* stringToCompare) noexcept;
			inline const bool ReadAndValidate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* startWith, _In_opt_ const char* endWith, _In_opt_ const char* invalidCharList) noexcept;
			inline const std::string ReadKeywordOrIdentifier(void) noexcept;
			inline const std::string ReadNumber(void) noexcept;
			inline const mcf::Token::Data ReadStringUtf8(void) noexcept;
			inline const mcf::Token::Data ReadSlashStartingToken(void) noexcept;
			inline const mcf::Token::Data ReadDotStartingToken(void) noexcept;
			inline const mcf::Token::Data ReadMacroToken(void) noexcept;
			inline const mcf::Token::Data ReadNumeric(void) noexcept;

			inline const mcf::Token::Type DetermineKeywordOrIdentifier(const std::string& tokenLiteral) noexcept;

		private:
			std::stack<Error> _tokens;
			const std::string _input;
			const std::string _name;
			size_t _currentPosition = 0;
			size_t _nextPosition = 0;
			size_t _currentLine = 1; // 코드 명령줄은 항상 1부터 시작합니다.
			size_t _currentIndex = 0;
			char _currentByte = 0;
		};
	}
}