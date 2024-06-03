import java.util.ArrayDeque;
import java.util.HashMap;

public class TypeChecker extends MangoBaseVisitor<Type> {
  ArrayDeque<HashMap<String, Symbol>> stack = new ArrayDeque<>();

  TypeChecker() {
    // adds global frame
    stack.add(new HashMap<>());
  }

  @Override
  public Type visitProgram(MangoParser.ProgramContext ctx) {
    if (ctx.children.size() == 2)
      return visit(ctx.children.getFirst());

    return super.visitProgram(ctx);
  }

  @Override
  public Type visitVariable(MangoParser.VariableContext ctx) {
    String name = ctx.ID().getText();

    if (stack.peek().containsKey(name)) {
      new TypeError("'" + name + "' has already been declared", 
        new Source(Runner.file, ctx.ID().getSymbol().getLine(), ctx.ID().getSymbol().getCharPositionInLine() + 1));
      return Type.ERROR;
    }

    Type valueType = (ctx.value != null)? visit(ctx.value) : Type.Primitive.NULL;
    Type type = (ctx.type() != null)? visit(ctx.type()) :
      (ctx.value != null)? valueType : Type.ANY;

    if (type != Type.ERROR && !type.superset(valueType)) {
      type = Type.ERROR;
      new TypeError("cannot assign type '" + valueType + "' to type '" + type + "'", 
        new Source(Runner.file, ctx.ID().getSymbol().getLine(), ctx.ID().getSymbol().getCharPositionInLine() + 1));
    }

    MangoParser.VariableStatementContext s = (MangoParser.VariableStatementContext) ctx.parent.parent;
    stack.peek().put(
      ctx.name.getText(),
      s.modifier.getType() == 1? new Symbol.Var(type) : new Symbol.Const(type)
    );

    return type;
  }

  @Override
  public Type visitNullableType(MangoParser.NullableTypeContext ctx) {
    Type type = visit(ctx.type());

    if (type == Type.ERROR)
      return Type.ERROR;
    else if (type == Type.ANY)
      return Type.ANY;
    else if (type == Type.Primitive.NULL)
      return Type.Primitive.NULL;
    else
      return new Type.Union(type, Type.Primitive.NULL);
  }

  @Override
  public Type visitUnionType(MangoParser.UnionTypeContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    if (left == Type.ANY || right == Type.ANY)
      return Type.ANY;
    else if (!left.equals(right))
      return new Type.Union(left, right);

    return left;
  }

  @Override
  public Type visitParenType(MangoParser.ParenTypeContext ctx) {
    return visit(ctx.type());
  }

  @Override
  public Type visitArrayType(MangoParser.ArrayTypeContext ctx) {
    return new Type.Array(visit(ctx.type()));
  }

  @Override
  public Type visitAnyType(MangoParser.AnyTypeContext ctx) {
    return Type.ANY;
  }

  @Override
  public Type visitIntType(MangoParser.IntTypeContext ctx) {
    return Type.Primitive.INT;
  }

  @Override
  public Type visitFloatType(MangoParser.FloatTypeContext ctx) {
    return Type.Primitive.FLOAT;
  }

  @Override
  public Type visitStringType(MangoParser.StringTypeContext ctx) {
    return Type.Primitive.STRING;
  }

  @Override
  public Type visitCharType(MangoParser.CharTypeContext ctx) {
    return Type.Primitive.CHAR;
  }

  @Override
  public Type visitBoolType(MangoParser.BoolTypeContext ctx) {
    return Type.Primitive.BOOL;
  }

  @Override
  public Type visitNullType(MangoParser.NullTypeContext ctx) {
    return Type.Primitive.NULL;
  }

  @Override
  public Type visitExpressionStatement(MangoParser.ExpressionStatementContext ctx) {
    return visit(ctx.children.getFirst());
  }

  @Override
  public Type visitParenExpr(MangoParser.ParenExprContext ctx) {
    return visit(ctx.expr);
  }

