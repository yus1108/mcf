#include "pch.h"
#include "common.h"

#include "lexer.h"
#include "framework.h"

namespace mcf
{
	namespace internal 
	{
		inline static const mcf::token_type determine_keyword_or_identifier(const std::string& tokenLiteral) noexcept
		{
			// identifier 조건에 부합하는 키워드만 등록합니다.
			constexpr const char* IDENTIFIER_KEYWORDS[] =
			{
				// TODO: #5 uint32 타입 추가 필요
				// TODO: #6 더 많은 signed/unsigned integer 타입 추가 필요
				// TODO: #7 decimal 타입 추가 필요
				"int32",
				"enum",
			};
			constexpr const size_t KEYWORDS_SIZE = array_size(IDENTIFIER_KEYWORDS);
			static_assert(enum_index(mcf::token_type::keyword_identifier_end) - enum_index(mcf::token_type::keyword_identifier_start) - 1 == KEYWORDS_SIZE, 
				"identifier keyword token_type count is changed. this array need to be changed as well.");

			for (size_t i = 0; i < KEYWORDS_SIZE; i++)
			{
				if (tokenLiteral.compare(IDENTIFIER_KEYWORDS[i]) == 0)
				{
					return mcf::enum_at<mcf::token_type>(enum_index(mcf::token_type::keyword_int32) + i);
				}
			}
			return mcf::token_type::identifier;
		}

		inline static constexpr const bool is_alphabet(const char byte) noexcept
		{
			return ('a' <= byte && byte <= 'z') || ('A' <= byte && byte <= 'Z');
		}

		inline static constexpr const bool is_digit(const char byte) noexcept
		{
			return ('0' <= byte && byte <= '9');
		}

		inline static const std::string read_file(const std::string& path)
		{
			std::string input;
			{
				std::ifstream file(path.c_str());
				if (file.is_open() == false || file.fail() == true)
				{
					return std::string();
				}

				std::string line;
				while (std::getline(file, line))
				{
					if (file.fail() == true)
					{
						return std::string();
					}
					input += line;
				}
			}
			return input;
		}
	}
}

mcf::lexer::lexer(const std::string& input, const bool isFIle) noexcept
	: _input(isFIle ? internal::read_file(input) : input)
	, _name(input)
{
	if ( _input.length() == 0 )
	{
		if (isFIle)
		{
			std::ifstream file(_name.c_str());
			if (file.fail())
			{
				_tokens.push(lexer::error_token::fail_read_file);
				return;
			}
		}
		_tokens.push( lexer::error_token::invalid_input_length );
		return;
	}

	read_next_byte();
	// reset index to 0 after read next byte
	_currentIndex = 0;
}

const mcf::lexer::error_token mcf::lexer::get_last_error_token(void) noexcept
{
	if (_tokens.empty())
	{ 
		return lexer::error_token::no_error;
	}

	const mcf::lexer::error_token lError = _tokens.top();
	_tokens.pop();
	return lError;
}

