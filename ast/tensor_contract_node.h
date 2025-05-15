#pragma once

namespace udf {
  
  /**
   * Class for describing tensor contraction nodes.
   * Implements tensor contractions (e.g., `t1 ** t2` for matrix
   * multiplication-like operations).
   */
  class tensor_contract_node : public cdk::expression_node {
    cdk::expression_node *_left;
    cdk::expression_node *_right;

  public:
    tensor_contract_node(int lineno, cdk::expression_node *left,
                        cdk::expression_node *right,
                        const std::string &operation)
        : cdk::expression_node(lineno), _left(left), _right(right) {}

    cdk::expression_node *left()  { return _left;  }
    cdk::expression_node *right() { return _right; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_contract_node(this, level); }

  };

} // udf
