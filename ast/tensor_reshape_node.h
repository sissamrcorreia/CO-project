#pragma once

namespace udf {
  /**
   * Class for describing tensor reshaping nodes.
   * Manages operations that change the tensor's shape while preserving data
   * (e.g., `t.reshape(3,2)`).
   */

  class tensor_reshape_node : public cdk::expression_node {
    cdk::expression_node *_argument;
    cdk::expression_node *_shape;

  public:
    tensor_reshape_node(int lineno, cdk::expression_node *argument,
                        cdk::expression_node *shape)
        : cdk::expression_node(lineno), _argument(argument), _shape(shape) {}

    cdk::expression_node *argument() { return _argument; }
    cdk::expression_node *shape() { return _shape; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_reshape_node(this, level);
    }
  };

} // udf
