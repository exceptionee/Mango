import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import org.antlr.v4.runtime.tree.ParseTree;

class Interpreter extends MangoBaseVisitor<Object> {
  HashMap<String, Object> heap = new HashMap<String, Object>();

  @Override
  public Void visitVariable(MangoParser.VariableContext ctx) {
    heap.put(ctx.name.getText(), ctx.value != null? visit(ctx.value) : null);
    return null;
  }
  
  @Override
  public Object visitParenExpr(MangoParser.ParenExprContext ctx) {
    return visit(ctx.expr);
  }

  @Override
  public Object visitArrayAccess(MangoParser.ArrayAccessContext ctx) {
    ArrayList<?> array = (ArrayList<?>) visit(ctx.array);
    int index = (int) (long) visit(ctx.index);

    if (index < 0 || array.size() <= index) {
      new RuntimeError("index " + index + " out of range for length " + array.size(),
        new Source(Runner.getFileName(), ctx.index.start.getLine(), ctx.index.start.getCharPositionInLine() + 1));
    }

    return array.get(index);
  }

  @Override
  public Object visitVarExpr(MangoParser.VarExprContext ctx) {
    return heap.get(ctx.ID().getText());
  }

  @Override
  public Object visitUnaryExpr(MangoParser.UnaryExprContext ctx) {
    Object value = visit(ctx.expr);

    switch (ctx.op.getType()) {
      case 19:
        return value instanceof Long? +((long) value) : +((double) value);

      case 20:
        return value instanceof Long? -((long) value) : -((double) value);

      case 21:
        return value instanceof Long? ((long) value) + 1 : ((double) value) + 1;

      case 22:
        return value instanceof Long? ((long) value) - 1 : ((double) value) - 1;

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
        case 24:
          return (long) left * (long) right;
        case 25:
          if ((long) right == 0)
            new RuntimeError("can not divide by 0",
              new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
          return (long) left / (long) right;
        case 26:
          return (long) left % (long) right;
      }
    }

    switch (ctx.op.getType()) {
      case 24:
        return left.doubleValue() * right.doubleValue();
      case 25:
        if (right.doubleValue() == 0)
          new RuntimeError("can not divide by 0",
            new Source(Runner.getFileName(), ctx.op.getLine(), ctx.op.getCharPositionInLine() + 1));
        return left.doubleValue() / right.doubleValue();
      case 26:
        return left.doubleValue() % right.doubleValue();
    }

    return null;
  }

  @Override
  public Object visitPrimaryExpr(MangoParser.PrimaryExprContext ctx) {
    Object left = visit(ctx.left);
    Object right = visit(ctx.right);

    if (ctx.op.getType() == 19) {
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
      case 27:
        return left.doubleValue() > right.doubleValue();
      
      case 28:
        return left.doubleValue() < right.doubleValue();

      case 29:
        return left.doubleValue() >= right.doubleValue();
      
      default:
        return left.doubleValue() <= right.doubleValue();
    }
  }

  @Override
  public Object visitEqualityExpr(MangoParser.EqualityExprContext ctx) {
    Object left = visit(ctx.left);
    Object right = visit(ctx.right);

    if (ctx.op.getType() == 31) {
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
    if (ctx.op.getType() == 33)
      return (boolean) visit(ctx.left) && (boolean) visit(ctx.right);

    return (boolean) visit(ctx.left) || (boolean) visit(ctx.right);
  }

  @Override
  public Object visitTernaryExpr(MangoParser.TernaryExprContext ctx) {
    return (boolean) visit(ctx.condition)? visit(ctx.value) : visit(ctx.default_);
  }

  @Override
  public Object visitIntLiteral(MangoParser.IntLiteralContext ctx) {
    try {
      return Long.parseLong(ctx.start.getText());
    } catch (NumberFormatException e) {
      new RuntimeError("number " + ctx.start.getText() + " out of range for type int",
        new Source(Runner.getFileName(), ctx.start.getLine(), ctx.start.getCharPositionInLine() + 1));
    }

    return null;
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
    return ctx.start.getType() == 40? true : false;
  }

	@Override
  public Object visitArrayLiteral(MangoParser.ArrayLiteralContext ctx) {
    ArrayList<Object> list = new ArrayList<Object>();

    if (ctx.array == null)
      return list;

    List<ParseTree> children = ctx.array.children;
    for (int i = 0; i < children.size(); i += 2) {
      list.add(visit(children.get(i)));
    }

    return list;
  }

  @Override
  public Object visitNullLiteral(MangoParser.NullLiteralContext ctx) {
    return null;
  }
}