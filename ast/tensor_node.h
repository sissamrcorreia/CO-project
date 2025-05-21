#pragma once

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {
  
  /**
   * Class for describing tensor declaration nodes.
   * Tensors can be written directly in programs. The format uses square bracket notation enclosing expressions (these expressions must be convertible to floating-point values).
   * Examples:
   * 2x2 Tensor: [[2,2], [2,3]]
   * 2x2 Tensor with expressions: [[2+1, i], [f(1), 2]]
   */
  class tensor_node : public cdk::expression_node {
    cdk::sequence_node *_elements;

  public:
    tensor_node(int lineno, cdk::sequence_node *elements)
      : cdk::expression_node(lineno), _elements(elements) {}

    cdk::sequence_node *elements() { return _elements; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_node(this, level); }

  };

} // udf
