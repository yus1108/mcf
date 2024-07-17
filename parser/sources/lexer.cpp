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
				std::getline(file, line);
				if (file.fail() == true)
				{
					return std::string();
				}

				constexpr const unsigned char encodingArray[][5] =
				{
					{0xef, 0xbb, 0xbf, 0},
				};
				constexpr size_t encodingArraySize = array_size(encodingArray);
				constexpr size_t MAX_ENCODING_BYTE = 4;

				for (size_t i = 0; i < encodingArraySize; ++i)
				{
					bool foundHeader = true;
					size_t offset = 0;

					const size_t encodingSize = strlen(reinterpret_cast<const char*>(encodingArray[i]));
					for (size_t j = 0; j <= MAX_ENCODING_BYTE - encodingSize; j++)
					{
						foundHeader = true;
						offset = j;
						for (; encodingArray[i][offset] != 0; ++offset)
						{
							// encodingArray 를 초기화 할 때 unsigned char 로만 초기화 가능한데 char 과 unsigned 를 비교하려면
							// 타입을 맞춰줘야함.
							if (line[offset] != static_cast<char>(encodingArray[i][offset]))
							{
								foundHeader = false;
								break;
							}
						}
					}

					if (foundHeader == true)
					{
						line.erase(0, encodingSize);
						break;
					}
				}

				input += line;
				while (std::getline(file, line))
				{
					if (file.fail() == true)
					{
						return std::string();
					}
					// 이전에 읽은 라인이 없다면 새로운 라인을 만들지 않는다. 
					input += "\n" + line;
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
	// 공백 문자는 스킵 하도록 합니다.
	while (_currentByte == ' ' || _currentByte == '\t' || _currentByte == '\n' || _currentByte == '\r')
	{
		// EOF 를 맞나면 즉시 종료 루프를 빠져나옵니다.
		if (_currentByte == 0)
		{
			break;
		}
		else if ( _currentByte == '\n' )
		{
			_currentLine++;
			_currentIndex = 0;
		}
		read_next_byte();
	}

	constexpr const size_t TOKEN_COUNT_BEGIN = __COUNTER__;
	mcf::token lToken;
	switch (_currentByte)
	{
	case 0: __COUNTER__;
		// 입력의 끝에 도달 하였을때 read_next_byte()가 호출될 경우 에러가 발생하기 때문에 EOF 를 만나면 강제로 종료합니다.
		return { token_type::eof, "\0", _currentLine, _currentIndex };
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
	case '/': 
		__COUNTER__; // count for slash
		__COUNTER__; // count for comment
		__COUNTER__; // count for comment_block
		return read_slash_starting_token();
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
		__COUNTER__; // count for keyword_variadic
		return read_dot_starting_token();
	case '#':
		__COUNTER__; // count for macro_iibrary_file_include
		__COUNTER__; // count for macro_project_file_include
		return read_macro_token();
	default:
		// keyword + identifier 는 첫 시작이 '_' 이거나 알파벳 이어야만 합니다.
		if (internal::is_alphabet(_currentByte) || _currentByte == '_')
		{
			__COUNTER__; // count for identifier
			__COUNTER__; // count for keyword_int32
			__COUNTER__; // count for keyword_enum
			// TODO: #8 0x (16진수), 0 (8진수), 또는 0b (2진수) 숫자의 토큰을 생성 가능하게 개선 필요
			lToken.Literal = read_keyword_or_identifier(); 
			lToken.Type = internal::determine_keyword_or_identifier(lToken.Literal);
			lToken.Line = _currentLine;
			lToken.Index = _currentIndex;
			return lToken; 
		}
		else if (internal::is_digit(_currentByte))
		{
			__COUNTER__; // count for integer_32bit
			lToken = { token_type::integer_32bit, read_number(), _currentLine, _currentIndex };
			// TODO: #7, #9 decimal 토큰을 생성 가능하게 개선 필요
			// TODO: #10 이후 postfix 로 타입 지정 가능하게 개선 필요
			return lToken;
		}
		else
		{
			__COUNTER__; // count for keyword_identifier_start
			__COUNTER__; // count for keyword_identifier_end
			__COUNTER__; // count for macro_start
			__COUNTER__; // count for macro_end
			lToken = { token_type::invalid, std::string(1, _currentByte), _currentLine, _currentIndex };
			debug_break(u8"예상치 못한 바이트 값이 들어 왔습니다. 토큰 생성에 실패 하였습니다. 현재 바이트[%u], ascii[%c]", _currentByte, _currentByte);
		}
	}
	constexpr const size_t TOKEN_COUNT = __COUNTER__ - TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");

	read_next_byte();
	return lToken;
}


inline const char mcf::lexer::get_next_byte(void) const noexcept
{
	return _nextPosition < _input.length() ? _input[_nextPosition] : 0;
}

