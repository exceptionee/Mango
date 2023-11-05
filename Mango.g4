grammar Mango;

program : statement* EOF ;

statement
   :  importStatement
   | 'export'? variableStatement
   | 'export'? functionDeclaration
   | 'export'? classDeclaration
   | 'export'? interfaceDeclaration
   | 'export'? enumDeclaration
   | ifStatement
   | switchStatement
   | iterationStatement
   | throwStatement
   | catchStatement
   | continueStatement
   | breakStatement
   | returnStatement
   | expressionStatement
   | emptyStatement
   ;

block : '{' statement* '}' ;

importStatement : 'import' (('{' idList '}') | ID | '*') 'from' STRING ';' ;

idList : ID (',' ID)* ;

variableStatement : variableDeclaration ';' ;

variableDeclaration : ('var' | 'const') variableList ;

variableList : variable (',' variable)* ;

variable : ID (':' type)? initializer? ;

initializer : '=' expression ;

functionDeclaration
   : 'function' ID '(' paramaterList? ')' (':' type)? block
   ;

paramaterList
   : restParamater
   | paramater (',' paramater)* (',' restParamater)?
   ;

restParamater : '...' ID (':' type)? initializer? ;

paramater : ID '?'? (':' type)? initializer? ;

classDeclaration : 'class' ID ('extends' ID)? ('implements' idList)? '{' classElement* '}' ;

classElement
   : accessModifier? 'static'? fieldDeclaration
   | accessModifier? 'static'? methodDeclaration
   | accessModifier? 'static'? (classDeclaration | interfaceDeclaration | enumDeclaration)
   ;

accessModifier : ('public' | 'private' | 'protected') ;

fieldDeclaration : variableList ';' ;

methodDeclaration : ID '(' paramaterList? ')' (':' type)? block ;

interfaceDeclaration : 'interface' ID ('extends' idList)? '{' interfaceParamater* '}' ;

interfaceParamater : paramater ';' ;

enumDeclaration : 'enum' ID ('extends' ID)? '{' variableList? '}' ;

ifStatement : 'if' '(' expression ')' (block | statement) ('else' 'if' '(' expression ')' (block | statement))* ('else' (block | statement))? ;

switchStatement : 'switch' '(' expression ')' '{' ('case' literal ':' statement*)* ('default' ':' statement*)? '}' ;

iterationStatement
   : 'for' '(' (variableDeclaration | expression)? ';' expression? ';' expression? ')' (block | statement)
   | 'for' '(' variableDeclaration 'in' expression ')' (block | statement)
   | 'while' '(' expression ')' (block | statement)
   | 'do' (block | statement) 'while' '(' expression ')' ';'
   ;

throwStatement : 'throw' expression ';' ;

catchStatement : 'try' (block | statement) 'catch' '(' variable ')' (block | statement) ('finally' (block | statement))? ;

continueStatement : 'continue' ';' ;

breakStatement : 'break' ';' ;

returnStatement : 'return' expression? ';' ;

expressionStatement : expression ';' ;

emptyStatement : ';' ;

type
   : type ('|' | '&') type
   | '(' type ')'
   | type '[' ']' // array
   | '[' typeList? ']' // tuple
   | '(' paramaterList? ')' '->' type
   | 'any'
   | 'null'
   | 'void'
   | 'object'
   | 'int'
   | 'float'
   | 'string'
   | 'char'
   | 'bool'
   | expression '.' ID
   | ID
   ;

typeList : type (',' type)* ;

expression
   : '(' expression ')'
   | expression '.' expression // member access
   | expression '[' expression ']' // array access
   | 'new' expression '(' expressionList? ')' // object creation
   | expression '(' expressionList? ')' // function call
   | expression 'as' type // cast
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
   : (ID | '(' paramaterList? ')') '->' (expression | block) #FunctionLiteral
   | '{' variableList? '}' #ObjectLiteral
   | INTEGER #IntegerLiteral
   | FLOAT #FloatLiteral
   | STRING #StringLiteral
   | CHAR   #CharLiteral
   | BOOLEAN #BooleanLiteral
   | '[' expressionList? ']' #ArrayLiteral
   | 'null' #NullLiteral
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
