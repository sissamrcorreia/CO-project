#pragma once

namespace udf {
  
  /**
   * Class for describing tensor contraction nodes.
   * Implements tensor contractions (e.g., `t1 ** t2` for matrix
   * multiplication-like operations).
   */
  class tensor_contract_node : public cdk::binary_operation_node {
  public:
    tensor_contract_node(int lineno, cdk::expression_node *left,
                        cdk::expression_node *right,
                        const std::string &operation)
      : cdk::binary_operation_node(lineno, left, right) {}

    void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_contract_node(this, level); }
  };

} // udf
