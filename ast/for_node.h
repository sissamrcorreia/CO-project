#pragma once

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Class for describing for-cycle nodes.
   */
  class for_node: public cdk::basic_node {
    cdk::sequence_node *_declaration;
    cdk::sequence_node *_condition;
    cdk::sequence_node *_increment;
    cdk::basic_node *_block;

  public:
    for_node(int lineno, cdk::sequence_node *declaration,
                       cdk::sequence_node *condition, cdk::sequence_node *increment,
                       cdk::basic_node *block)
            : cdk::basic_node(lineno), _declaration(declaration),
              _condition(condition), _increment(increment), _block(block) {}

    cdk::sequence_node *declaration() { return _declaration; }
    cdk::sequence_node *condition()   { return _condition;   }
    cdk::sequence_node *increment()   { return _increment;   }
    cdk::basic_node *block()          { return _block;       }

    void accept(basic_ast_visitor *sp, int level) { sp->do_for_node(this, level); }

  };

} // udf
