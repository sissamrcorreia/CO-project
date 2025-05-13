#pragma once

namespace udf {
  
  /**
   * Class for describing tensor indexing nodes.
   * Handles access to tensor elements via coordinates (e.g., `t@(i,j)`).
   */
  // TODO: review this class
  class tensor_index_node : public cdk::expression_node {
    cdk::expression_node *_argument; // The tensor being indexed
    cdk::expression_node *_index;    // The index expression (e.g., `i` or `j`)

  public:
    tensor_index_node(int lineno, cdk::expression_node *argument,
                      cdk::expression_node *index)
        : cdk::expression_node(lineno), _argument(argument), _index(index) {}

    cdk::expression_node *argument() { return _argument; }
    cdk::expression_node *index() { return _index; }

    void argument(cdk::expression_node *argument) { _argument = argument; }
    void index(cdk::expression_node *index) { _index = index; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_index_node(this, level);
    }
  };

} // udf
