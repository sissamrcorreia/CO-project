#pragma once

namespace udf {
  
  /**
   * Class for describing tensor indexing nodes.
   * Handles access to tensor elements via coordinates (e.g., `t@(i,j)`).
   */
  class tensor_index_node : public cdk::lvalue_node {
    cdk::expression_node *_argument;
    cdk::expression_node *_index;

  public:
    tensor_index_node(int lineno, cdk::expression_node *argument,
                      cdk::expression_node *index)
        : cdk::lvalue_node(lineno), _argument(argument), _index(index) {}

    cdk::expression_node *argument() { return _argument; }
    cdk::expression_node *index()    { return _index;    }

    void argument(cdk::expression_node *argument) { _argument = argument; }
    void index(cdk::expression_node *index)       { _index = index;       }

    void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_index_node(this, level); }

  };

} // udf
