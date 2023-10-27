grammar Mango;

program : statement* EOF ;

statement
   :  importStatement
   | 'export'? variableStatement
   | 'export'? functionDeclaration
   | 'export'? classDeclaration
   | 'export'? interfaceDeclaration
   | 'export'? enumDeclaration
   | expressionStatement
   | ifStatement
   | iterationStatement
   | throwStatement
   | catchStatement
   | continueStatement
   | breakStatement
   | returnStatement
   ;

block : '{' statement* '}' ;

importStatement
   : 'import' (ID | '*') 'from' ID ';'
   | 'import' '{' idList '}' 'from' ID ';'
   ;

idList : ID (',' ID)* ;

variableStatement : variableDeclaration ';' ;

variableDeclaration : ('var' | 'const') variableList ;

variableList : variable (',' variable)* ;

variable : ID (':' type)? initializer? ;

type
   : '(' paramaterList? ')' '->' type
   | '(' type ')'
   | type '[' ']' // array
   | '[' typeList? ']' // tuple
   | type ('|' | '&') type
   | 'null'
   | 'void'
   | 'any'
   | 'function'
   | 'int'
   | 'float'
   | 'string'
   | 'char'
   | 'bool'
   | ID
   | expression '.' ID
   ;

typeList : type (',' type)* ;

initializer : '=' expression ;

functionDeclaration
   : 'function' ID '(' paramaterList? ')' (':' type)? block
   ;

paramaterList
   : restParamater
   | paramater (',' paramater)* (',' restParamater)?
   ;

paramater : ID '?'? (':' type)? initializer? ;

restParamater : '...' ID (':' type)? initializer? ;

classDeclaration : 'class' ID ('extends' ID)? ('implements' idList)? '{' classElement* '}' ;

classElement
   : accessModifier? 'static'? fieldDeclaration
   | accessModifier? 'static'? methodDeclaration
   | accessModifier? 'static'? classDeclaration
   ;

accessModifier : ('public' | 'private' | 'protected') ;

fieldDeclaration : variableList ';' ;

methodDeclaration : ID '(' paramaterList? ')' (':' type)? block ;

interfaceDeclaration : 'interface' ID ('extends' ID)? '{' interfaceParamater* '}' ;

interfaceParamater : paramater ';' ;

enumDeclaration : 'enum' ID ('extends' ID)? '{' variableList ';' '}' ;

expressionStatement : expression ';' ;

ifStatement : 'if' '(' expression ')' block ('else' 'if' '(' expression ')' block)* ('else' block)? ;

iterationStatement
   : 'for' '(' (variableDeclaration | expression)? ';' expression? ';' expression? ')' block
   | 'for' '(' variableDeclaration 'in' expression ')' block
   | 'while' '(' expression ')' block
   | 'do' block 'while' '(' expression ')' ';'
   ;

throwStatement : 'throw' expression ';' ;

catchStatement : 'try' block 'catch' '(' variable ')' block ('finally' block)? ;

continueStatement : 'continue' ';' ;

breakStatement : 'break' ';' ;

returnStatement : 'return' expression? ';' ;

expression
   : (ID | '(' paramaterList? ')') '->' (expression | block)
   | '(' expression ')'
   | expression '.' expression // member access
   | expression '[' expression ']' // array access
   | 'new' expression '(' expressionList? ')' // object creation
   | expression '(' expressionList? ')' // function call
   | literal
   | ID
   | expression ('++' | '--') // postfix
   | ('+' | '-' | '++' | '--' | '!') expression // unary
   | expression ('*' | '/' | '%') expression
   | expression ('+' | '-') expression
   | expression ('>' | '<' | '>=' | '<=') expression
   | expression ('==' | '!=') expression
   | expression ('&&' | '||' | '&' | '|') expression
   | expression '?' expression ':' expression
   | expression ('=' | '*=' | '/=' | '%=' | '+=' | '-=') expression
   ;

expressionList : expression (',' expression)* ;

literal
   : INTEGER
   | FLOAT
   | STRING
   | CHAR
   | BOOLEAN
   | '[' expressionList? ']'
   | 'null'
   ;

WS : [ \t\n\r]+ -> skip ;
COMMENT : '/*' .*? '*/' -> skip ;
LINE_COMMENT : '//' .*? ('\r' | '\n' | '\r\n' | EOF) -> skip ;

INTEGER : DIGIT+ ;
FLOAT : INTEGER? '.' DIGIT* ;
STRING : '"' ( ESC | '\\"' | ~[\\\r\n] )*? '"' ;
CHAR : '\'' ( ESC | '\\\'' | ~[\\\r\n] ) '\'' ;
BOOLEAN : 'true' | 'false' ;

ID : [_a-zA-Z][_a-zA-Z0-9]* ;

fragment DIGIT : [0-9] ;
fragment ESC : '\\' [btnr\\] ;
