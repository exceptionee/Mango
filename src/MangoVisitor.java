// Generated from c://Users//Elias//Desktop//VSCode//Mango//src//Mango.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.tree.ParseTreeVisitor;

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link MangoParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
public interface MangoVisitor<T> extends ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link MangoParser#program}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitProgram(MangoParser.ProgramContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#statement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitStatement(MangoParser.StatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#expressionStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitExpressionStatement(MangoParser.ExpressionStatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#variableStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVariableStatement(MangoParser.VariableStatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#variableDeclaration}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVariableDeclaration(MangoParser.VariableDeclarationContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#variableList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVariableList(MangoParser.VariableListContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#variable}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVariable(MangoParser.VariableContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ArrayType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayType(MangoParser.ArrayTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code BoolType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolType(MangoParser.BoolTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code UnionType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnionType(MangoParser.UnionTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code StringType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitStringType(MangoParser.StringTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code CharType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCharType(MangoParser.CharTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code NullType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNullType(MangoParser.NullTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code AnyType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAnyType(MangoParser.AnyTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ParenType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParenType(MangoParser.ParenTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code IntType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntType(MangoParser.IntTypeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code FloatType}
	 * labeled alternative in {@link MangoParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFloatType(MangoParser.FloatTypeContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#typeList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTypeList(MangoParser.TypeListContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ArrayAccess}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayAccess(MangoParser.ArrayAccessContext ctx);
	/**
	 * Visit a parse tree produced by the {@code EqualityExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEqualityExpr(MangoParser.EqualityExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code LogicExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLogicExpr(MangoParser.LogicExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code LiteralExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLiteralExpr(MangoParser.LiteralExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code VarExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVarExpr(MangoParser.VarExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code PrimaryExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrimaryExpr(MangoParser.PrimaryExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code AssignmentExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAssignmentExpr(MangoParser.AssignmentExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ParenExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParenExpr(MangoParser.ParenExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ComparativeExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComparativeExpr(MangoParser.ComparativeExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code UnaryExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnaryExpr(MangoParser.UnaryExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code TernaryExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTernaryExpr(MangoParser.TernaryExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code SecondaryExpr}
	 * labeled alternative in {@link MangoParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSecondaryExpr(MangoParser.SecondaryExprContext ctx);
	/**
	 * Visit a parse tree produced by {@link MangoParser#expressionList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitExpressionList(MangoParser.ExpressionListContext ctx);
	/**
	 * Visit a parse tree produced by the {@code IntLiteral}
	 * labeled alternative in {@link MangoParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntLiteral(MangoParser.IntLiteralContext ctx);
	/**
	 * Visit a parse tree produced by the {@code FloatLiteral}
	 * labeled alternative in {@link MangoParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFloatLiteral(MangoParser.FloatLiteralContext ctx);
	/**
	 * Visit a parse tree produced by the {@code StringLiteral}
	 * labeled alternative in {@link MangoParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitStringLiteral(MangoParser.StringLiteralContext ctx);
	/**
	 * Visit a parse tree produced by the {@code CharLiteral}
	 * labeled alternative in {@link MangoParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCharLiteral(MangoParser.CharLiteralContext ctx);
	/**
	 * Visit a parse tree produced by the {@code BoolLiteral}
	 * labeled alternative in {@link MangoParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolLiteral(MangoParser.BoolLiteralContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ArrayLiteral}
	 * labeled alternative in {@link MangoParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayLiteral(MangoParser.ArrayLiteralContext ctx);
	/**
	 * Visit a parse tree produced by the {@code NullLiteral}
	 * labeled alternative in {@link MangoParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNullLiteral(MangoParser.NullLiteralContext ctx);
}