import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import org.antlr.v4.runtime.tree.ParseTree;

class Interpreter extends MangoBaseVisitor<Object> {
  ArrayDeque<HashMap<String, Object>> stack = new ArrayDeque<>();

  Interpreter() {
    // adds global frame
    stack.add(new HashMap<>());
  }

  @Override
  public Object visitProgram(MangoParser.ProgramContext ctx) {
    if (ctx.children.size() == 2)
      return visit(ctx.children.getFirst());

    return super.visitProgram(ctx);
  }

  @Override
  public Object visitExpressionStatement(MangoParser.ExpressionStatementContext ctx) {
    return visit(ctx.children.getFirst());
  }

  @Override
  public Void visitVariable(MangoParser.VariableContext ctx) {
    stack.peek().put(ctx.name.getText(), ctx.value != null? visit(ctx.value) : null);
    return null;
  }

  @Override
  public Object visitNullableType(MangoParser.NullableTypeContext ctx) {
    return visit(ctx.type());
  }

  @Override
  public Object visitArrayType(MangoParser.ArrayTypeContext ctx) {
    return visit(ctx.type());
  }

  @Override
  public Object visitUnionType(MangoParser.UnionTypeContext ctx) {
    Class<?> left = (Class<?>)visit(ctx.left);
    Class<?> right = (Class<?>)visit(ctx.right);

    if (left == Object.class || right == Object.class)
      return Object.class;
    else if (left != right)
      return new HashSet<>(Arrays.asList(left, right));

    return left;
  }

  @Override
  public Object visitAnyType(MangoParser.AnyTypeContext ctx) {
    return Object.class;
  }

  @Override
  public Object visitIntType(MangoParser.IntTypeContext ctx) {
    return Long.class;
  }

  @Override
  public Object visitFloatType(MangoParser.FloatTypeContext ctx) {
    return Double.class;
  }

  @Override
  public Object visitStringType(MangoParser.StringTypeContext ctx) {
    return String.class;
  }

  @Override
  public Object visitCharType(MangoParser.CharTypeContext ctx) {
    return Character.class;
  }

  @Override
  public Object visitBoolType(MangoParser.BoolTypeContext ctx) {
    return Boolean.class;
  }
  
  @Override
  public Object visitParenExpr(MangoParser.ParenExprContext ctx) {
    return visit(ctx.expr);
  }

  @Override
  public Object visitArrayAccess(MangoParser.ArrayAccessContext ctx) {
    ArrayList<?> array = (ArrayList<?>) visit(ctx.array);
    long index = (long) visit(ctx.index);

    if (index < 0 || array.size() <= index) {
      new RuntimeError("index " + index + " out of range for length " + array.size(),
        new Source(Runner.file, ctx.index.start.getLine(), ctx.index.start.getCharPositionInLine() + 1));
    }

    return array.get((int) index);
  }

  @Override
  public Object visitVarExpr(MangoParser.VarExprContext ctx) {
    for (HashMap<String, Object> frame : stack)
      if (frame.containsKey(ctx.ID().getText()))
        return frame.get(ctx.ID().getText());

    return null;
  }

  @Override
  public Object visitPostFixExpr(MangoParser.PostFixExprContext ctx) {
    Object temp = visit(ctx.expr);

    if (ctx.op.getType() == 20) {
      if (temp instanceof Long)
        assign(ctx.expr, (Long) temp + 1);
      else
        assign(ctx.expr, (Double) temp + 1);
    }
    else {
      if (temp instanceof Long)
        assign(ctx.expr, (Long) temp - 1);
      else
        assign(ctx.expr, (Double) temp - 1);
    }

    return temp;
  }

  @Override
  public Object visitUnaryExpr(MangoParser.UnaryExprContext ctx) {
    Object value = visit(ctx.expr);

    switch (ctx.op.getType()) {
      case 20:
        if (value instanceof Long)
          return assign(ctx.expr, ((Long) value) + 1);
        else
          return assign(ctx.expr, ((Double) value) + 1);

      case 21:
        if (value instanceof Long)
          return assign(ctx.expr, ((Long) value) - 1);
        else
          return assign(ctx.expr, ((Double) value) - 1);

      case 22:
        if (value instanceof Long)
          return +((Long) value);

        return +((Double) value);

      case 23:
        if (value instanceof Long)
          return -((Long) value);

        return -((Double) value);

      default:
        return !((boolean) value);
    }
  }

