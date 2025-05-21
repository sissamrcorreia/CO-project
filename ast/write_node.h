#pragma once

#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Class for describing print nodes.
   */
  class write_node : public cdk::basic_node {
    cdk::sequence_node *_arguments;
    bool _newline = false;

  public:
    write_node(int lineno, cdk::sequence_node *arguments, bool newline) :
        cdk::basic_node(lineno), _arguments(arguments), _newline(newline) {
    }

    cdk::sequence_node *arguments() { return _arguments; }
    bool newline() { return _newline; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_write_node(this, level); }

  };

} // udf
