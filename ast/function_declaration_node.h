#pragma once

#include <string>
#include <vector>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Class for describing function declarations.
   */
  class function_declaration_node: public cdk::typed_node {
    int _qualifier; // 0 = none, 1 = public, 2 = forward
    std::string _identifier;
    cdk::sequence_node *_arguments;
    std::shared_ptr<cdk::basic_type> _return_type;
    std::vector<std::shared_ptr<cdk::basic_type>> _arg_types;

  public:
    // Constructor for function declarations without body (forward declarations)
    function_declaration_node(int lineno, int qualifier, const std::string &identifier, cdk::sequence_node *arguments) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments) {
      type(cdk::primitive_type::create(0, cdk::TYPE_VOID));
    }

    // Constructor for function declarations with body
    function_declaration_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> funType, const std::string &identifier,
                              cdk::sequence_node *arguments) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments), _return_type(funType) {
      type(funType);
    }

    int qualifier() { return _qualifier; }

    std::string& identifier() { return _identifier; }

    cdk::typed_node* argument(size_t ax) { return dynamic_cast<cdk::typed_node*>(_arguments->node(ax)); }

    cdk::sequence_node* arguments() { return _arguments;}

    std::shared_ptr<cdk::basic_type> return_type() const { return _return_type; }
    std::vector<std::shared_ptr<cdk::basic_type>>& arg_types() { return _arg_types; }
    const std::vector<std::shared_ptr<cdk::basic_type>>& arg_types() const { return _arg_types; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_function_declaration_node(this, level); }
  };

} // udf