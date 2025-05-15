#pragma once

#include <cdk/ast/unary_operation_node.h>

namespace udf {

  /**
   * Class for describing stack allocation nodes.
   */
  class alloc_node : public cdk::unary_operation_node {
    cdk::expression_node *_argument;

  public:
    alloc_node(int lineno, cdk::expression_node *argument)
        : cdk::unary_operation_node(lineno, argument) {}

    void accept(basic_ast_visitor *sp, int level) { sp->do_alloc_node(this, level); }

  };

} // udf