const mcf::token mcf::lexer::read_next_token(void) noexcept
{
	if (_currentPosition == _input.size())
	{ 
		return { token_type::eof, "\0" };
	}
	mcf::token lToken;

	// 공백 문자는 스킵 하도록 합니다.
	while (_currentByte == ' ' || _currentByte == '\t' || _currentByte == '\n' || _currentByte == '\r')
	{
		read_next_byte();
		if ( _currentByte == '\n' )
		{
			_currentLine++;
			_currentIndex = 0;
		}
	}

	constexpr const size_t TOKEN_COUNT_BEGIN = __COUNTER__;
	switch (_currentByte)
	{
	case 0: __COUNTER__;
		lToken = { token_type::eof, "\0", _currentLine, _currentIndex };
		break;
	case '=': __COUNTER__;
		lToken = { token_type::assign, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '+': __COUNTER__;
		lToken = { token_type::plus, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '-': __COUNTER__;
		lToken = { token_type::minus, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '*': __COUNTER__;
		lToken = { token_type::asterisk, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '/': __COUNTER__;
		lToken = { token_type::slash, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '<': __COUNTER__;
		lToken = { token_type::lt, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '>': __COUNTER__;
		lToken = { token_type::gt, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '&': __COUNTER__;
		lToken = { token_type::ampersand, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '(': __COUNTER__;
		lToken = { token_type::lparen, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ')': __COUNTER__;
		lToken = { token_type::rparen, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '{': __COUNTER__;
		lToken = { token_type::lbrace, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '}': __COUNTER__;
		lToken = { token_type::rbrace, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '[': __COUNTER__;
		lToken = { token_type::lbracket, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ']': __COUNTER__;
		lToken = { token_type::rbracket, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ';': __COUNTER__;
		lToken = { token_type::semicolon, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ',': __COUNTER__;
		lToken = { token_type::comma, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ':': __COUNTER__;
		lToken = { token_type::colon, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '.':
		return read_dot_starting_token();
		__COUNTER__; // count for keyword_variadic
	default:
		// keyword + identifier 는 첫 시작이 '_' 이거나 알파벳 이어야만 합니다.
		if (internal::is_alphabet(_currentByte) || _currentByte == '_')
		{
			// TODO: #8 0x (16진수), 0 (8진수), 또는 0b (2진수) 숫자의 토큰을 생성 가능하게 개선 필요
			lToken.Literal = read_keyword_or_identifier(); 
			lToken.Type = internal::determine_keyword_or_identifier(lToken.Literal);
			lToken.Line = _currentLine;
			lToken.Index = _currentIndex;
			return lToken; 
			__COUNTER__; // count for identifier
			__COUNTER__; // count for keyword_int32
			__COUNTER__; // count for keyword_enum
		}
		else if (internal::is_digit(_currentByte))
		{
			lToken = { token_type::integer_32bit, read_number(), _currentLine, _currentIndex };
			// TODO: #7, #9 decimal 토큰을 생성 가능하게 개선 필요
			// TODO: #10 이후 postfix 로 타입 지정 가능하게 개선 필요
			return lToken;
			__COUNTER__; // count for integer_32bit
		}
		else
		{
			lToken = { token_type::invalid, std::string(1, _currentByte), _currentLine, _currentIndex };
			default_break(u8"예상치 못한 바이트 값이 들어 왔습니다. 토큰 생성에 실패 하였습니다. 현재 바이트[%u], ascii[%c]", _currentByte, _currentByte);
			__COUNTER__; // count for keyword_identifier_start
			__COUNTER__; // count for keyword_identifier_end
		}
	}
	constexpr const size_t TOKEN_COUNT = __COUNTER__ - TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");

	read_next_byte();
	return lToken;
}


inline const char mcf::lexer::get_next_char(void) const noexcept
{
	return _nextPosition < _input.length() ? _input[_nextPosition] : 0;
}

void mcf::lexer::read_next_byte(void) noexcept
{
	const size_t length = _input.length();
	debug_assert(_nextPosition <= length, u8"currentPosition 은 inputLength 보다 크거나 같을 수 없습니다!. inputLength=%llu, nextPosition=%llu", length, _nextPosition);

	_currentByte = (_nextPosition > length) ? 0 : _input[_nextPosition];
	_currentPosition = _nextPosition;
	_nextPosition += 1;
	_currentIndex += 1;
}

const std::string mcf::lexer::read_keyword_or_identifier(void) noexcept
{
	debug_assert(internal::is_alphabet(_currentByte) || _currentByte == '_', u8"키워드 혹은 식별자의 시작은 알파벳이거나 '_' 이어야만 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;
	read_next_byte();

	// 첫 문자 이후로는 알파벳, 숫자, 또는 '_' 이어야 합니다.
	while (internal::is_alphabet(_currentByte) || internal::is_digit(_currentByte) || (_currentByte == '_'))
	{
		read_next_byte();
	}
	return _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
}

const std::string mcf::lexer::read_number(void) noexcept
{
	debug_assert(internal::is_digit(_currentByte), u8"숫자의 시작은 0부터 9까지의 문자여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;

	while (internal::is_digit(_currentByte))
	{
		read_next_byte();
	}
	return _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
}

const mcf::token mcf::lexer::read_dot_starting_token(void) noexcept
{
	debug_assert(_currentByte == '.', u8"이 함수가 호출될때 '.'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;

	// 연속되는 문자열이 "..."(keyword_variadic) 인지 검사합니다.
	read_next_byte();
	if (_currentByte == '.' && get_next_char() == '.')
	{
		read_next_byte();
		return { token_type::keyword_variadic, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	return { token_type::invalid, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}