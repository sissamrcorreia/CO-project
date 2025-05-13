#pragma once

#include "block_node.h"
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/typed_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/functional_type.h>
#include <cdk/types/primitive_type.h>
#include <cdk/types/typename_type.h>
#include <memory>
#include <vector>

namespace udf {

  /**
   * Class for describing function definition nodes.
   */
  class function_definition_node : public cdk::expression_node {
    cdk::sequence_node *_arguments;
    udf::block_node *_block;
    bool _is_main;

  public:
    function_definition_node(int lineno, cdk::sequence_node *arguments,
                  std::shared_ptr<cdk::basic_type> return_type,
                  udf::block_node *block, bool is_main = false)
        : cdk::expression_node(lineno), _arguments(arguments), _block(block),
          _is_main(is_main) {
      std::vector<std::shared_ptr<cdk::basic_type>> arg_types;
      for (size_t i = 0; i < arguments->size(); i++) {
        arg_types.push_back(
            dynamic_cast<cdk::typed_node *>(arguments->node(i))->type());
      }

      this->type(cdk::functional_type::create(arg_types, return_type));
    }

    function_definition_node(int lineno, cdk::sequence_node *arguments)
        : cdk::expression_node(lineno), _arguments(arguments), _block(nullptr),
          _is_main(true) {
      this->type(cdk::functional_type::create(
          cdk::primitive_type::create(4, cdk::TYPE_INT)));
    }

    cdk::sequence_node *arguments() { return _arguments; }
    cdk::basic_node *block() { return _block; }

    bool is_main() { return _is_main; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_function_definition_node(this, level);}
  };

} // udf