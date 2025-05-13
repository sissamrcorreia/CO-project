#pragma once

#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Class for describing write nodes.
   */
  class write_node : public cdk::basic_node {
    cdk::expression_node *_argument;

  public:
    write_node(int lineno, cdk::expression_node *argument)
        : cdk::basic_node(lineno), _argument(argument) {}

    cdk::expression_node *argument() { return _argument; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_write_node(this, level);
    }
  };

} // udf
