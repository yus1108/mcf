#include "pch.h"
#include "lexer.h"

namespace mcf
{
	namespace internal {
		struct
		{
			std::stack<lexer::error_token> Tokens;
		} gErrorHandler;

		inline static constexpr const bool is_alphabet(const char byte) noexcept
		{
			return ('a' <= byte && byte <= 'z') || ('A' <= byte && byte <= 'Z');
		}

		inline static constexpr const bool is_digit(const char byte) noexcept
		{
			return ('0' <= byte && byte <= '9');
		}

		std::unordered_set<std::string> gKeywords =
		{
			// TODO: uint32 타입 추가 필요
			// TODO: 더 많은 signed/unsigned integer 타입 추가 필요
			// TODO: decimal 타입 추가 필요
			"int32",
		};
	}
}

mcf::lexer::lexer(const std::string input) noexcept
	: _input(input)
{
	if (_input.length() == 0)
	{
		internal::gErrorHandler.Tokens.push(lexer::error_token::invalid_input_length);
		return;
	}

	read_next_byte();
}

mcf::lexer::~lexer(void) noexcept
{}

const mcf::lexer::error_token mcf::lexer::get_last_error_token(void) noexcept
{
	if (internal::gErrorHandler.Tokens.empty()) 
	{ 
		return lexer::error_token::no_error;
	}

	const mcf::lexer::error_token lError = internal::gErrorHandler.Tokens.top();
	internal::gErrorHandler.Tokens.pop();
	return lError;
}

const mcf::token mcf::lexer::read_next_token() noexcept
{
	mcf::token lToken;

	// 공백 문자는 스킵 하도록 합니다.
	while (_currentByte == ' ' || _currentByte == '\t' || _currentByte == '\n' || _currentByte == '\r')
	{
		read_next_byte();
	}

	switch (_currentByte)
	{
	case 0:
		lToken = { token_type::eof, "" + _currentByte };
		break;
	case '=':
		lToken = { token_type::assign, std::string(1, _currentByte) };
		break;
	case '+':
		lToken = { token_type::plus, std::string(1, _currentByte) };
		break;
	case '-':
		lToken = { token_type::minus, std::string(1, _currentByte) };
		break;
	case '*':
		lToken = { token_type::asterisk, std::string(1, _currentByte) };
		break;
	case '/':
		lToken = { token_type::slash, std::string(1, _currentByte) };
		break;
	case ';':
		lToken = { token_type::semicolon, std::string(1, _currentByte) };
		break;
	default:
		// keyword + identifier 는 첫 시작이 '_' 이거나 알파벳 이어야만 합니다.
		if (internal::is_alphabet(_currentByte) || _currentByte == '_')
		{
			// TODO: 0x (16진수), 0 (8진수), 또는 0b (2진수) 숫자의 토큰을 생성 가능하게 개선 필요
			lToken.Literal = read_keyword_or_identifier();
			lToken.Type = internal::gKeywords.find(lToken.Literal) == internal::gKeywords.end() ? mcf::token_type::identifier : mcf::token_type::keyword;
			return lToken;
		}
		else if (internal::is_digit(_currentByte))
		{
			lToken = { token_type::integer_32bit, read_number() };
			// TODO: decimal 토큰을 생성 가능하게 개선 필요
			// TODO: 이후 postfix 로 타입 지정 가능하게 개선 필요
			return lToken;
		}
		else
		{
			lToken = { token_type::illegal, std::string(1, _currentByte) };
			wdefault_break(L"예상치 못한 바이트 값이 들어 왔습니다. 토큰 생성에 실패 하였습니다. 현재 바이트[%u], ascii[%c]", _currentByte, _currentByte);
		}
	}

	read_next_byte();
	return lToken;
}

void mcf::lexer::read_next_byte(void) noexcept
{
	const size_t lLength = _input.length();
	wdebug_assert(_nextPosition <= lLength, L"nextPosition 은 inputLength 보다 크거나 같을 수 없습니다!. inputLength=%llu, nextPosition=%llu", lLength, _nextPosition);

	_currentByte = (_nextPosition > lLength) ? 0 : _input[_nextPosition];
	_currentPosition = _nextPosition;
	_nextPosition += 1;
}

std::string mcf::lexer::read_keyword_or_identifier(void) noexcept
{
	wdebug_assert(internal::is_alphabet(_currentByte) || _currentByte == '_', L"키워드 혹은 식별자의 시작은 알파벳이거나 '_' 이어야만 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t lFirstLetterPosition = _currentPosition;
	read_next_byte();

	// 첫 문자 이후로는 알파벳, 숫자, 또는 '_' 이어야 합니다.
	while (internal::is_alphabet(_currentByte) || internal::is_digit(_currentByte) || (_currentByte == '_'))
	{
		read_next_byte();
	}
	return _input.substr(lFirstLetterPosition, _currentPosition - lFirstLetterPosition);
}

std::string mcf::lexer::read_number(void) noexcept
{
	wdebug_assert(internal::is_digit(_currentByte), L"숫자의 시작은 0부터 9까지의 문자여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t lFirstLetterPosition = _currentPosition;

	while (internal::is_digit(_currentByte))
	{
		read_next_byte();
	}
	return _input.substr(lFirstLetterPosition, _currentPosition - lFirstLetterPosition);
}