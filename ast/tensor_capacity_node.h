#pragma once

namespace udf {
  
  /**
   * Class for describing tensor capacity queries.
   * Returns the total number of elements in a tensor (e.g., `t.capacity`).
   */

  class tensor_capacity_node : public cdk::expression_node {
    cdk::expression_node *_argument;

  public:
    tensor_capacity_node(int lineno, cdk::expression_node *argument)
        : cdk::expression_node(lineno), _argument(argument) {}

    cdk::expression_node *argument() { return _argument; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_capacity_node(this, level);
    }
  };

} // udf
