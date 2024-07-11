#pragma once

namespace mcf
{
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
			inline	virtual const mcf::ast::node_type		get_node_type(void) const noexcept override final { return mcf::ast::node_type::statement; }
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
			inline	virtual const mcf::ast::node_type		get_node_type(void) const noexcept override final { return mcf::ast::node_type::expression; }
		};

		class literal_expession final : public expression
		{
		public: 
			explicit literal_expession(void) noexcept = default;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::literal; }

		private:
			// TODO: 다른 literal 토큰 추가시 적용되게 수정
			const mcf::token	_token = { token_type::invalid, std::string() }; // { token_type::integer_32bit, literal }
			const std::string	_value;
		};

		class identifier_expression final : public expression
		{
		public:
			explicit identifier_expression(void) noexcept = default;
			explicit identifier_expression(const mcf::token& token) noexcept : _token(token) {}

			inline const mcf::token&  get_token( void ) const noexcept { return _token; }

			inline virtual const mcf::ast::expression_type get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::literal; }

		private:
			const mcf::token _token = { token_type::invalid, std::string() }; // { token_type::identifier, identifier }
		};

		class data_type_expression final : public expression
		{
		public:
			explicit data_type_expression(void) noexcept = default;
			explicit data_type_expression(const mcf::token& token) noexcept : _token(token) {}

			inline const mcf::token_type get_type(void) const noexcept { return _token.Type; }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::literal; }

		private:
			const mcf::token _token = { token_type::invalid, std::string() }; // { token_type::keyword, "int32" }
		};

		class variable_declaration_statement final : public statement
		{
		public:
			explicit variable_declaration_statement(void) noexcept = default;
			explicit variable_declaration_statement(const mcf::ast::data_type_expression& dataType,
													const mcf::ast::identifier_expression& name,
													const mcf::ast::expression* rightExpression) noexcept;

			inline const mcf::token_type	get_type(void) const noexcept { return _dataType.get_type(); }
			inline const std::string&		get_name(void) const noexcept { return _name.get_token().Literal; }

			inline virtual const mcf::ast::statement_type get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::variable_declaration; }

		private:
			const mcf::ast::data_type_expression				_dataType; // { keyword, int32 }
			const mcf::ast::identifier_expression				_name;
			const std::unique_ptr<const mcf::ast::expression>	_rightExpression;
		};
	}
}
