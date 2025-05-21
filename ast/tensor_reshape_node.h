#pragma once

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {
  /**
   * Class for describing tensor reshaping nodes.
   * Manages operations that change the tensor's shape while preserving data
   * (e.g., `t.reshape(3,2)`).
   */
  class tensor_reshape_node : public cdk::expression_node {
    cdk::expression_node *_tensor;
    cdk::sequence_node *_arguments;

  public:
    tensor_reshape_node(int lineno, cdk::expression_node *tensor, cdk::sequence_node *arguments)
      : cdk::expression_node(lineno), _tensor(tensor), _arguments(arguments) {}

    cdk::expression_node *tensor() { return _tensor; }
    cdk::sequence_node *arguments() { return _arguments; }
    cdk::expression_node *argument(size_t ix) { return dynamic_cast<cdk::expression_node*>(_arguments->node(ix)); }

    void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_reshape_node(this, level); }

  };

} // udf
