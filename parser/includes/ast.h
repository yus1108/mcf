#pragma once
#include <vector>
#include <memory>
#include "lexer.h"

namespace mcf
{
	namespace ast
	{
		enum class node_type : unsigned char
		{
			invalid = 0,

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
			virtual const std::string			convert_to_string(void) const noexcept = 0;
		};

		enum class statement_type : unsigned char
		{
			invalid = 0,

			variable,			// <data_type> identifier [optional: assign <expression>] semicolon
			variable_assign,	// identifier assign <expression> semicolon
			function,
			function_call,
			enum_def,			// keyword_enum <identifier> [optional: colon <data_type>] lbrace !<enum_block> rbrace semicolon

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
			invalid = 0,

			literal,	// integer_32bit, string_utf8
			identifier, // identifier
			data_type,	// keyword_int32, keyword_utf8, keyword_void

			// plus|minus <literal>
			// plus|minus <identifier>
			prefix,
			infix,			// <expression> plus|minus|asterisk|slash <expression>
			index_unknown,	// 평가기에서 index expression 을 평가할때 []인지 확인 하기 위해 사용되는 타입입니다.
			index,			// <expression> lbracket [optional: <expression>] rbracket
			
			// unused|in|out <data_type> <identifier>
			function_parameter,
			// [optional: unused] function_parameter_variadic
			function_parameter_variadic,
			// <function_parameter>
			// <function_parameter_variadic>
			// <function_parameter> comma <function_parameter>
			// <function_parameter_list> comma <function_parameter_variadic>
			function_parameter_list,
			// [variable] [optional: !<function_block>]
			// [variable_assign] [optional: !<function_block>]
			// [function_call] [optional: !<function_block>]
			function_block,
			function_call,

			enum_value_increment,	// 평가기에서 default enum value 를 increment 하기 위해 있는 expression 타입입니다.
			enum_block,				// identifier [optional: assign expression] [optional: comma !<enum_block>]

			// 이 밑으로는 수정하면 안됩니다.
			count,
		};

		class expression : public node
		{
		public: 
					virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept = 0;
			inline	virtual const mcf::ast::node_type		get_node_type(void) const noexcept override final { return mcf::ast::node_type::expression; }
		};

		using unique_statement = std::unique_ptr<const mcf::ast::statement>;
		using unique_expression = std::unique_ptr<const mcf::ast::expression>;
		using expression_array = std::vector<unique_expression>;
		using statement_array = std::vector<unique_statement>;

		class program final
		{
		public:
			explicit program(void) noexcept = default;
			explicit program(statement_array&& statements) noexcept;

			inline	const size_t				get_statement_count(void) const noexcept { return _statements.size(); }
					const mcf::ast::statement*	get_statement_at(const size_t index) const noexcept;

			const std::string				convert_to_string(void) const noexcept;
			const std::vector<mcf::token>	convert_to_tokens(void) const noexcept;

		private:
			statement_array	_statements;
		};

		class literal_expession final : public expression
		{
		public: 
			explicit literal_expession(void) noexcept = default;
			explicit literal_expession(const mcf::token& token) noexcept : _token(token) {}

			inline const mcf::token& get_token(void) const noexcept { return _token; }

			inline virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::literal; }
			inline virtual const std::string				convert_to_string(void) const noexcept override final { return _token.Literal; }

		private:
			const mcf::token	_token = { token_type::invalid, std::string() }; // { token_type::integer_32bit, literal }
		};

		class identifier_expression final : public expression
		{
		public:
			explicit identifier_expression(void) noexcept = default;
			explicit identifier_expression(const mcf::token& token) noexcept : _token(token) {}

			inline	virtual const mcf::ast::expression_type get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::identifier; }
			inline	virtual const std::string				convert_to_string(void) const noexcept override final { return _token.Literal; }

