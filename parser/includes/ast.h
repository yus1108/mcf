#pragma once

namespace mcf
{
	template<typename T>
	class pointer
	{
	public:
		pointer(T* data) noexcept : _data(data) {}
		pointer(void) noexcept = delete;
		pointer(const pointer& data) noexcept = default;
		pointer(pointer&& data) noexcept = default;

		inline pointer& operator=(const T* data) noexcept = delete;
		inline pointer& operator=(const pointer& other) noexcept = delete;
		inline pointer& operator=( pointer&& other) noexcept = delete;

		inline bool	operator==(const T* rhs) const noexcept { return _data == rhs; }
		inline bool	operator==(const pointer& rhs) const noexcept { return _data == rhs._data; }
		inline bool	operator!=(const T* rhs) const noexcept { return _data != rhs; }
		inline bool	operator!=(const pointer& rhs) const noexcept { return _data != rhs._data; }

		inline const T* operator->(void) const noexcept { return _data; }
		inline const T& operator*(void) const noexcept { return *_data; }

		inline T*		get_unsafe_pointer(void) noexcept { return _data; }
		inline const T* get_unsafe_pointer(void) const noexcept { return _data; }

		template <typename OtherType>
		inline const pointer<OtherType>	cast_to(void) const noexcept { return static_cast<OtherType*>(_data); }

	private:
		const T* _data = nullptr;
	};

	namespace ast
	{
		class node
		{
		public:
			virtual ~node(void) noexcept {}
			virtual const std::string get_token_literal(void) const noexcept = 0;
		};

		class statement : public node
		{
		public: virtual const std::string get_token_literal(void) const noexcept override = 0;
		};

		class expression : public node
		{
		public: virtual const std::string get_token_literal(void) const noexcept override = 0;
		};

		class program final
		{
		public:
			~program(void) {}

			const std::string get_token_literal(void) const noexcept { return _statements.size() > 0 ? _statements[0]->get_token_literal() : std::string(); }

			inline const size_t								get_statement_size( void ) const noexcept;
			inline mcf::pointer<mcf::ast::statement>		get_statement_at(const size_t index);
			inline const mcf::pointer<mcf::ast::statement>	get_statement_at(const size_t index) const;

		private:
			std::vector<std::unique_ptr<mcf::ast::statement>> _statements;
		};

		class literal_expession : public expression
		{
		public: 
			virtual const std::string get_token_literal(void) const noexcept override { return _token.Literal; }

		private:
			// TODO: 다른 literal 토큰 추가시 적용되게 수정
			const mcf::token	_token; // token_type::integer_32bit
			const std::string	_value;
		};

		class identifier_expression : public expression
		{
		public:
			virtual const std::string get_token_literal(void) const noexcept override { return _token.Literal; }

		private:
			const mcf::token	_token; // token_type::identifier
			const std::string	_value;
		};

		class data_type_expression : public expression
		{
		public:
			virtual const std::string get_token_literal(void) const noexcept override { return _token.Literal; }

		private:
			const mcf::token				_token; // token_type::keyword
			const mcf::token_keyword_type	_keywordType;
		};

		class variable_declaration_statement : public statement
		{
		public:
			virtual const std::string get_token_literal(void) const noexcept override { return _dataType.get_token_literal(); }

		private:
			const mcf::ast::data_type_expression	_dataType; // { keyword, uint32 }
			const std::string						_name;
			const std::unique_ptr<expression>		_expression;
		};
	}
}
