# mcf
This is a toy compiler project named as mcf (making compiler is fun) aiming to make a compiler specialized for game development

## Terms

`TOKEN`: the value between '\`' represents a token

`\<Expression>`: the value between braces represents a expression

`[Statement]`: the value between brackets represents a statement

{ ... }: "..." can be token/expression/statement, and this represents that these "..." can be OUPTED OUT when parsing

{ ... }+: "..." can be token/expression/statement, and this represents that these "..." must be there at least ONE or MORE.

{ ... }*: "..." can be token/expression/statement, and this represents that these "..." can be there NONE or MORE.

[prerequisite: ...]: "..." can be token/expression/statement, and this represents that these "..." must be required before parsing next symbol

[expectnext: ...]: "..." can be token/expression/statement, and this represents that this "..." must be required for next symbol

Any number after symbol name is to differentiate the same symbol with different evaluator explanations.

## \<Expression>

### `\<Type>`

PARSER:
1. `IDENTIFIER`
2. `\<Index>`

EVALUATOR:
`IDENTIFIER`: identifier must be registered as type unless used for `[Typedef]`
`\<Index>`: for this index expression, the leftmost expression must be TYPE identifier, and the index value must be either the integer or opted out.

RETURN_TYPE: TYPE_INFO

### `\<Index>`

PARSER:
`\<Expression>` {`LBRACKET` {`\<Expression>`} `RBRACKET`}+

RETURN_TYPE: TYPE_INFO


## \<NonGenericExpression>

### `\<MapInitializer>`

PARSER:
`LBRACE` {`\<Expression>` `ASSIGN` `\<Expression>` `COMMA`}+ `RBRACE`

### `\<VariableSignature>`

PARSER:
`IDENTIFIER` `COLON` `\<Type>`

### `\<FunctionSignature>`

PARSER:
[prerequisite: `KEYWORD_FUNC`] `IDENTIFIER` `\<FunctionParams>` `POINTING` `KEYWORD_VOID`

### `\<FunctionParams>`

PARSER:
1. `LPAREN` `KEYWORD_VOID` `RPAREN`
2. `LPAREN` `VARIADIC` `RPAREN`
3. `LPAREN` `{`\<VariableSignature>` {`COMMA`}}+ {[prerequisite: `COMMA`] `VARIADIC`} `RPAREN`

### `\<Statements>`

PARSER:
[prerequisite: `LBRACE`] {`[Statement]`}* [expectnext: `RBRACE`]


## [Statement]

### `[IncludeLibrary]`

PARSER:
`MACRO_INCLUDE` `LT` `KEYWORD_ASM` `COMMA` `STRING` `GT`

EVALUATOR:
`KEYWORD_LIB`: this represent that this will include a library file without any header
`STRING`: this represent the path for the binary library file

### `[Typedef]`

PARSER:
`KEYWORD_TYPEDEF` `\<VariableSignature>` {`POINTING` `KEYWORD_BIND` `\<MapInitializer>`} `SEMICOLON`

EVALUATOR:
`IDENTIFIER`: represent a new type equivalent to source type with the size specified if any and/or the values specified if any.
`\<Type>`: represent a existing source type with a specified size of source type if any.
{`POINTING` `KEYWORD_BIND` `\<MapInitializer>`}: represent values specified if any. If not, it will inherit values from the source type.
`\<MapInitializer>`: for this expression, a key for each item must be identifier.

### `[Extern]`

PARSER:
`KEYWORD_EXTERN` `KEYWORD_ASM` `KEYWORD_FUNC` `\<FunctionSignature>` `SEMICOLON`

EVALUATOR:
`KEYWORD_ASM`: when calling this function, it will pass calling without function description.
`\<FunctionSignature>`: the function signature will be written as extern in the taget assembly.

### `[Let]`

PARSER:
`KEYWORD_LET` `\<VariableSignature>` {`ASSIGN` `\<Expression>`} `SEMICOLON`

EVALUATOR:
{`ASSIGN` `\<Expression>`}: if this phrase exist, the variable signature will be stored as data with initialized value; otherwise, it will be stored as bss.


### `[Func]`

PARSER:
`KEYWORD_FUNC` `\<FunctionSignature>` `LBRACE` `\<Statements>` `RBRACE`

EVALUATOR:
`\<FunctionSignature>`: the signature is stored to code section.
`\<Statements>`: `[Return]` is required at the end of statements if the signature has return type, and both type must be matched.

### `[Return]`

PARSER:
`KEYWORD_RETURN` `\<Expression>` `SEMICOLON`

EVALUATOR:
`\<Expression>`: this expression must have value