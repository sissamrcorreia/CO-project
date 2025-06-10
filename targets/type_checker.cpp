#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

// must come after other #includes
#include "udf_parser.tab.h"


#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void udf::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_capacity_node(udf::tensor_capacity_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_capacity argument must be a tensor");
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_tensor_rank_node(udf::tensor_rank_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_rank argument must be a tensor");
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_tensor_dim_node(udf::tensor_dim_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_dim tensor argument must be a tensor");
  }
  node->dimension()->accept(this, lvl + 2);
  if (!node->dimension()->is_typed(cdk::TYPE_INT)) {
    throw std::string("tensor_dim dimension argument must be an integer");
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_tensor_dims_node(udf::tensor_dims_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_dims argument must be a tensor");
  }
  // Returns a pointer to the dimensions array (size_t*)
  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
}

void udf::type_checker::do_tensor_index_node(udf::tensor_index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  if (!node->base()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_index tensor argument must be a tensor");
  }
  auto indices = node->position();
  for (size_t i = 0; i < indices->size(); i++) {
    auto index = dynamic_cast<cdk::expression_node*>(indices->node(i));
    index->accept(this, lvl + 2);
    if (!index->is_typed(cdk::TYPE_INT)) {
      throw std::string("tensor_index indices must be integers");
    }
  }
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void udf::type_checker::do_tensor_reshape_node(udf::tensor_reshape_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->tensor()->accept(this, lvl + 2);
  if (!node->tensor()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_reshape tensor argument must be a tensor");
  }
  auto new_dims = node->arguments();
  for (size_t i = 0; i < new_dims->size(); i++) {
    auto dim = dynamic_cast<cdk::expression_node*>(new_dims->node(i));
    dim->accept(this, lvl + 2);
    if (!dim->is_typed(cdk::TYPE_INT)) {
      throw std::string("tensor_reshape dimensions must be integers");
    }
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_TENSOR));
}

void udf::type_checker::do_tensor_contract_node(udf::tensor_contract_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_contract left argument must be a tensor");
  }
  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor_contract right argument must be a tensor");
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_TENSOR));
}

void udf::type_checker::do_tensor_node(udf::tensor_node *const node, int lvl) {
  ASSERT_UNSPEC;
  auto elements = node->elements();
  if (!elements || elements->size() == 0) {
    throw std::string("tensor literal cannot be empty");
  }

  // Ensure all sub-sequences have the same length and contain doubles
  size_t first_size = 0;
  bool first = true;
  for (size_t i = 0; i < elements->size(); i++) {
    auto subseq = dynamic_cast<cdk::sequence_node*>(elements->node(i));
    if (!subseq) {
      throw std::string("tensor elements must be sequences");
    }
    subseq->accept(this, lvl + 2);
    for (size_t j = 0; j < subseq->size(); j++) {
      auto elem = dynamic_cast<cdk::expression_node*>(subseq->node(j));
      if (!elem->is_typed(cdk::TYPE_DOUBLE)) {
        throw std::string("tensor elements must be doubles");
      }
    }
    if (first) {
      first_size = subseq->size();
      first = false;
    } else if (subseq->size() != first_size) {
      throw std::string("inconsistent dimensions in tensor literal");
    }
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_TENSOR));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_address_of_node(udf::address_of_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_index_node(udf::index_node * const node, int lvl) {
  ASSERT_UNSPEC;
  std::shared_ptr<cdk::reference_type> btype;

  if (node->base()) {
    node->base()->accept(this, lvl + 2);
    btype = cdk::reference_type::cast(node->base()->type());
    if (!node->base()->is_typed(cdk::TYPE_POINTER))
      throw std::string("pointer expression expected in index left-value");
  } else {
    btype = cdk::reference_type::cast(_function->type());
    if (!_function->is_typed(cdk::TYPE_POINTER))
      throw std::string("return pointer expression expected in index left-value");
    throw std::string("index node without base is not supported");
  }

  node->index()->accept(this, lvl + 2);
  if (!node->index()->is_typed(cdk::TYPE_INT))
    throw std::string("integer expression expected in left-value index");

  // Set the type of this node to the type pointed to by the pointer
  node->type(btype->referenced());
}

//---------------------------------------------------------------------------

void udf::type_checker::do_block_node(udf::block_node *const node, int lvl) {
  // EMPTY
}

void udf::type_checker::do_continue_node(udf::continue_node *const node, int lvl) {
  if (!_inLoop) {
    throw std::string("continue statement outside loop at line " + std::to_string(node->lineno()));
  }
}

void udf::type_checker::do_break_node(udf::break_node *const node, int lvl) {
  if (!_inLoop) {
    throw std::string("break statement outside loop at line " + std::to_string(node->lineno()));
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void udf::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

void udf::type_checker::do_nullptr_node(udf::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, nullptr));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_alloc_node(udf::alloc_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("objects argument must be an integer at line " + std::to_string(node->lineno()));
  }
  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void udf::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void udf::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of binary expression");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of binary expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else {
    throw std::string("wrong type in unary logical expression");
  }
}

