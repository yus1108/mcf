﻿# mcf
This is a toy compiler project named as mcf (making compiler is fun) aiming to make a compiler specialized for game development

## `Terms` :

`...` : this can be replaced in practice as a series of any symbols (tokens|expressions|statements) with at least one symbol.

`TOKEN` : the highlighted and capitalized identifier represents a token

`<Expression>` : the value between braces represents a expression

`[Statement]` : the value between brackets represents a statement

`{ ... }` : this represents that `...` is optional when parsing.

`{ ... }+` : this represents that `...` must be there at least ONE or MORE.

`{ ... }*` : this represents that `...` can be there NONE or MORE.

`[prerequisite: TOKEN]` : this represents that `TOKEN` must be required before parsing next symbol.

`[expectnext: TOKEN]` : this represents that `TOKEN` must be required for next symbol.

---

## `<Expression>` :

### `<Identifier>`

#### PARSER :
`IDENTIFIER`

---

### `<Literal>`

#### PARSER :
`LITERAL`

---

### `<String>`

#### PARSER :
`STRING`

---

### `<Prefix>`

#### PARSER :
1. `MINUS` `<Expression>`
2. `BANG` `<Expression>`
3. `AMPERSAND` `<Expression>`

---

### `<Group>`

#### PARSER :
`LPAREN` `<Expression>` `RPAREN`

---

### `<Infix>`

#### PARSER :
1. `<Expression>` `EQUAL` `<Expression>`
2. `<Expression>` `NOT_EQUAL` `<Expression>`
3. `<Expression>` `LT` `<Expression>`
4. `<Expression>` `GT` `<Expression>`
5. `<Expression>` `PLUS` `<Expression>`
6. `<Expression>` `MINUS` `<Expression>`
7. `<Expression>` `ASTERISK` `<Expression>`
8. `<Expression>` `SLASH` `<Expression>`

---

### `<Call>`

#### PARSER :
`<Expression>` `LPAREN` {`<Expression>` {`COMMA` `<Expression>`}* {`COMMA`}} `RPAREN`

---

### `<Index>`

#### PARSER :
`<Expression>` `LBRACKET` {`<Expression>`} `RBRACKET`

---

### `<As>`

#### PARSER :
`<Expression>` `KEYWORD_AS` `<TypeSignature>`

---

### `<Initializer>`

#### PARSER :
`LBRACE` `<Expression>` {`COMMA` `<Expression>`}* {`COMMA`} `RBRACE`

---

## `<INTERMEDIATE>` :

### `<Variadic>`

#### PARSER :
`VARIADIC` `<Identifier>`

---

### `<TypeSignature>`

#### PARSER :
1. {`KEYWORD_UNSIGNED`} `<Identifier>`
2. {`KEYWORD_UNSIGNED`} `<Index>`

#### EVALUATOR :

`<Identifier>`: identifier must be registered as type unless used for `[Typedef]`

`<Index>`: for this index expression, the leftmost expression must be TYPE identifier, and the index value must be integer.

---

### `<VariableSignature>`

#### PARSER :
`<Identifier>` `COLON` `<TypeSignature>`

---

### `<FunctionParams>`

#### PARSER :
1. `LPAREN` `KEYWORD_VOID` `RPAREN`
2. `LPAREN` `<Variadic>` `RPAREN`
3. `LPAREN` `<VariableSignature>` {`COMMA` `<VariableSignature>`}* {`COMMA` {`<Variadic>`}} `RPAREN`

---

### `<FunctionSignature>`

#### PARSER :
1. `KEYWORD_FUNC` `<Identifier>` `<FunctionParams>` `POINTING` `KEYWORD_VOID`
2. `KEYWORD_FUNC` `<Identifier>` `<FunctionParams>` `POINTING` `<TypeSignature>`

---

## `[Statement]` : 

### `[IncludeLibrary]`

#### PARSER :
`MACRO_INCLUDE` `LT` `KEYWORD_ASM` `COMMA` `STRING` `GT`

#### EVALUATOR:
* `KEYWORD_LIB` : this represent that this will include a library file without any header.
* `STRING` : this represent the path for the binary library file.

---

### `[Typedef]`

#### PARSER :
`KEYWORD_TYPEDEF` `<VariableSignature>` SEMICOLON`

#### EVALUATOR:
* `<VariableSignature>` : represent a new type equivalent to source type with the size specified if any and/or the values specified if any.

---

### `[Extern]`

#### PARSER :
`KEYWORD_EXTERN` `<FunctionSignature>` `SEMICOLON`

#### EVALUATOR:
* `<FunctionSignature>` : the function signature will be written as extern in the taget assembly.

---

### `[Let]`

#### PARSER :
`KEYWORD_LET` `<VariableSignature>` {`ASSIGN` `<Expression>`} `SEMICOLON`

#### EVALUATOR:
{`ASSIGN` `<Expression>`} : if this phrase exist, the variable signature will be stored as data with initialized value; otherwise, it will be stored as bss.

---

### `[Block]`

#### PARSER :
`LBRACE` {`[Statement]`}* `RBRACE`

#### EVALUATOR:
{`[Statement]`}*: scope must be pushed for these statements.

---

### `[Return]`

#### PARSER :
`KEYWORD_RETURN` `<Expression>` `SEMICOLON`

#### EVALUATOR:
`<Expression>`: this expression must have value

---

### `[Func]`

#### PARSER :
`<FunctionSignature>` `[Block]`

#### EVALUATOR:
* `<FunctionSignature>`: the signature is stored to code section.
* `<Block>`: `[Return]` is required at the end of statements if the signature has return type, and both type must be matched.

---

### `[Main]`

#### PARSER :
1. `KEYWORD_MAIN` `<FunctionParams>` `POINTING` `KEYWORD_VOID` `[Block]`
2. `KEYWORD_MAIN` `<FunctionParams>` `POINTING` `<TypeSignature>` `[Block]`

#### EVALUATOR:
* `<FunctionParams>`: params are stored to bss section.
* `<Block>`: `[Return]` is required at the end of statements if the signature has return type, and both type must be matched.

---

### `[Expression]` :

#### PARSER :
`<Expression>` `SEMICOLON`

#### EVALUATOR:
`<Expression>`: expressions like function call can be statement.

---

### `[Unused]` :

#### PARSER :
`KEYWORD_UNUSED` `LPAREN` {`<Identifier>` {`COMMA` `<Identifier>`}* {`COMMA`}} `RPAREN`

#### EVALUATOR:
{`<Identifier>` {`COMMA` `<Identifier>`}* {`COMMA`}}: variables passed will be checked as used.

---
