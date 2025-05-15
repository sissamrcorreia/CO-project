#pragma once

namespace udf {
  
  /**
   * Class for describing tensor declaration nodes.
   * Tensors can be written directly in programs. The format uses square bracket notation enclosing expressions (these expressions must be convertible to floating-point values).
   * Examples:
   * 2x2 Tensor: [[2,2], [2,3]]
   * 2x2 Tensor with expressions: [[2+1, i], [f(1), 2]]
   */
  class tensor_node : public cdk::expression_node {
    std::vector<cdk::expression_node*> _dimensions;
    cdk::expression_node *_initializer;

    public:
      tensor_node(int lineno, std::vector<cdk::expression_node*> *dimensions,
                  cdk::expression_node *initializer)
          : cdk::expression_node(lineno), _dimensions(*dimensions), _initializer(initializer) {}

      std::vector<cdk::expression_node*> *dimensions() { return &_dimensions; }
      cdk::expression_node *initializer() { return _initializer; }

      void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_node(this, level); }
  };

} // udf
