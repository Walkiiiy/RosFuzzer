import cpp
import semmle.code.cpp.ir.dataflow.DataFlow

// Configuration for function pointer calls
class Conf extends DataFlow::Configuration {
  Conf() { this = "Conf" }

  override predicate isSource(DataFlow::Node source) { source.asExpr() instanceof FunctionAccess }

  override predicate isSink(DataFlow::Node sink) { sink.asExpr() = any(ExprCall call).getExpr() }
}

// Direct function calls
predicate directCall(Function caller, Function callee) {
  exists(FunctionCall fc |
    fc.getEnclosingFunction() = caller and
    fc.getTarget() = callee
  )
}

// Virtual method calls
predicate virtualCall(Function caller, Function callee) {
  exists(Call vc |
    vc.getEnclosingFunction() = caller and
    vc.getTarget() = callee and
    exists(MemberFunction mf |  
      mf = callee and
      exists(MemberFunction base |
        base = mf.getAnOverriddenFunction*() and
        base.isVirtual()
      )
    )
  )
}

// Function pointer calls
predicate functionPointerCall(Function caller, Function callee) {
  exists(FunctionAccess funcAccess, DataFlow::Node sink |
    any(Conf conf).hasFlow(DataFlow::exprNode(funcAccess), sink) and
    sink.getEnclosingCallable() = caller and
    callee = funcAccess.getTarget()
  )
}

// Combined edge predicate
predicate edges(Function caller, Function callee) {
  directCall(caller, callee) or
  virtualCall(caller, callee) or
  functionPointerCall(caller, callee)
}

// Reachability predicate (includes transitive calls)
predicate reachable(Function src, Function dest) {
  edges(src, dest)
  or
  exists(Function mid |
    edges(src, mid) and
    reachable(mid, dest)
  )
}

// Entry point predicate
predicate isEntryPoint(Function f) {
  // Be permissive so we still match when CodeQL stores qualified names or overloads.
  f.hasName("ENTRY_FNC") or
  f.getQualifiedName().matches("%::ENTRY_FNC") or
  f.getQualifiedName().matches("%ENTRY_FNC%")
}

// Main query (direct edges only to keep result size small)
from Function start, Function end
where
  isEntryPoint(start) and
  edges(start, end)
select
  start as caller,
  end as callee,
  start.getFile() as caller_src,
  end.getFile() as callee_src
