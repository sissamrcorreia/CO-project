#pragma once

#include <cdk/ast/basic_node.h>

namespace udf {

  /**
   * Class for describing continue nodes.
   */
  class continue_node : public cdk::basic_node {
    int _level;

  public:
    continue_node(int lineno, int level) : cdk::basic_node(lineno), _level(level) {
    }

    int level() { return _level; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_continue_node(this, level); }

  };

} // udf
