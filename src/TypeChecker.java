import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import org.antlr.v4.runtime.tree.ParseTree;

public class TypeChecker extends MangoBaseVisitor<Type> {
  HashMap<String, Type> heap = new HashMap<String, Type>();

  /* TODO */
  @Override
  public Type visitUnionType(MangoParser.UnionTypeContext ctx) {
    Type left = visit(ctx.left);
    Type right = visit(ctx.right);

    if (left != right)
      return new Type.Union(Arrays.asList(left, right));

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
  public Type visitVariable(MangoParser.VariableContext ctx) {
    Type type = (ctx.type() != null)? visit(ctx.type()) : null;
    Type valueType = (ctx.value != null)? visit(ctx.value) : null;

    if (type == Type.ERROR || valueType == Type.ERROR)
      return null;
    else if (type == null && valueType == null)
      heap.put(ctx.name.getText(), Type.ANY);
    else if (type == null)
      heap.put(ctx.name.getText(), valueType);
    else if (valueType == null)
      heap.put(ctx.name.getText(), type);
    else {
      if (type.superset(valueType))
        heap.put(ctx.name.getText(), type);
      else
        new TypeError("can assign type '" + valueType + "' to type '" + type + "'", 
          new Source(Runner.getFileName(), 0, 0));
    }

    return null;
  }

  @Override
  public Type visitParenExpr(MangoParser.ParenExprContext ctx) {
    return visit(ctx.expr);
  }

  @Override
  public Type visitArrayAccess(MangoParser.ArrayAccessContext ctx) {
    return ((Type.Array) visit(ctx.array)).type();
  }

  @Override
  public Type visitVarExpr(MangoParser.VarExprContext ctx) {
    return heap.get(ctx.ID().getText());
  }

  @Override
  public Type visitUnaryExpr(MangoParser.UnaryExprContext ctx) {
    if (ctx.op.getType() == 19 || ctx.op.getType() == 20) {
      Type type = visit(ctx.expr);

      if (type == Type.Primitive.INT)
        return Type.Primitive.INT;
      else if (Type.NUMBER.superset(type))
        return type;

      new TypeError("can not perform operation '" + ctx.op.getText() + "' on type '" + type + "'",
        new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
      return Type.ERROR;
    }

    return Type.NUMBER;
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

    new TypeError("can not perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
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
    else if (ctx.op.getType() == 19 && left == Type.Primitive.STRING && right == Type.Primitive.STRING)
      return Type.Primitive.STRING;

    new TypeError("can not perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
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
    
    new TypeError("can not perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
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
    
    new TypeError("can not perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
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
    
    new TypeError("can not perform operation '" + ctx.op.getText() + "' on types '" + left + "' and '" + right + "'",
      new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
    return Type.ERROR;
  }

  @Override
  public Type visitTernaryExpr(MangoParser.TernaryExprContext ctx) {
    Type condition = visit(ctx.condition);
    Type value = visit(ctx.value);
    Type default_ = visit(ctx.default_);

    if (condition == Type.ERROR || value == Type.ERROR || default_ == Type.ERROR)
      return Type.ERROR;
    else if (condition == Type.Primitive.BOOL) {
      if (value == default_)
        return value;

      return new Type.Union(Arrays.asList(value, default_));
    }

    new TypeError("can not convert type '" + condition + "' to type 'bool'",
      new Source(Runner.getFileName(), ctx.condition.start.getLine(), ctx.condition.start.getCharPositionInLine() + 1));
    return Type.ERROR;
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
    if (ctx.expressionList() == null)
      return new Type.Array(Type.Primitive.NULL);

    ArrayList<Type> list = new ArrayList<Type>();

    List<ParseTree> children = ctx.expressionList().children;
    for (int i = 1; i < children.size(); i += 2) {
      list.add(visit(children.get(i)));
    }

    return new Type.Array(new Type.Union(list));
  }

  @Override
  public Type visitNullLiteral(MangoParser.NullLiteralContext ctx) {
    return Type.Primitive.NULL;
  }
}