		private:
			const mcf::token _token = { token_type::invalid, std::string() }; // { token_type::identifier, identifier }
		};

		class data_type_expression final : public expression
		{
		public:
			explicit data_type_expression(void) noexcept = default;
			explicit data_type_expression(const bool isConst, const mcf::token& token) noexcept : _token(token), _isConst(isConst) {}

			inline const mcf::token_type get_type(void) const noexcept { return _token.Type; }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::data_type; }
			inline	virtual const std::string				convert_to_string(void) const noexcept override final { return (_isConst == true ? "const " : "") + _token.Literal; }

		private:
			mcf::token	_token		= { token_type::invalid, std::string() }; // { token_type::keyword, "int32" }
			bool		_isConst	= false; // token_type::const == true
		};

		class prefix_expression final : public expression
		{
		public:
			explicit prefix_expression(void) noexcept = default;
			explicit prefix_expression(const mcf::token& prefix, const mcf::ast::expression* targetExpression) noexcept 
				: _prefixOperator(prefix), _targetExpression(targetExpression) {}

			inline const mcf::token&			get_prefix_operator_token(void) const noexcept { return _prefixOperator; }
			inline const mcf::ast::expression*	get_target_expression(void) const noexcept { return _targetExpression.get(); }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::prefix; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			using expression = std::unique_ptr<const mcf::ast::expression>;

			const mcf::token _prefixOperator = { token_type::invalid, std::string() };	// { prefix_operator, literal }
			const expression _targetExpression;											// <expression>
		};

		class infix_expression final : public expression
		{
		public:
			explicit infix_expression(void) noexcept = default;
			explicit infix_expression(const mcf::ast::expression* left, const mcf::token& infix, const mcf::ast::expression* right) noexcept
				: _left(left), _infixOperator(infix), _right(right) {}

			inline const mcf::token&			get_infix_operator_token(void) const noexcept { return _infixOperator; }
			inline const mcf::ast::expression*	get_left_expression(void) const noexcept { return _left.get(); }
			inline const mcf::ast::expression*	get_right_expression(void) const noexcept { return _right.get(); }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::infix; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			using expression = std::unique_ptr<const mcf::ast::expression>;

			const mcf::token _infixOperator = { token_type::invalid, std::string() };	// { infix_operator, literal }
			const expression _left;														// <expression>
			const expression _right;													// <expression>
		};

		class unknown_index_expression final : public expression
		{
		public:
			explicit unknown_index_expression(void) noexcept = default;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::index_unknown; }
			inline	virtual const std::string				convert_to_string(void) const noexcept override final { return std::string(); }
		};

		class index_expression final : public expression
		{
		public:
			explicit index_expression(void) noexcept = default;
			explicit index_expression(const mcf::ast::expression* left, const mcf::ast::expression* index) noexcept
				: _left(left), _index(index) {}

			inline const mcf::ast::expression* get_left_expression(void) const noexcept { return _left.get(); }
			inline const mcf::ast::expression* get_index_expression(void) const noexcept { return _index.get(); }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::index; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			const unique_expression _left;	// <expression>
			const unique_expression _index;	// <expression>
		};

		class function_parameter_expression final : public expression
		{
		public:
			explicit function_parameter_expression(void) noexcept = default;
			explicit function_parameter_expression(const mcf::token& dataFor, const mcf::ast::expression* dataType, const mcf::ast::expression* name) noexcept;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::function_parameter; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			token				_for;
			unique_expression	_type;
			unique_expression	_name;
		};

		class function_parameter_variadic_expression final : public expression
		{
		public:
			explicit function_parameter_variadic_expression(void) noexcept = default;
			explicit function_parameter_variadic_expression(const mcf::token& dataFor) noexcept : _for(dataFor) {}

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::function_parameter_variadic; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

			token _for;
		};

		class function_parameter_list_expression final : public expression
		{
		public:
			explicit function_parameter_list_expression(void) noexcept = default;
			explicit function_parameter_list_expression(expression_array&& list) noexcept;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::function_parameter_list; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			expression_array _list;
		};
		using unique_function_parameter_list = std::unique_ptr<const mcf::ast::function_parameter_list_expression>;

		class function_block_expression final : public expression
		{
		public:
			explicit function_block_expression(void) noexcept = default;
			explicit function_block_expression(statement_array&& statements) noexcept;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::enum_block; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			statement_array _statements;
		};
		using unique_function_block = std::unique_ptr<const mcf::ast::function_block_expression>;

		using function_call_parameter_list = std::vector<std::pair<bool, unique_expression>>;
		class function_call_expression final : public expression
		{
		public:
			explicit function_call_expression(void) noexcept = default;
			explicit function_call_expression(const mcf::ast::expression* function, expression_array&& parameters) noexcept;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::function_call; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			unique_expression	_function;
			expression_array	_parameters;
		};

		class enum_value_increment final : public expression
		{
		public:
			explicit enum_value_increment(void) noexcept = default;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::enum_value_increment; }
			inline	virtual const std::string				convert_to_string(void) const noexcept override final { return std::string(); }
		};
		
		class enum_block_expression final : public expression
		{
		public:
			using unique_expression = std::unique_ptr<const mcf::ast::expression>;

		public:
			explicit enum_block_expression(void) noexcept = default;
			explicit enum_block_expression(std::vector<mcf::ast::identifier_expression >& names, expression_array&& values) noexcept;

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::enum_block; }
			virtual const std::string						convert_to_string(void) const noexcept override final;

		private:
			using name_vector		= std::vector<mcf::ast::identifier_expression>;

			name_vector			_names;
			expression_array	_values;
		};

		class macro_include_statement final : public statement
		{
		public:
			explicit macro_include_statement(void) noexcept = default;
			explicit macro_include_statement(mcf::token token) noexcept;

			inline virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::variable; }
			inline virtual const std::string				convert_to_string(void) const noexcept override final { return _token.Literal; }

		private:
			token		_token;
			std::string _path;
		};

		class variable_statement final : public statement
		{
		public:
			explicit variable_statement(void) noexcept = default;
			explicit variable_statement(const mcf::ast::data_type_expression& dataType,
										const mcf::ast::expression* name,
										const mcf::ast::expression* initExpression) noexcept;

			inline	const mcf::token_type		get_type(void) const noexcept { return _dataType.get_type(); }
					const std::string			get_name(void) const noexcept;
			inline	const mcf::ast::expression*	get_init_expression(void) const noexcept { return _initExpression.get(); }

			inline	virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::variable; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			const mcf::ast::data_type_expression				_dataType;
			const std::unique_ptr<const mcf::ast::expression>	_name;
			const std::unique_ptr<const mcf::ast::expression>	_initExpression;
		};

		class variable_assign_statement final : public statement
		{
		public:
			explicit variable_assign_statement(void) noexcept = default;
			explicit variable_assign_statement(const mcf::ast::expression* name, const mcf::ast::expression* assignExpression) noexcept;

					const std::string			get_name(void) const noexcept;
			inline const mcf::ast::expression*	get_assign_expression(void) const noexcept { return _assignedExpression.get(); }

			inline	virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::variable_assign; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			const std::unique_ptr<const mcf::ast::expression>	_name;
			const std::unique_ptr<const mcf::ast::expression>	_assignedExpression;
		};

		class function_statement final : public statement
		{
		public:
			explicit function_statement(void) noexcept = default;
			explicit function_statement(const mcf::ast::expression* returnType, 
										identifier_expression name,
										const mcf::ast::function_parameter_list_expression* parameters,
										const mcf::ast::function_block_expression* statementsBlock) noexcept;

			inline	virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::function; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			unique_expression				_returnType;
			identifier_expression			_name;
			unique_function_parameter_list	_parameters;
			unique_function_block			_statementsBlock;
		};

		class function_call_statement final : public statement
		{
		public:
			explicit function_call_statement(void) noexcept = default;
			explicit function_call_statement(const mcf::ast::function_call_expression* callExpression) noexcept : _callExpression(callExpression) {}

			inline virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::function_call; }
			inline virtual const std::string				convert_to_string(void) const noexcept override final { return _callExpression->convert_to_string() + ";"; }

		private:
			std::unique_ptr<const mcf::ast::function_call_expression>	_callExpression;
		};

		class enum_statement final : public statement
		{
		public:
			explicit enum_statement(void) noexcept = default;
			explicit enum_statement(const mcf::ast::data_type_expression& name, 
									const mcf::ast::data_type_expression& dataType,
									const mcf::ast::enum_block_expression* values) noexcept;

			inline const std::string& get_name(void) const noexcept { return _name.convert_to_string(); }

			inline	virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::enum_def; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			using block_statement = std::unique_ptr<const mcf::ast::enum_block_expression>;
			const mcf::ast::data_type_expression	_name;
			const mcf::ast::data_type_expression	_dataType;
			const block_statement					_values;
		};
	}
}