inline void mcf::lexer::read_next_byte(void) noexcept
{
	const size_t length = _input.length();
	debug_assert(_nextPosition <= length, u8"currentPosition 은 inputLength 보다 크거나 같을 수 없습니다!. inputLength=%llu, nextPosition=%llu", length, _nextPosition);

	_currentByte = (_nextPosition > length) ? 0 : _input[_nextPosition];
	_currentPosition = _nextPosition;
	_nextPosition += 1;
	_currentIndex += 1;
}

inline const bool mcf::lexer::read_line_if_start_with(std::string* optionalOut, const char* startWith) noexcept
{
	const size_t startWithLength = std::strlen(startWith);
	const size_t firstLetterPosition = _currentPosition;

	for (size_t i = 0; i < startWithLength; ++i)
	{
		if (_currentByte != startWith[i] || _currentByte == 0)
		{
			if (optionalOut != nullptr)
			{
				*optionalOut = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
			}
			return false;
		}
		read_next_byte();
	}

	while (_currentByte != 0 && _currentByte != '\r' && _currentByte != '\n')
	{
		read_next_byte();
	}
	if (optionalOut != nullptr)
	{
		*optionalOut = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
	}
	return true;
}

inline const bool mcf::lexer::read_and_validate(std::string* optionalOut, const char* stringToCompare) noexcept
{
	const size_t stringToCompareLength = std::strlen(stringToCompare);
	const size_t firstLetterPosition = _currentPosition;

	for ( size_t i = 0; i < stringToCompareLength; ++i )
	{
		if ( _currentByte != stringToCompare[i] || _currentByte == 0 )
		{
			if (optionalOut != nullptr)
			{
				*optionalOut = _input.substr( firstLetterPosition, _currentPosition - firstLetterPosition );
			}
			return false;
		}
		read_next_byte();
	}

	if ( optionalOut != nullptr )
	{
		*optionalOut = _input.substr( firstLetterPosition, _currentPosition - firstLetterPosition );
	}
	return true;
}

inline const bool mcf::lexer::read_and_validate(std::string* optionalOut, const char* startWith, const char* endWith, const char* invalidCharList) noexcept
{
	const size_t firstLetterPosition = _currentPosition;

	if (read_and_validate(optionalOut, startWith) == false)
	{
		return false;
	}

	// endWith 문자열인지 확인합니다.
	while (read_and_validate(nullptr, endWith) == false)
	{
		// 입력의 끝에 도달 하면 바로 종료
		if (_currentByte == 0)
		{
			debug_message(u8"`%s`으로 끝나기 전에 EOF에 도달하였습니다.", endWith);
			if (optionalOut != nullptr)
			{
				*optionalOut = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
			}
			return false;
		}

		// endWith 문자열이 아니고 EOF 도 아니라면 다음 입력을 받습니다.
		read_next_byte();

		if (invalidCharList == nullptr)
		{
			continue;
		}

		// 허용하지 않는 문자들이 들어 오게 되었을 때 강제 종료
		for (size_t i = 0; invalidCharList[i] != 0; ++i)
		{
			if (_currentByte == invalidCharList[i])
			{
				debug_message(u8"들어오면 안되는 문자가 들어왔습니다. 현재 문자=%c, 값=%d", _currentByte, _currentByte);
				if (optionalOut != nullptr)
				{
					*optionalOut = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
				}
				return false;
			}
		}
	}

	if (optionalOut != nullptr)
	{
		*optionalOut = _input.substr(firstLetterPosition + strlen(startWith), _currentPosition - firstLetterPosition - strlen(endWith));
	}
	return true;
}

inline const std::string mcf::lexer::read_keyword_or_identifier(void) noexcept
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

