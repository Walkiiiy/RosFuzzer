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
  f.hasName("callback") or
  f.getQualifiedName().matches("%::callback") or
  f.getQualifiedName().matches("%callback%")
}

// Main query (direct edges only to keep result size small)
from Function start, Function end, Location start_loc, Location end_loc, string caller_sig, string callee_sig
where
  isEntryPoint(start) and
  edges(start, end) and
  start_loc = start.getLocation() and
  end_loc = end.getLocation() and
  caller_sig = start.getQualifiedName() + start.getParameterString() and
  callee_sig = end.getQualifiedName() + end.getParameterString()
select
  start as caller,
  end as callee,
  start.getFile() as caller_src,
  end.getFile() as callee_src,
  start_loc.getStartLine() as start_body_start_line,
  start_loc.getEndLine() as start_body_end_line,
  end_loc.getStartLine() as end_body_start_line,
  end_loc.getEndLine() as end_body_end_line,
  caller_sig as caller_signature,
  start.getParameterString() as caller_parameter_string,
  start.getType() as caller_return_type,
  start.getUnspecifiedType() as caller_return_type_inferred,
  callee_sig as callee_signature,
  end.getParameterString() as callee_parameter_string,
  end.getType() as callee_return_type,
  end.getUnspecifiedType() as callee_return_type_inferred