  @Override
  public Number visitSecondaryExpr(MangoParser.SecondaryExprContext ctx) {
    Number left = (Number) visit(ctx.left);
    Number right = (Number) visit(ctx.right);

    if (left instanceof Long && right instanceof Long) {
      switch (ctx.op.getType()) {
        case 25:
          return (long) left * (long) right;
        case 26:
          if ((long) right == 0)
            new RuntimeError("can not divide by 0",
              new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
          return (long) left / (long) right;
        default:
          return (long) left % (long) right;
      }
    }

    switch (ctx.op.getType()) {
      case 24:
        return left.doubleValue() * right.doubleValue();
      case 25:
        if (right.doubleValue() == 0)
          new RuntimeError("can not divide by 0",
            new Source(Runner.file, ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
        return left.doubleValue() / right.doubleValue();
      default:
        return left.doubleValue() % right.doubleValue();
    }
  }

  @Override
  public Object visitPrimaryExpr(MangoParser.PrimaryExprContext ctx) {
    Object left = visit(ctx.left);
    Object right = visit(ctx.right);

    if (ctx.op.getType() == 22) {
      if (left instanceof String)
        return ((String) left) + ((String) right);
      else if (left instanceof Long && right instanceof Long)
        return ((long) left) + ((long) right);

      return ((Number) left).doubleValue() + ((Number) right).doubleValue();
    }

    if (left instanceof Long && right instanceof Long)
      return ((long) left) - ((long) right);

    return ((Number) left).doubleValue() - ((Number) right).doubleValue();
  }

  @Override
  public Object visitComparativeExpr(MangoParser.ComparativeExprContext ctx) {
    Number left = (Number) visit(ctx.left);
    Number right = (Number) visit(ctx.right);

    switch (ctx.op.getType()) {
      case 28:
        return left.doubleValue() > right.doubleValue();
      
      case 29:
        return left.doubleValue() < right.doubleValue();

      case 30:
        return left.doubleValue() >= right.doubleValue();
      
      default:
        return left.doubleValue() <= right.doubleValue();
    }
  }

  @Override
  public Object visitEqualityExpr(MangoParser.EqualityExprContext ctx) {
    Object left = visit(ctx.left);
    Object right = visit(ctx.right);

    if (ctx.op.getType() == 32) {
      if (left instanceof Number && right instanceof Number)
        return ((Number) left).doubleValue() == ((Number) right).doubleValue();
      else if (left instanceof String && right instanceof String)
        return left.equals(right);
      else if (left instanceof ArrayList && right instanceof ArrayList)
        return left.equals(right);

      return left == right;
    }

    if (left instanceof Number && right instanceof Number)
      return ((Number) left).doubleValue() != ((Number) right).doubleValue();
    else if (left instanceof String && right instanceof String)
      return !left.equals(right);
    else if (left instanceof ArrayList && right instanceof ArrayList)
      return !left.equals(right);
    
    return left != right;
  }

  @Override
  public Object visitLogicExpr(MangoParser.LogicExprContext ctx) {
    if (ctx.op.getType() == 34)
      return (boolean) visit(ctx.left) && (boolean) visit(ctx.right);

    return (boolean) visit(ctx.left) || (boolean) visit(ctx.right);
  }

  @Override
  public Object visitNullishExpr(MangoParser.NullishExprContext ctx) {
    Object left = visit(ctx.left);

    return left != null? left : visit(ctx.right);
  }

  @Override
  public Object visitCastExpr(MangoParser.CastExprContext ctx) {
    Object left = visit(ctx.left);
    Object type = visit(ctx.right);

    if (ctx.right instanceof MangoParser.NullTypeContext && left == null)
      return null;
    else if (ctx.right instanceof MangoParser.NullableTypeContext) {
      if (left == null || validCast(left, type)) return left;
    }
    else if (validCast(left, type))
      return left;

    return new RuntimeError("cannot cast value '" + left + "' to type '" + ctx.right.getText() + "'",
      new Source(Runner.file, ctx.start.getLine(), ctx.start.getCharPositionInLine() + 1));
  }

  @Override
  public Object visitTernaryExpr(MangoParser.TernaryExprContext ctx) {
    return (boolean) visit(ctx.condition)? visit(ctx.value) : visit(ctx.default_);
  }

  @Override
  public Object visitAssignmentExpr(MangoParser.AssignmentExprContext ctx) {
    if (ctx.op.getType() == 6)
      return assign(ctx.left, visit(ctx.right));

    Object left = visit(ctx.left);
    Object right = visit(ctx.right);

    switch (ctx.op.getType()) {
      case 38:
        if (left instanceof Long)
          return assign(ctx.left, (Long) left + (Long) right);
        
        return assign(ctx.left, (Double) left + (Double) right);

      case 39:
        if (left instanceof Long)
          return assign(ctx.left, (Long) left - (Long) right);
        
        return assign(ctx.left, (Double) left - (Double) right);
      
      case 40:
        if (left instanceof Long)
          return assign(ctx.left, (Long) left * (Long) right);
        
        return assign(ctx.left, (Double) left * (Double) right);

      case 41:
        if (left instanceof Long)
          return assign(ctx.left, (Long) left / (Long) right);
        
        return assign(ctx.left, (Double) left / (Double) right);

      default:
        if (left instanceof Long)
          return assign(ctx.left, (Long) left % (Long) right);
        
        return assign(ctx.left, (Double) left % (Double) right);
    }
  }

  @Override
  public Object visitIntLiteral(MangoParser.IntLiteralContext ctx) {
    try {
      return Long.parseLong(ctx.start.getText());
    } catch (NumberFormatException e) {
      return new RuntimeError("number " + ctx.start.getText() + " out of range for type int",
        new Source(Runner.file, ctx.start.getLine(), ctx.start.getCharPositionInLine() + 1));
    }
  }

  @Override
  public Object visitFloatLiteral(MangoParser.FloatLiteralContext ctx) {
    return Double.parseDouble(ctx.start.getText());
  }

  @Override
  public String visitStringLiteral(MangoParser.StringLiteralContext ctx) {
    return ctx.start.getText().translateEscapes();
  }

  @Override
  public Object visitCharLiteral(MangoParser.CharLiteralContext ctx) {
    String text = ctx.start.getText();

    if (text.length() == 1)
      return text.charAt(0);

    switch (text) {
      case "\\b":
        return '\b';

      case "\\t":
        return '\t';

      case "\\n":
        return '\n';
        
      case "\\\\":
        return '\\';

      default:
        return '\'';
    }
  }

  @Override
  public Boolean visitBoolLiteral(MangoParser.BoolLiteralContext ctx) {
    return ctx.start.getType() == 43? true : false;
  }

	@Override
  public Object visitArrayLiteral(MangoParser.ArrayLiteralContext ctx) {
    ArrayList<Object> list = new ArrayList<Object>();

    if (ctx.array == null)
      return list;

    List<ParseTree> children = ctx.array.children;
    for (int i = 0; i < children.size(); i += 2)
      list.add(visit(children.get(i)));

    return list;
  }

  @SuppressWarnings("unchecked")
  private Object assign(MangoParser.ExpressionContext ctx, Object value) {
    if (ctx instanceof MangoParser.VarExprContext) {
      String name = ((MangoParser.VarExprContext) ctx).ID().getText();

      for (HashMap<String, Object> frame : stack) {
        if (frame.containsKey(name)) {
          frame.put(name, value);
          return value;
        }
      }
    }

    MangoParser.ArrayAccessContext access = (MangoParser.ArrayAccessContext) ctx;
    ArrayList<Object> array = (ArrayList<Object>) visit(ctx.getChild(0));
    long index = (long) visit(ctx.getChild(2));

    if (index < 0 || array.size() <= index) {
      new RuntimeError("index " + index + " out of range for length " + array.size(),
        new Source(Runner.file, access.index.start.getLine(), access.start.getCharPositionInLine() + 1));
    }

    array.set((int) index, value);
    return value;
  }

  @SuppressWarnings("unchecked")
  private boolean validCast(Object obj, Object type) {
    if (type instanceof Class) {
      if (obj instanceof ArrayList) {
        for (Object e : (ArrayList<?>) obj)
          if (!validCast(e, type)) return false;

        return true;
      }

      return ((Class<?>) type).isInstance(obj);
    }

    for (Class<?> c : ((HashSet<Class<?>>) type))
      if (validCast(obj, c)) return true;

    return false;
  }
}