#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/functional_type.h>

namespace udf {

  /**
   * Class for describing function declarations.
   */
  class function_declaration_node : public cdk::typed_node {
    std::string _identifier;
    cdk::sequence_node *_arguments;
    std::shared_ptr<cdk::basic_type> _return_type;
    std::vector<std::shared_ptr<cdk::basic_type>> _arg_types;

  public:
    function_declaration_node(int lineno,
                             const std::string &identifier,
                             cdk::sequence_node *arguments,
                             std::shared_ptr<cdk::basic_type> return_type)
        : cdk::typed_node(lineno),
          _identifier(identifier),
          _arguments(arguments),
          _return_type(return_type) {
      
      for (size_t i = 0; i < _arguments->size(); ++i) {
        auto arg = dynamic_cast<cdk::typed_node*>(_arguments->node(i));
        _arg_types.push_back(arg->type());
      }

      std::vector<std::shared_ptr<cdk::basic_type>> output_types;
      output_types.push_back(_return_type);

      this->type(cdk::functional_type::create(_arg_types, _return_type));

    }

    const std::string &identifier() const { return _identifier; }
    cdk::sequence_node *arguments() const { return _arguments; }
    std::shared_ptr<cdk::basic_type> return_type() const { return _return_type; }
    const std::vector<std::shared_ptr<cdk::basic_type>> &arg_types() const { return _arg_types; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_function_declaration_node(this, level); }

  };

} // udf