inline const std::string mcf::lexer::read_number(void) noexcept
{
	debug_assert(internal::is_digit(_currentByte), u8"숫자의 시작은 0부터 9까지의 문자여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;

	while (internal::is_digit(_currentByte))
	{
		read_next_byte();
	}
	return _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
}

inline const mcf::token mcf::lexer::read_slash_starting_token(void) noexcept
{
	debug_assert(_currentByte == '/', u8"이 함수가 호출될때 '/'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	// 처음 '/'를 읽습니다.
	const size_t firstLetterPosition = _currentPosition;
	read_next_byte();

	// 연속되는 문자열이 "//"(comment) 인지 검사합니다.
#if defined(_DEBUG)
	std::string debugOutput;
	if (read_line_if_start_with(&debugOutput, "/") == true)
#else
	if (read_line_if_start_with(nullptr, "/") == true)
#endif
	{
		return { token_type::comment, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	// 연속되는 문자열이 "/*[^"*/"]*/"(comment_block) 인지 검사합니다.
#if defined(_DEBUG)
	if (read_and_validate(&debugOutput, "*", "*/", nullptr) == true)
#else
	if (read_and_validate(&nullptr, "*", "*/", nullptr) == true)
#endif
	{
		return { token_type::comment_block, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	// 위의 comment_block("/*[^"*/"]*/")의 검사가 실패 하는 경우는 시작 문자열의 '*'스타를 못읽었거나 EOF 로 끝나는 경우 뿐인데 EOF로 인해 실패 하였다면 invalid 한 토큰을 리턴합니다.
	if (_currentByte == 0)
	{
		debug_message(u8"주석에서 예기치 않은 파일의 끝이 나타났습니다.");
		return { token_type::invalid, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	debug_assert(debugOutput.empty() == true, u8"결과 값이 '/'가 아닌데 token_type::slash를 반환 하려 하고 있습니다! 현재 파싱된 문자열=`%s` 다음 문자='%c'(%d)",
		_input.substr(firstLetterPosition, _currentPosition - firstLetterPosition).c_str(), get_next_byte(), get_next_byte());
	return { token_type::slash, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}

inline const mcf::token mcf::lexer::read_dot_starting_token(void) noexcept
{
	debug_assert(_currentByte == '.', u8"이 함수가 호출될때 '.'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;

	// 연속되는 문자열이 "..."(keyword_variadic) 인지 검사합니다.
	read_next_byte();
	while (_currentByte == '.')
	{
		read_next_byte();
	}

	// 연속되는 문자열이 "..."(keyword_variadic) 인지 검사합니다.
	const std::string variadicCandidate = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
	if (variadicCandidate == "...")
	{
		return { token_type::keyword_variadic, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	// "..." 보다 '.' 문자가 더 많다면 에러를 발생 시킵니다..
	return { token_type::invalid, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}

inline const mcf::token mcf::lexer::read_macro_token( void ) noexcept
{
	debug_assert( _currentByte == '#', u8"매크로는 '#'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte );

	// 매크로 시작 문자열에 부합하는 키워드만 등록합니다.
	constexpr const char* MACRO_START_WITH[] =
	{
		"#include", // macro_iibrary_file_include
		"#include", // macro_project_file_include
	};
	constexpr const size_t MACRO_START_WITH_SIZE = array_size(MACRO_START_WITH);
	static_assert(enum_index(token_type::macro_end) - enum_index(token_type::macro_start) - 1 == MACRO_START_WITH_SIZE,
		"macro token_type count is changed. this array need to be changed as well.");

	token_type tokenType = token_type::invalid;

	const size_t firstLetterPosition = _currentPosition;
	
	// 첫 문자('#') 이후로는 알파벳이어야 합니다.
	read_next_byte();
	while (internal::is_alphabet(_currentByte))
	{
		read_next_byte();
	}

	std::string macroType = _input.substr( firstLetterPosition, _currentPosition - firstLetterPosition );
	for (size_t i = 0; i < MACRO_START_WITH_SIZE; i++)
	{
		if (macroType == MACRO_START_WITH[i])
		{
			tokenType = enum_at<token_type>(enum_index(token_type::macro_start) + i + 1);
			break;
		}
	}

	constexpr const size_t MACRO_COUNT_BEGIN = __COUNTER__;
	std::string target;
	switch (tokenType)
	{
	case token_type::macro_iibrary_file_include: __COUNTER__;
	case token_type::macro_project_file_include: __COUNTER__;
		// 공백 문자는 스킵합니다.
		while (_currentByte == ' ' || _currentByte == '\t')
		{
			read_next_byte();
		}

		if (_currentByte == '<' && read_and_validate(nullptr, "<", ">", "\n\r") == true)
		{
			return { token_type::macro_iibrary_file_include, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
		}
		else if (_currentByte == '"' && read_and_validate(nullptr, "\"", "\"", "\n\r") == true)
		{
			return { token_type::macro_project_file_include, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
		}
		debug_break(u8"#include <target> 매크로의 토큰 생성에 실패 하였습니다. 현재 바이트[%u], ascii[%c]", _currentByte, _currentByte);

	case token_type::invalid:
		break;

	default:
		debug_break(u8"예상치 못한 바이트 값이 들어 왔습니다. 토큰 생성에 실패 하였습니다. 현재 바이트[%u], ascii[%c]", _currentByte, _currentByte);
	}
	constexpr const size_t MACRO_COUNT = __COUNTER__ - MACRO_COUNT_BEGIN - 1;
	static_assert(enum_index(token_type::macro_end) - enum_index(token_type::macro_start) - 1 == MACRO_COUNT,
		"macro token_type count is changed. this switch need to be changed as well.");

	return { token_type::invalid, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}