#pragma once

#include <cdk/ast/lvalue_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {
  
  /**
   * Class for describing tensor indexing nodes.
   * Handles access to tensor elements via coordinates (e.g., `t@(i,j)`).
   */
  class tensor_index_node: public cdk::lvalue_node {
    cdk::expression_node *_base;
    cdk::sequence_node *_position;

  public:
    tensor_index_node(int lineno, cdk::expression_node *base, cdk::sequence_node *position) :
        cdk::lvalue_node(lineno), _base(base), _position(position) {
    }

    cdk::expression_node *base() { return _base; }
    cdk::sequence_node *position() { return _position; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_index_node(this, level); }

  };

} // udf
