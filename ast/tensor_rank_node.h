#pragma once

namespace udf {
  
  /**
   * Class for describing tensor rank queries.
   * Returns the number of dimensions of a tensor (e.g., `t.rank`).
   */
  class tensor_rank_node : public cdk::expression_node {
    cdk::expression_node *_argument;

  public:
    tensor_rank_node(int lineno, cdk::expression_node *argument)
        : cdk::expression_node(lineno), _argument(argument) {}

    cdk::expression_node *argument() { return _argument; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_rank_node(this, level); }

  };

} // udf