void udf::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("left argument of && must be an integer at line " + std::to_string(node->lineno()));
  }
  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("right argument of && must be an integer at line " + std::to_string(node->lineno()));
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("left argument of || must be an integer at line " + std::to_string(node->lineno()));
  }
  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("right argument of || must be an integer at line " + std::to_string(node->lineno()));
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  auto symbol = _symtab.find(id);
  if (symbol) node->type(symbol->type());
  else{
    throw std::string("undeclared variable '" + id + "'");
  }
}

void udf::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void udf::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  try {
    node->lvalue()->accept(this, lvl);
  } catch (const std::string &id) {
    auto symbol = std::make_shared<udf::symbol>(
      false,                // constant
      0,                    // qualifier
      cdk::primitive_type::create(4, cdk::TYPE_INT), // type
      id,                   // name
      false,                // initialized
      false                 // function
    );
    _symtab.insert(id, symbol);
    _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
    node->lvalue()->accept(this, lvl);
  }

  if (!node->lvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of assignment expression");

  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of assignment expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_declaration_node(udf::function_declaration_node *const node, int lvl) {
  std::string id;

  // "fix" naming issues...
  if (node->identifier() == "udf")
    id = "_main";
  else if (node->identifier() == "_main")
    id = "._main";
  else
    id = node->identifier();

  // remember symbol so that args know
  auto function = std::make_shared<udf::symbol>(
    false,                // constant
    0,                    // qualifier
    node->type(),         // type
    id,                   // name
    false,                // initialized
    true                  // function
  );

  std::vector<std::shared_ptr<cdk::basic_type>> argtypes;
  for (size_t ax = 0; ax < node->arguments()->size(); ax++)
    argtypes.push_back(node->argument(ax)->type());
  function->set_argument_types(argtypes);

  std::shared_ptr<udf::symbol> previous = _symtab.find(function->name());
  if (previous)
  {
    if (false /*FIXME: should verify fields*/)
    {
      throw std::string("conflicting declaration for '" + function->name() + "'");
    }
  }
  else
  {
    _symtab.insert(function->name(), function);
    _parent->set_new_symbol(function);
  }
}

void udf::type_checker::do_function_definition_node(udf::function_definition_node *const node, int lvl) {
  std::string id;

  // "fix" naming issues...
  if (node->identifier() == "udf")
    id = "_main";
  else if (node->identifier() == "_main")
    id = "._main";
  else
    id = node->identifier();

  _inBlockReturnType = nullptr;

  // remember symbol so that args know
  auto function = std::make_shared<udf::symbol>(
    false,                // constant
    0,                    // qualifier
    node->type(),         // type
    id,                   // name
    false,                // initialized
    true                  // function
  );

  std::vector<std::shared_ptr<cdk::basic_type>> argtypes;
  for (size_t ax = 0; ax < node->arguments()->size(); ax++)
    argtypes.push_back(node->argument(ax)->type());
  function->set_argument_types(argtypes);

  std::shared_ptr<udf::symbol> previous = _symtab.find(function->name());
  if (previous)
  {
    if (previous->forward() && ((previous->qualifier() == tPUBLIC && node->qualifier() == tPUBLIC) || (previous->qualifier() == tPRIVATE && node->qualifier() == tPRIVATE)))
    {
      _symtab.replace(function->name(), function);
      _parent->set_new_symbol(function);
    }
    else
    {
      throw std::string("conflicting definition for '" + function->name() + "'");
    }
  }
  else
  {
    _symtab.insert(function->name(), function);
    _parent->set_new_symbol(function);
  }
}

void udf::type_checker::do_return_node(udf::return_node *const node, int lvl) {
  if (node->retval())
  {
    if (_function->type() != nullptr && _function->is_typed(cdk::TYPE_VOID))
      throw std::string(
          "initializer specified for void function.");

    node->retval()->accept(this, lvl + 2);

    // function is auto: copy type of first return expression
    if (_function->type() == nullptr)
    {
      _function->set_type(node->retval()->type());
      return; // simply set the type
    }

    if (_inBlockReturnType == nullptr)
    {
      _inBlockReturnType = node->retval()->type();
    }
    else
    {
      if (_inBlockReturnType != node->retval()->type())
      {
        _function->set_type(cdk::primitive_type::create(0, cdk::TYPE_ERROR)); // probably irrelevant
        throw std::string("all return statements in a function must return the same type.");
      }
    }

    std::cout << "FUNCT TYPE " << (_function->type() == nullptr ? "auto" : cdk::to_string(_function->type())) << std::endl;
    std::cout << "RETVAL TYPE " << cdk::to_string(node->retval()->type()) << std::endl;

    if (_function->is_typed(cdk::TYPE_INT))
    {
      if (!node->retval()->is_typed(cdk::TYPE_INT))
        throw std::string("wrong type for initializer (integer expected).");
    }
    else if (_function->is_typed(cdk::TYPE_DOUBLE))
    {
      if (!node->retval()->is_typed(cdk::TYPE_INT) && !node->retval()->is_typed(cdk::TYPE_DOUBLE))
      {
        throw std::string("wrong type for initializer (integer or double expected).");
      }
    }
    else if (_function->is_typed(cdk::TYPE_STRING))
    {
      if (!node->retval()->is_typed(cdk::TYPE_STRING))
      {
        throw std::string("wrong type for initializer (string expected).");
      }
    }
    else if (_function->is_typed(cdk::TYPE_POINTER))
    {
      // FIXME: trouble!!!
      int ft = 0, rt = 0;
      auto ftype = _function->type();
      while (ftype->name() == cdk::TYPE_POINTER)
      {
        ft++;
        ftype = cdk::reference_type::cast(ftype)->referenced();
      }
      auto rtype = node->retval()->type();
      while (rtype != nullptr && rtype->name() == cdk::TYPE_POINTER)
      {
        rt++;
        rtype = cdk::reference_type::cast(rtype)->referenced();
      }

      std::cout << "FUNCT TYPE " << cdk::to_string(_function->type()) << " --- " << ft << " -- " << ftype->name() << std::endl;
      std::cout << "RETVAL TYPE " << cdk::to_string(node->retval()->type()) << " --- " << rt << " -- " << cdk::to_string(rtype)
                << std::endl;

      bool compatible = (ft == rt) && (rtype == nullptr || (rtype != nullptr && ftype->name() == rtype->name()));
      if (!compatible)
        throw std::string("wrong type for return expression (pointer expected).");
    }
    else
    {
      throw std::string("unknown type for initializer.");
    }
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_variable_declaration_node(udf::variable_declaration_node *const node, int lvl) {
  ASSERT_UNSPEC;
  if (node->initializer() != nullptr) {
    node->initializer()->accept(this, lvl + 2);

    if (node->is_typed(cdk::TYPE_INT)) {
      if (!node->initializer()->is_typed(cdk::TYPE_INT))
        throw std::string("wrong type for initializer (integer expected).");

    } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
      if (!node->initializer()->is_typed(cdk::TYPE_INT) && !node->initializer()->is_typed(cdk::TYPE_DOUBLE))
        throw std::string("wrong type for initializer (integer or double expected).");

    } else if (node->is_typed(cdk::TYPE_STRING)) {
      if (!node->initializer()->is_typed(cdk::TYPE_STRING))
        throw std::string("wrong type for initializer (string expected).");

    } else if (node->is_typed(cdk::TYPE_POINTER)) {
      if (!node->initializer()->is_typed(cdk::TYPE_POINTER)) { // FIXME
        auto in = (cdk::literal_node<int> *)node->initializer();
        if (in == nullptr || in->value() != 0)
          throw std::string("wrong type for initializer (pointer expected).");
      }

    } else if (node->is_typed(cdk::TYPE_TENSOR)) {
      if (!node->initializer()->is_typed(cdk::TYPE_TENSOR))
        throw std::string("wrong type for initializer (tesnor expected).");

    } else {
      throw std::string("unknown type for initializer.");
    }
  }

  const std::string &id = node->identifier();
  auto symbol = std::make_shared<udf::symbol>(
  false,                // constant
  0,                    // qualifier
  node->type(),         // type
  id,                   // name
  (bool)node->initializer(), // initialized
  false                 // function
);
  if (_symtab.insert(id, symbol)) {
    _parent->set_new_symbol(symbol); // advise parent that a symbol has been inserted
  } else {
    throw std::string("variable '" + id + "' redeclared");
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_evaluation_node(udf::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void udf::type_checker::do_write_node(udf::write_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);
}

void udf::type_checker::do_input_node(udf::input_node *const node, int lvl) {
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_for_node(udf::for_node *const node, int lvl) {
  // _symtab.push();
  // _inLoop = true;
  // if (node->declaration())
  //   node->declaration()->accept(this, lvl + 2);
  // if (node->condition()) {
  //   node->condition()->accept(this, lvl + 2);
  //   auto cond_expr = dynamic_cast<cdk::expression_node*>(node->condition());
  //   if (!cond_expr || !cond_expr->is_typed(cdk::TYPE_INT))
  //     throw std::string("for loop condition must be an integer at line " + std::to_string(node->lineno()));
  // }
  // if (node->increment()) node->increment()->accept(this, lvl + 2);
  
  // if (node->block()) node->block()->accept(this, lvl + 2);

  // _inLoop = false;
  // _symtab.pop();
}

//---------------------------------------------------------------------------

void udf::type_checker::do_if_node(udf::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  if (!node->condition()->is_typed(cdk::TYPE_INT))
    throw std::string("expected integer condition");
}

void udf::type_checker::do_if_else_node(udf::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  if (!node->condition()->is_typed(cdk::TYPE_INT))
    throw std::string("expected integer condition");
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_call_node(udf::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;

  const std::string &id = node->identifier();
  auto symbol = _symtab.find(id);
  if (symbol == nullptr)
    throw std::string("symbol '" + id + "' is undeclared.");
  if (!symbol->isFunction())
    throw std::string("symbol '" + id + "' is not a function.");

  if (symbol->is_typed(cdk::TYPE_STRUCT))
  {
    // declare return variable for passing to function call
    const std::string return_var_name = "$return_" + id;
    auto return_symbol = std::make_shared<udf::symbol>(
      false,                // constant
      0,                    // qualifier
      symbol->type(),       // type
      return_var_name,      // name
      false,                // initialized
      false                 // function
    );
    if (_symtab.insert(return_var_name, return_symbol))
    {
    }
    else
    {
      // if already declared, ignore new insertion
    }
  }

  node->type(symbol->type());

  if (node->arguments()->size() == symbol->number_of_arguments())
  {
    node->arguments()->accept(this, lvl + 4);
    for (size_t ax = 0; ax < node->arguments()->size(); ax++)
    {
      if (node->argument(ax)->type() == symbol->argument_type(ax))
        continue;
      if (symbol->argument_is_typed(ax, cdk::TYPE_DOUBLE) && node->argument(ax)->is_typed(cdk::TYPE_INT))
        continue;
      throw std::string("type mismatch for argument " + std::to_string(ax + 1) + " of '" + id + "'.");
    }
  }
  else
  {
    throw std::string(
        "number of arguments in call (" + std::to_string(node->arguments()->size()) + ") must match declaration (" + std::to_string(symbol->number_of_arguments()) + ").");
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_sizeof_node(udf::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------
