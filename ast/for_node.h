#pragma once

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Class for describing for-cycle nodes.
   */
  class for_node : public cdk::basic_node {
    cdk::expression_node *_init;
    cdk::expression_node *_condition;
    cdk::expression_node *_step;
    cdk::basic_node *_instruction;

  public:
    for_node(int lineno, cdk::expression_node *init, cdk::expression_node *condition, cdk::expression_node *step,
             cdk::basic_node *instruction) :
        cdk::basic_node(lineno), _init(init), _condition(condition), _step(step), _instruction(instruction) {
    }

    cdk::expression_node* init()      { return _init;        }
    cdk::expression_node* condition() { return _condition;   }
    cdk::expression_node* step()      { return _step;        }
    cdk::basic_node* instruction()    { return _instruction; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_for_node(this, level); }

  };

} // udf
