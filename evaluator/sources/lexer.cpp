#include "pch.h"
#include "lexer.h"

namespace mcf
{
	namespace internal 
	{
		constexpr static const bool IS_ALPHABET(const char byte) noexcept
		{
			return ('a' <= byte && byte <= 'z') || ('A' <= byte && byte <= 'Z');
		}

		inline static const std::string ReadFile(const std::string& path)
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

				constexpr const unsigned char ENCODING_ARRAY[][5] =
				{
					{0xef, 0xbb, 0xbf, 0},
				};
				constexpr size_t encodingArraySize = MCF_ARRAY_SIZE(ENCODING_ARRAY);
				constexpr size_t MAX_ENCODING_BYTE = 4;

				for (size_t i = 0; i < encodingArraySize; ++i)
				{
					bool foundHeader = true;
					size_t offset = 0;

					const size_t encodingSize = strlen(reinterpret_cast<const char*>(ENCODING_ARRAY[i]));
					for (size_t j = 0; j <= MAX_ENCODING_BYTE - encodingSize; j++)
					{
						foundHeader = true;
						offset = j;
						for (; ENCODING_ARRAY[i][offset] != 0; ++offset)
						{
							// encodingArray 를 초기화 할 때 unsigned char 로만 초기화 가능한데 char 과 unsigned 를 비교하려면
							// 타입을 맞춰줘야함.
							if (line[offset] != static_cast<char>(ENCODING_ARRAY[i][offset]))
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

const mcf::Token::Data mcf::Token::FindPredefinedKeyword(const std::string& tokenLiteral) noexcept
{
	// identifier 조건에 부합하는 키워드만 등록합니다.
	constexpr const char* IDENTIFIER_KEYWORDS[] =
	{
		"asm",
		"extern",
		"typedef",
		"bind",
		"let",
		"func",
		"main",
		"void",
		"unsigned",
		"return",
		"unused",
	};
	constexpr const size_t KEYWORDS_SIZE = MCF_ARRAY_SIZE(IDENTIFIER_KEYWORDS);
	static_assert(ENUM_INDEX(Type::KEYWORD_IDENTIFIER_END) - ENUM_INDEX(Type::KEYWORD_IDENTIFIER_START) - 1 == KEYWORDS_SIZE,
		"identifier keyword TokenType count is changed. this array need to be changed as well.");

	// 기본 제공 키워드 검색
	for (size_t i = 0; i < KEYWORDS_SIZE; i++)
	{
		if (tokenLiteral.compare(IDENTIFIER_KEYWORDS[i]) == 0)
		{
			return Data{ mcf::ENUM_AT<Type>(ENUM_INDEX(Type::KEYWORD_IDENTIFIER_START) + i + 1), tokenLiteral, 0, 0 };
		}
	}

	return Data{ Type::INVALID, "INVALID", 0, 0 };
}

mcf::Lexer::Object::Object(const std::string& input, const bool isFIle) noexcept
	: _input(isFIle ? internal::ReadFile(input) : input)
	, _name(input)
{
	if ( _input.length() == 0 )
	{
		if (isFIle)
		{
			std::ifstream file(_name.c_str());
			if (file.fail())
			{
				_tokens.push(Error::FAIL_READ_FILE);
				return;
			}
		}
		_tokens.push(Error::INVALID_INPUT_LENGTH);
		return;
	}

	ReadNextByte();
	// reset index to 0 after read next byte
	_currentIndex = 0;
}

const mcf::Lexer::Error mcf::Lexer::Object::GetLastErrorToken(void) noexcept
{
	if (_tokens.empty())
	{ 
		return Error::SUCCESS;
	}

	const Error lError = _tokens.top();
	_tokens.pop();
	return lError;
}

const mcf::Token::Data mcf::Lexer::Object::ReadNextToken(void) noexcept
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
		ReadNextByte();
	}

	constexpr const size_t TOKEN_COUNT_BEGIN = __COUNTER__;
	mcf::Token::Data token;
	switch (_currentByte)
	{
	case 0: __COUNTER__;
		// 입력의 끝에 도달 하였을때 read_next_byte()가 호출될 경우 에러가 발생하기 때문에 EOF 를 만나면 강제로 종료합니다.
		return { Token::Type::END_OF_FILE, "\0", _currentLine, _currentIndex };
	case '"': __COUNTER__;
		return ReadStringUtf8();
	case '=':
	{
		__COUNTER__; // count for ASSIGN
		__COUNTER__; // count for EQUAL

		const size_t firstLetterPosition = _currentPosition;

		// 연속되는 문자열이 "==" 인지 검사합니다.
		ReadNextByte();
		if (_currentByte == '=')
		{
			ReadNextByte();
			return { Token::Type::EQUAL, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
		}
		return { Token::Type::ASSIGN, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}
	case '+': __COUNTER__;
		token = { Token::Type::PLUS, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '-': 
	{
		__COUNTER__; // count for MINUS
		__COUNTER__; // count for POINTING

		const size_t firstLetterPosition = _currentPosition;

		// 연속되는 문자열이 "->" 인지 검사합니다.
		ReadNextByte();
		if ( _currentByte == '>' )
		{
			ReadNextByte();
			return { Token::Type::POINTING, _input.substr( firstLetterPosition, _currentPosition - firstLetterPosition ), _currentLine, _currentIndex };
		}
		return { Token::Type::MINUS, _input.substr( firstLetterPosition, _currentPosition - firstLetterPosition ), _currentLine, _currentIndex };
	}
	case '*': __COUNTER__;
		token = { Token::Type::ASTERISK, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '/': 
		__COUNTER__; // count for SLASH
		__COUNTER__; // count for COMMENT
		__COUNTER__; // count for COMMENT_BLOCK
		return ReadSlashStartingToken();
	case '!': 
	{
		__COUNTER__; // count for BANG
		__COUNTER__; // count for NOT_EQUAL

		const size_t firstLetterPosition = _currentPosition;

		// 연속되는 문자열이 "!=" 인지 검사합니다.
		ReadNextByte();
		if (_currentByte == '=')
		{
			ReadNextByte();
			return { Token::Type::NOT_EQUAL, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
		}
		return { Token::Type::BANG, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}
	case '<': __COUNTER__;
		token = { Token::Type::LT, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '>': __COUNTER__;
		token = { Token::Type::GT, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '&': __COUNTER__;
		token = { Token::Type::AMPERSAND, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '(': __COUNTER__;
		token = { Token::Type::LPAREN, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ')': __COUNTER__;
		token = { Token::Type::RPAREN, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '{': __COUNTER__;
		token = { Token::Type::LBRACE, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '}': __COUNTER__;
		token = { Token::Type::RBRACE, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '[': __COUNTER__;
		token = { Token::Type::LBRACKET, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ']': __COUNTER__;
		token = { Token::Type::RBRACKET, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ':': 
	{
		__COUNTER__; // count for COLON
		__COUNTER__; // count for DOUBLE_COLON

		const size_t firstLetterPosition = _currentPosition;

		// 연속되는 문자열이 "::" 인지 검사합니다.
		ReadNextByte();
		if (_currentByte == ':')
		{
			ReadNextByte();
			return { Token::Type::DOUBLE_COLON, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
		}
		return { Token::Type::COLON, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}
	case ';': __COUNTER__;
		token = { Token::Type::SEMICOLON, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case ',': __COUNTER__;
		token = { Token::Type::COMMA, std::string(1, _currentByte), _currentLine, _currentIndex };
		break;
	case '.':
		__COUNTER__; // count for VARIADIC
		return ReadDotStartingToken();
	case '#':
		__COUNTER__; // count for MACRO_INCLUDE
		return ReadMacroToken();
	default: 
		// 토큰 리터럴의 시작 문자가 문자열 그룹 패턴에 속한 경우 default 에서 처리합니다.
		// keyword + identifier 는 첫 시작이 '_' 이거나 알파벳 이어야만 합니다.
		if (internal::IS_ALPHABET(_currentByte) || _currentByte == '_')
		{
			__COUNTER__; // count for IDENTIFIER
			__COUNTER__; // count for KEYWORD_ASM
			__COUNTER__; // count for KEYWORD_EXTERN
			__COUNTER__; // count for KEYWORD_TYPEDEF
			__COUNTER__; // count for KEYWORD_BIND
			__COUNTER__; // count for KEYWORD_LET
			__COUNTER__; // count for KEYWORD_FUNC
			__COUNTER__; // count for KEYWORD_MAIN
			__COUNTER__; // count for KEYWORD_VOID
			__COUNTER__; // count for KEYWORD_UNSIGNED
			__COUNTER__; // count for KEYWORD_RETURN
			__COUNTER__; // count for KEYWORD_UNUSED
			token.Literal = ReadKeywordOrIdentifier(); 
			token.Type = DetermineKeywordOrIdentifier(token.Literal);
			token.Line = _currentLine;
			token.Index = _currentIndex;
			return token; 
		}
		else if (mcf::Internal::IS_DIGIT(_currentByte))
		{
			__COUNTER__; // count for INTEGER
			return ReadNumeric();
		}
		else
		{
			__COUNTER__; // count for KEYWORD_IDENTIFIER_START
			__COUNTER__; // count for KEYWORD_IDENTIFIER_END
			__COUNTER__; // count for MACRO_START
			__COUNTER__; // count for MACRO_END
			token = { Token::Type::INVALID, std::string(1, _currentByte), _currentLine, _currentIndex };
			MCF_DEBUG_BREAK(u8"예상치 못한 바이트 값이 들어 왔습니다. 토큰 생성에 실패 하였습니다. 현재 바이트[%u], ascii[%c]", _currentByte, _currentByte);
			break;
		}
	}
	constexpr const size_t TOKEN_COUNT = __COUNTER__ - TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == TOKEN_COUNT, "TokenType count is changed. this SWITCH need to be changed as well.");

	ReadNextByte();
	return token;
}

inline const char mcf::Lexer::Object::GetNextByte(void) const noexcept
{
	return _nextPosition < _input.length() ? _input[_nextPosition] : 0;
}

inline void mcf::Lexer::Object::ReadNextByte(void) noexcept
{
	const size_t length = _input.length();
	MCF_DEBUG_ASSERT(_nextPosition <= length, u8"currentPosition 은 inputLength 보다 크거나 같을 수 없습니다!. inputLength=%llu, nextPosition=%llu", length, _nextPosition);

	_currentByte = (_nextPosition > length) ? 0 : _input[_nextPosition];
	_currentPosition = _nextPosition;
	_nextPosition += 1;
	_currentIndex += 1;
}

inline const bool mcf::Lexer::Object::ReadLineIfStartWith(_Outptr_opt_ std::string* optionalOut, const char* _In_opt_ startWith) noexcept
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
		ReadNextByte();
	}

	while (_currentByte != 0 && _currentByte != '\r' && _currentByte != '\n')
	{
		ReadNextByte();
	}
	if (optionalOut != nullptr)
	{
		*optionalOut = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
	}
	return true;
}

inline const bool mcf::Lexer::Object::ReadAndValidate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* stringToCompare) noexcept
{
	MCF_DEBUG_ASSERT(stringToCompare != nullptr, u8"stringToCompare가 null일 수 없습니다.");

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
		ReadNextByte();
	}

	if ( optionalOut != nullptr )
	{
		*optionalOut = _input.substr( firstLetterPosition, _currentPosition - firstLetterPosition );
	}
	return true;
}

inline const bool mcf::Lexer::Object::ReadAndValidate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* startWith, _In_opt_ const char* endWith, _In_opt_ const char* invalidCharList) noexcept
{
	MCF_DEBUG_ASSERT(startWith != nullptr || endWith != nullptr, u8"startWith와 endWith 둘다 null이면 안됩니다.");
	MCF_DEBUG_ASSERT(endWith != nullptr || (endWith == nullptr && invalidCharList != nullptr), u8"endWith가 null인 경우 invalidCharList는 null이면 안됩니다.");

	const size_t firstLetterPosition = _currentPosition;

	if (startWith != nullptr && ReadAndValidate(optionalOut, startWith) == false)
	{
		return false;
	}

	if (endWith == nullptr)
	{
		if (optionalOut != nullptr)
		{
			*optionalOut = _input.substr(firstLetterPosition + strlen(startWith), _currentPosition - firstLetterPosition - strlen(endWith));
		}
		return true;
	}

	// endWith 문자열인지 확인합니다.
	while (ReadAndValidate(nullptr, endWith) == false)
	{
		// 입력의 끝에 도달 하면 바로 종료
		if (_currentByte == 0)
		{
			MCF_DEBUG_BREAK(u8"`%s`으로 끝나기 전에 EOF에 도달하였습니다.", endWith);
			if (optionalOut != nullptr)
			{
				*optionalOut = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
			}
			return false;
		}

		// endWith 문자열이 아니고 EOF 도 아니라면 다음 입력을 받습니다.
		ReadNextByte();

		if (invalidCharList == nullptr)
		{
			continue;
		}

		// 허용하지 않는 문자들이 들어 오게 되었을 때 강제 종료
		for (size_t i = 0; invalidCharList[i] != 0; ++i)
		{
			if (_currentByte == invalidCharList[i])
			{
				MCF_DEBUG_BREAK(u8"들어오면 안되는 문자가 들어왔습니다. 현재 문자=%c, 값=%d", _currentByte, _currentByte);
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

inline const std::string mcf::Lexer::Object::ReadKeywordOrIdentifier(void) noexcept
{
	MCF_DEBUG_ASSERT(internal::IS_ALPHABET(_currentByte) || _currentByte == '_', u8"키워드 혹은 식별자의 시작은 알파벳이거나 '_' 이어야만 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;
	ReadNextByte();

	// 첫 문자 이후로는 알파벳, 숫자, 또는 '_' 이어야 합니다.
	while (internal::IS_ALPHABET(_currentByte) || mcf::Internal::IS_DIGIT(_currentByte) || (_currentByte == '_'))
	{
		ReadNextByte();
	}
	return _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
}

inline const std::string mcf::Lexer::Object::ReadNumber(void) noexcept
{
	MCF_DEBUG_ASSERT(mcf::Internal::IS_DIGIT(_currentByte), u8"숫자의 시작은 0부터 9까지의 문자여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;

	while (mcf::Internal::IS_DIGIT(_currentByte))
	{
		ReadNextByte();
	}
	return _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
}

const mcf::Token::Data mcf::Lexer::Object::ReadStringUtf8(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentByte == '"', u8"이 함수가 호출될때 '\"'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	// 처음 '"'를 읽습니다.
	const size_t firstLetterPosition = _currentPosition;
	ReadNextByte();

	// 연속되는 문자열이 `"[^"\n\r]*"`(STRING) 인지 검사합니다.
	if (ReadAndValidate(nullptr, nullptr, "\"", "\n\r") == true)
	{
		// TODO: #24 valid utf8 문자열인지 확인 필요, 아닌 경우 INVALID 토큰을 보냅고 토큰 생성 실패 시킵니다.
		return { Token::Type::STRING, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	return { Token::Type::INVALID, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}

inline const mcf::Token::Data mcf::Lexer::Object::ReadSlashStartingToken(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentByte == '/', u8"이 함수가 호출될때 '/'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	// 처음 '/'를 읽습니다.
	const size_t firstLetterPosition = _currentPosition;
	ReadNextByte();

	// 연속되는 문자열이 `//`(comment) 인지 검사합니다.
	if (ReadLineIfStartWith(nullptr, "/") == true)
	{
		return { Token::Type::COMMENT, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	// 연속되는 문자열이 `/*[^"*/"]*/`(comment_block) 인지 검사합니다.
	if (ReadAndValidate(nullptr, "*", "*/", nullptr) == true)
	{
		return { Token::Type::COMMENT_BLOCK, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	std::string tokenLiteral = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
	// 위의 comment_block(`/*[^"*/"]*/`)의 검사가 실패 하는 경우는 시작 문자열의 '*'스타를 못읽었거나 EOF 로 끝나는 경우 뿐이므로
	// 문자열이 "/*"로 시작 하는 상태에서 EOF로 인해 실패 하였다면 INVALID 한 토큰을 리턴합니다.
	if ((tokenLiteral.rfind("/*", 0) == 0) && _currentByte == 0)
	{
		MCF_DEBUG_BREAK(u8"주석에서 예기치 않은 파일의 끝이 나타났습니다.");
		return { Token::Type::INVALID, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	return { Token::Type::SLASH, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}

inline const mcf::Token::Data mcf::Lexer::Object::ReadDotStartingToken(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentByte == '.', u8"이 함수가 호출될때 '.'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	const size_t firstLetterPosition = _currentPosition;

	// 연속되는 문자열이 "..."(VARIADIC) 인지 검사합니다.
	ReadNextByte();
	while (_currentByte == '.')
	{
		ReadNextByte();
	}

	// 연속되는 문자열이 "..."(VARIADIC) 인지 검사합니다.
	const std::string variadicCandidate = _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition);
	if (variadicCandidate == "...")
	{
		return { Token::Type::VARIADIC, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
	}

	// "..." 보다 '.' 문자가 더 많다면 에러를 발생 시킵니다..
	return { Token::Type::INVALID, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}

inline const mcf::Token::Data mcf::Lexer::Object::ReadMacroToken( void ) noexcept
{
	MCF_DEBUG_ASSERT( _currentByte == '#', u8"매크로는 '#'으로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte );

	// 매크로 시작 문자열에 부합하는 키워드만 등록합니다.
	constexpr const char* MACRO_START_WITH[] =
	{
		"#include", // MACRO_INCLUDE
	};
	constexpr const size_t MACRO_START_WITH_SIZE = MCF_ARRAY_SIZE(MACRO_START_WITH);
	static_assert(ENUM_INDEX(Token::Type::MACRO_END) - ENUM_INDEX(Token::Type::MACRO_START) - 1 == MACRO_START_WITH_SIZE,
		"macro TokenType count is changed. this array need to be changed as well.");

	Token::Type tokenType = Token::Type::INVALID;

	const size_t firstLetterPosition = _currentPosition;
	
	// 첫 문자('#') 이후로는 알파벳이어야 합니다.
	ReadNextByte();
	while (internal::IS_ALPHABET(_currentByte))
	{
		ReadNextByte();
	}

	std::string macroType = _input.substr( firstLetterPosition, _currentPosition - firstLetterPosition );
	for (size_t i = 0; i < MACRO_START_WITH_SIZE; i++)
	{
		if (macroType == MACRO_START_WITH[i])
		{
			tokenType = ENUM_AT<Token::Type>(ENUM_INDEX(Token::Type::MACRO_START) + i + 1);
			break;
		}
	}

	constexpr const size_t MACRO_COUNT_BEGIN = __COUNTER__;
	std::string target;
	switch (tokenType)
	{
	case Token::Type::MACRO_INCLUDE: __COUNTER__;
		return { Token::Type::MACRO_INCLUDE, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };

	case Token::Type::INVALID:
		break;

	default:
		MCF_DEBUG_BREAK(u8"예상치 못한 바이트 값이 들어 왔습니다. 토큰 생성에 실패 하였습니다. 현재 바이트[%u], ascii[%c]", _currentByte, _currentByte);
		break;
	}
	constexpr const size_t MACRO_COUNT = __COUNTER__ - MACRO_COUNT_BEGIN - 1;
	static_assert(ENUM_INDEX(Token::Type::MACRO_END) - ENUM_INDEX(Token::Type::MACRO_START) - 1 == MACRO_COUNT,
		"macro TokenType count is changed. this switch need to be changed as well.");

	return { Token::Type::INVALID, _input.substr(firstLetterPosition, _currentPosition - firstLetterPosition), _currentLine, _currentIndex };
}

inline const mcf::Token::Data mcf::Lexer::Object::ReadNumeric(void) noexcept
{
	MCF_DEBUG_ASSERT(mcf::Internal::IS_DIGIT(_currentByte), u8"이 함수가 호출될 때 숫자로 시작하여야 합니다. 시작 문자=%c, 값=%d", _currentByte, _currentByte);

	mcf::Token::Data token;

	token.Literal = ReadNumber();
	token.Type = Token::Type::INTEGER; // 타입이 명시되지 않은 정수 리터럴의 기본 값은 64bit integer 입니다.

	// TODO: #8 0x (16진수), 0 (8진수), 또는 0b (2진수) 숫자의 토큰을 생성 가능하게 개선 필요
	// TODO: #9 decimal 토큰을 생성 가능하게 개선 필요
	token.Line = _currentLine;
	token.Index = _currentIndex;
	return token;
}

const mcf::Token::Type mcf::Lexer::Object::DetermineKeywordOrIdentifier(const std::string& tokenLiteral) noexcept
{
	const mcf::Token::Type tokenFound = Token::FindPredefinedKeyword(tokenLiteral).Type;
	return tokenFound != Token::Type::INVALID ? tokenFound : mcf::Token::Type::IDENTIFIER;
}