  @Override
  public Type visitArrayAccess(MangoParser.ArrayAccessContext ctx) {
    Type left = visit(ctx.array);

    if (left instanceof Type.Array)
      return ((Type.Array) left).type();

    new TypeError("cannot perform an array access on type '" + left + "'",
      new Source(Runner.file, ctx.array.start.getLine(), ctx.array.start.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitVarExpr(MangoParser.VarExprContext ctx) {
    for (HashMap<String, Symbol> frame : stack)
      if (frame.containsKey(ctx.ID().getText()))
        return frame.get(ctx.ID().getText()).type();

    new ReferenceError("'" + ctx.ID().getText() + "' is not declared",
      new Source(Runner.file, ctx.start.getLine(), ctx.start.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitPostFixExpr(MangoParser.PostFixExprContext ctx) {
    Type type = visit(ctx.expr);

    if (type == Type.ERROR)
      return Type.ERROR;

    if (!(ctx.expr instanceof MangoParser.VarExprContext) && !(ctx.expr instanceof MangoParser.ArrayAccessContext)) {
      new SyntaxError("invalid left hand side for postfix operation '" + ctx.op.getText() + "'",
        new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
      return Type.ERROR;
    }
    else if (stack.peek().get(ctx.expr.stop.getText()) instanceof Symbol.Const) {
      new TypeError("cannot reassign constant '" + ctx.expr.stop.getText() + "'",
        new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
      return Type.ERROR;
    }
    else if (!Type.NUMBER.superset(type)) {
      new TypeError("cannot perform operation '" + ctx.op.getText() + "' on type '" + type + "'",
        new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
      return Type.ERROR;
    }

    return type;
  }

  @Override
  public Type visitUnaryExpr(MangoParser.UnaryExprContext ctx) {
    Type type = visit(ctx.expr);

    if (type == Type.ERROR)
      return Type.ERROR;

    switch (ctx.op.getType()) {
      case 20:
      case 21:
        if (!(ctx.expr instanceof MangoParser.VarExprContext) && !(ctx.expr instanceof MangoParser.ArrayAccessContext)) {
          new SyntaxError("invalid right hand side for unary operation '" + ctx.op.getText() + "'",
            new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
          return Type.ERROR;
        }
        else if (stack.peek().get(ctx.expr.stop.getText()) instanceof Symbol.Const) {
          new TypeError("cannot reassign constant '" + ctx.expr.stop.getText() + "'",
            new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
          return Type.ERROR;
        }
        else if (!Type.NUMBER.superset(type)) {
          new TypeError("cannot perform operation '" + ctx.op.getText() + "' on type '" + type + "'",
            new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
          return Type.ERROR;
        }

        return type;

      case 22:
      case 23:
        if (Type.NUMBER.superset(type))
          return type;

        new TypeError("cannot perform operation '" + ctx.op.getText() + "' on type '" + type + "'",
          new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
        return Type.ERROR;
    
      default:
        if (type == Type.Primitive.BOOL)
          return Type.Primitive.BOOL;

        new TypeError("cannot perform operation '!' on type '" + type + "'",
          new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
        return Type.ERROR;
    }
  }

  @Override
  public Type visitSecondaryExpr(MangoParser.SecondaryExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    else if (left == Type.Primitive.INT && right == Type.Primitive.INT)
      return Type.Primitive.INT;
    else if (Type.NUMBER.superset(left) && Type.NUMBER.superset(right))
      return Type.Primitive.FLOAT;

    new TypeError("cannot perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitPrimaryExpr(MangoParser.PrimaryExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    else if (left == Type.Primitive.INT && right == Type.Primitive.INT)
      return Type.Primitive.INT;
    else if (Type.NUMBER.superset(left) && Type.NUMBER.superset(right))
      return Type.Primitive.FLOAT;
    else if (ctx.op.getType() == 22 && left == Type.Primitive.STRING && right == Type.Primitive.STRING)
      return Type.Primitive.STRING;

    new TypeError("cannot perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitComparativeExpr(MangoParser.ComparativeExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    else if (Type.NUMBER.superset(left) && Type.NUMBER.superset(right))
      return Type.Primitive.BOOL;
    
    new TypeError("cannot perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitEqualityExpr(MangoParser.EqualityExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    else if (left.equals(right))
      return Type.Primitive.BOOL;
    
    new TypeError("cannot perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitLogicExpr(MangoParser.LogicExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);
    
    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    else if (left == Type.Primitive.BOOL && right == Type.Primitive.BOOL)
      return Type.Primitive.BOOL;
    
    new TypeError("cannot perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitNullishExpr(MangoParser.NullishExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    else if (left == Type.ANY || right == Type.ANY)
      return Type.ANY;
    else if (left.equals(right))
      return left;
    else
      return new Type.Union(left, right);
  }

  @Override
  public Type visitCastExpr(MangoParser.CastExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;

    return right;
  }

  @Override
  public Type visitTernaryExpr(MangoParser.TernaryExprContext ctx) {
    Type condition = visit(ctx.condition);
    Type value = visit(ctx.value);
    Type default_ = visit(ctx.default_);

    if (condition == Type.ERROR || value == Type.ERROR || default_ == Type.ERROR)
      return Type.ERROR;
    else if (condition == Type.Primitive.BOOL)
      return value == default_? value : new Type.Union(value, default_);

    new TypeError("cannot convert type '" + condition + "' to type 'bool'",
      new Source(Runner.file, ctx.condition.start.getLine(), ctx.condition.start.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitAssignmentExpr(MangoParser.AssignmentExprContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left == Type.ERROR || right == Type.ERROR)
      return Type.ERROR;
    else if (!(ctx.left instanceof MangoParser.VarExprContext) && !(ctx.left instanceof MangoParser.ArrayAccessContext)) {
      new SyntaxError("invalid left hand side for assignment operator '" + ctx.op.getText() + "'",
        new Source(Runner.file, ctx.left.start.getLine(), ctx.left.start.getCharPositionInLine() + 1));
      return Type.ERROR;
    }
    else if (stack.peek().get(ctx.left.stop.getText()) instanceof Symbol.Const) {
      new TypeError("cannot reassign constant '" + ctx.left.stop.getText() + "'",
        new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
      return Type.ERROR;
    }
    else if (ctx.op.getType() != 6 && !Type.NUMBER.superset(left)) {
      new TypeError("cannot perform arithmetic assignment '" + ctx.op.getText() + "' on type '" + left + "'", 
        new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
      return Type.ERROR;
    }
    else if (!left.superset(right)) {
      new TypeError("cannot perform assignment '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'", 
        new Source(Runner.file, ctx.right.start.getLine(), ctx.right.start.getCharPositionInLine() + 1));
      return Type.ERROR;
    }

    return right;
  }

  @Override
  public Type visitIntLiteral(MangoParser.IntLiteralContext ctx) {
    return Type.Primitive.INT;
  }

  @Override
  public Type visitFloatLiteral(MangoParser.FloatLiteralContext ctx) {
    return Type.Primitive.FLOAT;
  }

  @Override
  public Type visitStringLiteral(MangoParser.StringLiteralContext ctx) {
    return Type.Primitive.STRING;
  }

  @Override
  public Type visitCharLiteral(MangoParser.CharLiteralContext ctx) {
    return Type.Primitive.CHAR;
  }

  @Override
  public Type visitBoolLiteral(MangoParser.BoolLiteralContext ctx) {
    return Type.Primitive.BOOL;
  }

	@Override
  public Type visitArrayLiteral(MangoParser.ArrayLiteralContext ctx) {
    if (ctx.array == null)
      return new Type.Array(Type.Primitive.NULL);

    Type.Union union = new Type.Union();

    for (int i = 0; i < ctx.array.children.size(); i += 2)
      union.add(visit(ctx.array.children.get(i)));

    if (union.contains(Type.ERROR))
      return Type.ERROR;
    else if (union.contains(Type.ANY))
      return Type.ANY;
    else if (union.size() == 1)
      return new Type.Array(union.getFirst());
    else
      return new Type.Array(union);
  }

  @Override
  public Type visitNullLiteral(MangoParser.NullLiteralContext ctx) {
    return Type.Primitive.NULL;
  }
}