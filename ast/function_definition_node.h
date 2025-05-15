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
  class function_definition_node : public cdk::typed_node {
    int _qualifier;
    std::string _identifier;
    cdk::sequence_node *_arguments;
    udf::block_node *_block;

  public:
    // Constructor for function definitions without body (forward declarations)
    function_definition_node(int lineno, int qualifier, const std::string &identifier, cdk::sequence_node *arguments,
                             udf::block_node *block) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments), _block(block) {
      type(cdk::primitive_type::create(0, cdk::TYPE_VOID));
    }

    // Constructor for function definitions with body
    function_definition_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> funType, const std::string &identifier,
                             cdk::sequence_node *arguments, udf::block_node *block) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments), _block(block) {
      type(funType);
    }

    int qualifier() { return _qualifier; }

    const std::string& identifier() const { return _identifier; }

    cdk::sequence_node* arguments() { return _arguments; }

    cdk::typed_node* argument(size_t ax) { return dynamic_cast<cdk::typed_node*>(_arguments->node(ax)); }

    udf::block_node* block() { return _block; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_function_definition_node(this, level);}
  };

} // udf
