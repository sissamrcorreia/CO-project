#pragma once

#include <cdk/ast/basic_node.h>

namespace udf {

  /**
   * Class for describing break nodes.
   */
  class break_node : public cdk::basic_node {
    int _level;

  public:
    break_node(int lineno, int level) : cdk::basic_node(lineno), _level(level) {}

    int level() { return _level; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_break_node(this, level);
    }
  };

} // udf
