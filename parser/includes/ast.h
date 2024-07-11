#pragma once

namespace mcf
{
	template<typename T>
	class pointer
	{
	public:
		pointer( T* data ) noexcept : _data( data ) {}
		pointer( const T* data ) noexcept : _data( data ) {}
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
		inline const	pointer<OtherType> cast_to(void) const noexcept { return static_cast<OtherType*>(_data); }
		template <typename OtherType>
		inline			pointer<OtherType> cast_to( void ) noexcept { return static_cast<OtherType*>(_data); }

	private:
		T* _data = nullptr;
	};

	namespace ast
	{
		enum class node_type : unsigned char
		{
			expression,
			statement,

			// 이 밑으로는 수정하면 안됩니다.
			count,
		};

		class node
		{
		public:
			virtual ~node(void) noexcept {}
			virtual const mcf::ast::node_type	get_node_type(void) const noexcept = 0;
		};

		enum class statement_type : unsigned char
		{
			variable_declaration,

			// 이 밑으로는 수정하면 안됩니다.
			count,
		};

		class statement : public node
		{
		public: 
					virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept = 0;
			inline	virtual const mcf::ast::node_type		get_node_type(void) const noexcept final { return mcf::ast::node_type::statement; }
		};

		enum class expression_type : unsigned char
		{
			literal,
			identifier,
			data_type,

			// 이 밑으로는 수정하면 안됩니다.
			count,
		};

		class expression : public node
		{
		public: 
					virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept = 0;
			inline	virtual const mcf::ast::node_type		get_node_type(void) const noexcept final { return mcf::ast::node_type::expression; }
		};

		class program final
		{
		public:
			~program(void) {}

			const size_t							get_statement_size(void) const noexcept;
			mcf::pointer<mcf::ast::statement>		get_statement_at(const size_t index);
			const mcf::pointer<mcf::ast::statement>	get_statement_at(const size_t index) const;

		private:
			std::vector<std::unique_ptr<mcf::ast::statement>> _statements;
		};

		class literal_expession final : public expression
		{
		public: 
			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept final { return mcf::ast::expression_type::literal; }

		private:
			// TODO: 다른 literal 토큰 추가시 적용되게 수정
			const mcf::token	_token; // token_type::integer_32bit
			const std::string	_value;
		};

		class identifier_expression final : public expression
		{
		public:
			inline const mcf::token&  get_token( void ) const noexcept { return _token; }
			inline const std::string& get_value( void ) const noexcept { return _value; }

			inline virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept final { return mcf::ast::expression_type::literal; }

		private:
			const mcf::token	_token; // token_type::identifier
			const std::string	_value;
		};

		class data_type_expression final : public expression
		{
		public:
			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept final { return mcf::ast::expression_type::literal; }

		private:
			const mcf::token				_token; // token_type::keyword
			const mcf::token_keyword_type	_keywordType;
		};

		class variable_declaration_statement final : public statement
		{
		public:
			inline const std::string& get_name( void ) const { return _name.get_value(); }
			inline const mcf::token&  get_name_token( void ) const { return _name.get_token(); }

			inline	virtual const mcf::ast::statement_type	get_expression_type(void) const noexcept final { return mcf::ast::statement_type::variable_declaration; }

		private:
			const mcf::ast::data_type_expression	_dataType; // { keyword, uint32 }
			const mcf::ast::identifier_expression	_name;
			const std::unique_ptr<expression>		_expression;
		};
	}
}
